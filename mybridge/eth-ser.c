#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "lwip/opt.h"
#include "lwip/init.h"
#include "lwip/tcp.h"
#include "lwip/tcpip.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/netifapi.h"
#include "netif/etharp.h"
#include "netif/tapif.h"

#include "bridge_eth_ser.h"

#include "libser.h"
#include "bridge_helper.h"

unsigned char debug_flags;
static struct ip_addr ipaddr, netmask, gw;

/* network interface handle */
struct netif netif;

/* serial device handle */
static struct serdev_t sd;

/* semaphore for serial access */
sys_sem_t 	sem_ser;

#define SER_BUF_LEN	1500

static void tcpip_init_done(void *arg)
{
	sys_sem_t *sem;
	sem = arg;
	sys_sem_signal(*sem);
}

/**
 * Thread yang meneruskan packet dari serial ke eth
 */
static void ser_to_eth_thread(void *arg)
{
	int maxfd;
	fd_set readfs;
	uint8_t buf[SER_BUF_LEN];
	int nread;
	struct pbuf *pbuf;
	int pack_len;
	int select_res;

	maxfd = sd.fd + 1;
	while (1) {
		FD_SET(sd.fd, &readfs);
		
		/* block */
		select_res = select(maxfd, &readfs, NULL, NULL, NULL);

		if (select_res == 0 || select_res == -1) {
			continue;
		}
		if (FD_ISSET(sd.fd, &readfs)) {
			/* get the semaphore */
			sys_sem_wait(sem_ser);
			
			/* read */
			//nread = ser_safe_read(sd.fd, buf, SER_BUF_LEN);
			nread = serbridge_safe_read(sd.fd, buf, &pack_len);

			/* release/signal the semaphore */
			sys_sem_signal(sem_ser);

			printf("nread = %d\n", nread);
			/* sent to eth */
			pbuf = bridge_pbuf_build((char *)buf, nread);
			if (pbuf != NULL)
				netif.linkoutput(&netif, pbuf);

		}
		else {
			printf("unknown event\n");
		}
	}
}

/**
 * Thread yang meneruskan data dari eth ke serial
 *
 * Packet data diambil dari bridge_queue
 */
static void eth_to_ser_thread(void *arg)
{
	struct bridge_data *bd;
	while (1) {
		bd = bridge_e2s_dequeue();
		if (bd != NULL) {
			printf("%s() packet len = %d\n", __func__, bd->len);
			
			/* get the semaphore */
			sys_sem_wait(sem_ser);

			/* send to serial device */
			serbridge_safe_write(sd.fd, bd->payload, bd->len);

			/* release the semaphore */
			sys_sem_signal(sem_ser);

			/* free the data */
			bridge_data_destroy(bd);
		}
	}
}

static void mainthread(void *arg)
{
	sys_sem_t sem;

	netif_init();

	/** inisialisasi stack **/
	sem = sys_sem_new(0);
	tcpip_init(tcpip_init_done, &sem);
	sys_sem_wait(sem);
	sys_sem_free(sem);
	printf("TCP/IP INitialized\n");

	/* inisialisasi network interface : NON PORTABLE CODE */
	netif_set_default(netif_add(&netif, &ipaddr, &netmask, &gw, NULL, tapif_init, tcpip_input));
	netif_set_up(&netif);

	/* block forever */
	sem = sys_sem_new(0);
	sys_sem_wait(sem);
}

int main(int argc, char **argv)
{
	
	struct in_addr inaddr;
	char ip_str[16] = {0};
	char nm_str[16] = {0};
	char gw_str[16] = {0};

	if (argc != 2) {
		printf("cara pakai : sudo ./eth-ser serial_output_device\n");
		exit(-1);
	}

	/* serial device initialization */
	strcpy(sd.name, argv[1]);
	sd.baudrate = B9600;
	if (ser_open(&sd) < 0) {
		printf("can't open serial input device, aborting..\n");
		exit(-1);
	}

	sem_ser = sys_sem_new(1);
	printf("serial device initialized\n");

	/* set lwip debug flags */
	debug_flags = LWIP_DBG_OFF;

	/*init eth*/
	IP4_ADDR(&gw, 192, 168, 0, 1);
	IP4_ADDR(&netmask, 255, 255, 255, 0);
	IP4_ADDR(&ipaddr, 192, 168, 0, 3);
	
	inaddr.s_addr = ipaddr.addr;
	strncpy(ip_str, inet_ntoa(inaddr), sizeof(ip_str));
	inaddr.s_addr = netmask.addr;
	strncpy(nm_str, inet_ntoa(inaddr), sizeof(nm_str));
	inaddr.s_addr = gw.addr;
	strncpy(gw_str, inet_ntoa(inaddr), sizeof(gw_str));

	printf("Host at %s mask %s gateway %s\n", ip_str, nm_str, gw_str);

	lwip_init();

	/* inisialsisasi bridge */
	if (bridge_eth_ser_init() != ERR_OK) {
		printf("bridge eth-ser initialization failed\n");
		exit(-1);
	}

	printf("System initialized\n");

	sys_thread_new("main_thread", mainthread, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
	sys_thread_new("eth_to_ser_thread", eth_to_ser_thread, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
	sys_thread_new("ser_to_eth_thread", ser_to_eth_thread, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);

	pause();	
	return 0;
}
