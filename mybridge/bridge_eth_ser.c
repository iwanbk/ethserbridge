/**
 * 2011
 * @author Iwan Budi Kusnanto  ( iwan.b.kusnanto@gmail.com )
 */
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/pbuf.h"
#include "lwip/debug.h"
#include "lwip/ip.h"
#include "lwip/mem.h"
#include "lwip/inet.h"

#include "bridge_eth_ser.h"
#include "lwip/list.h"

#define BRIDGE_DEBUGF(debug,x) LWIP_PLATFORM_DIAG(x)
#define _err_print	printf

/* jumlah maksimal panjang payload dalam list bridge_e2s_list */
#define BRIDGE_E2S_LIST_MAXLEN		2000


/* list yang mem-buffer data dari ethernet ke serial */
static struct bridge_data bridge_e2s_list;

/* jumlah total panjang payload(byte) yang ada di buffer */
static u16_t bridge_e2s_list_totlen;

/* semaphore untuk mem-protect akses ke bridge_e2s_list */
static sys_sem_t bridge_e2s_list_sem;

/**
 * create bridge_data dari packet buffer
 */
struct bridge_data *bridge_data_create(struct pbuf *p)
{
	struct bridge_data *bd = mem_malloc(sizeof(struct bridge_data));
	if (bd == NULL) {
		return NULL;
	}

	bd->payload = mem_malloc(p->len);
	if (bd->payload == NULL) {
		mem_free(bd);
		return NULL;
	}

	bd->len = p->len;
	memcpy(bd->payload, p->payload, bd->len);
	return bd;
}

err_t bridge_data_destroy(struct bridge_data *bd)
{
	if (bd == NULL)
		return ERR_MEM;
	mem_free(bd->payload);
	mem_free(bd);
	return ERR_OK;
}
/**
 * copy packet buffer dan masukkan ke bridge_e2s_list
 * packet buffer dimasukkan ke tail
 */
err_t bridge_e2s_queue(struct pbuf *p)
{
	struct bridge_data *bd = NULL;
	
	//bridge_print_pbuf(p);

	/* cek total len */
	if (bridge_e2s_list_totlen + p->len > BRIDGE_E2S_LIST_MAXLEN) {
		/*buffer penuh, drop packet */
		_err_print("%s() packet dropped\n", __func__);
		_err_print("\ttot_len=%d, p len=%d, maxlen=%d\n",bridge_e2s_list_totlen,
				p->len, BRIDGE_E2S_LIST_MAXLEN);
		return ERR_MEM;
	}

	bd = bridge_data_create(p);

	if (bd == NULL) {
		_err_print("%s() can't allocate bridge data\n",__func__);
		return ERR_MEM; 
	}

	list_add_tail(&(bd->list), &(bridge_e2s_list.list));

	bridge_e2s_list_totlen += bd->len;

	/* up/signal the semaphore */
	sys_sem_signal(bridge_e2s_list_sem);

	return ERR_OK;
}

struct bridge_data *bridge_e2s_dequeue()
{
	struct list_head *pos, *q;
	struct bridge_data *bd = NULL;

	/* get the semaphore */
	sys_sem_wait(bridge_e2s_list_sem);

	/* get the data */
	list_for_each_safe(pos, q, &bridge_e2s_list.list) {
		bd = list_entry(pos, struct bridge_data, list);
		list_del(pos);
	}
	/* update bridge_e2s_list_totlen */
	if (bd != NULL)
		bridge_e2s_list_totlen -= bd->len;

	return bd;
}

err_t bridge_eth_ser_init()
{
	INIT_LIST_HEAD(&bridge_e2s_list.list);
	bridge_e2s_list_totlen = 0;
	bridge_e2s_list_sem = sys_sem_new(0);

	return ERR_OK;
}

/**
 * print packet buffer
 */
void bridge_print_pbuf(struct pbuf *p)
{
  struct ip_hdr *iphdr = p->payload;
  u8_t *payload;

  payload = (u8_t *)iphdr + IP_HLEN;

  BRIDGE_DEBUGF(IP_DEBUG, ("----- BRIDGE PRINT PBUF ---------\n"));
  BRIDGE_DEBUGF(IP_DEBUG, ("IP header:\n"));
  BRIDGE_DEBUGF(IP_DEBUG, ("+-------------------------------+\n"));
  BRIDGE_DEBUGF(IP_DEBUG, ("|%2"S16_F" |%2"S16_F" |  0x%02"X16_F" |     %5"U16_F"     | (v, hl, tos, len)\n",
                    IPH_V(iphdr),
                    IPH_HL(iphdr),
                    IPH_TOS(iphdr),
                    ntohs(IPH_LEN(iphdr))));
  BRIDGE_DEBUGF(IP_DEBUG, ("+-------------------------------+\n"));
  BRIDGE_DEBUGF(IP_DEBUG, ("|    %5"U16_F"      |%"U16_F"%"U16_F"%"U16_F"|    %4"U16_F"   | (id, flags, offset)\n",
                    ntohs(IPH_ID(iphdr)),
                    ntohs(IPH_OFFSET(iphdr)) >> 15 & 1,
                    ntohs(IPH_OFFSET(iphdr)) >> 14 & 1,
                    ntohs(IPH_OFFSET(iphdr)) >> 13 & 1,
                    ntohs(IPH_OFFSET(iphdr)) & IP_OFFMASK));
  BRIDGE_DEBUGF(IP_DEBUG, ("+-------------------------------+\n"));
  BRIDGE_DEBUGF(IP_DEBUG, ("|  %3"U16_F"  |  %3"U16_F"  |    0x%04"X16_F"     | (ttl, proto, chksum)\n",
                    IPH_TTL(iphdr),
                    IPH_PROTO(iphdr),
                    ntohs(IPH_CHKSUM(iphdr))));
  BRIDGE_DEBUGF(IP_DEBUG, ("+-------------------------------+\n"));
  BRIDGE_DEBUGF(IP_DEBUG, ("|  %3"U16_F"  |  %3"U16_F"  |  %3"U16_F"  |  %3"U16_F"  | (src)\n",
                    ip4_addr1(&iphdr->src),
                    ip4_addr2(&iphdr->src),
                    ip4_addr3(&iphdr->src),
                    ip4_addr4(&iphdr->src)));
  BRIDGE_DEBUGF(IP_DEBUG, ("+-------------------------------+\n"));
  BRIDGE_DEBUGF(IP_DEBUG, ("|  %3"U16_F"  |  %3"U16_F"  |  %3"U16_F"  |  %3"U16_F"  | (dest)\n",
                    ip4_addr1(&iphdr->dest),
                    ip4_addr2(&iphdr->dest),
                    ip4_addr3(&iphdr->dest),
                    ip4_addr4(&iphdr->dest)));
  BRIDGE_DEBUGF(IP_DEBUG, ("+-------------------------------+\n"));
}

