#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "libser.h"
#define BUF_LEN 500
int main(int argc, char **argv)
{
	fd_set readfs;		/* file descriptor set */
	int maxfd;		/* maximum file desciptor used */
	int loop = 1;		/* loop while TRUE */
	struct serdev_t sd;
	uint8_t str[BUF_LEN];

	if (argc != 2) {
		printf("./tes_baca_select /dev/serial\n");
		exit(-1);
	}

	sd.baudrate = B9600;
	strcpy(sd.name, argv[1]);
	if (ser_open(&sd) < 0) {
		printf("can't open serial dev\n");
		exit(-1);
	}

	maxfd = sd.fd + 1;	/* maximum bit entry (fd) to test */

	/* loop for input */
	while (loop) {
		FD_SET(sd.fd, &readfs);	/* set testing for source 1 */
		/* block until input becomes available */
		printf("waiting...\n");fflush(stdout);
		select(maxfd, &readfs, NULL, NULL, NULL);
		if (FD_ISSET(sd.fd, &readfs)) {
			printf("reading...\n");fflush(stdout);
			int nread = ser_safe_read(sd.fd, str, BUF_LEN);
			printf("terbaca = %d bytes\n", nread);
		}
		else
			printf("holai\n");
		//FD_ZERO(&readfs);
	}
	return 0;
}
