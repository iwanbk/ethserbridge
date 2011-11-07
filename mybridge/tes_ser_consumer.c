#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "libser.h"

int main(int argc, char **argv)
{
	struct serdev_t sd;
	uint8_t buf[2000];
	int nread;
	uint16_t pack_len;


	strcpy(sd.name, argv[1]);
	sd.baudrate = B9600;

	if (ser_open(&sd) < 0) {
		printf("can't open ser dev\n");
		exit(0);
	}

	while(1) {
		nread = serbridge_safe_read(sd.fd, buf, &pack_len);
		printf("pack_len = %d, nread = %d\n", pack_len, nread);
		if (pack_len != nread) {
			printf("\t\tBAD BAD BAD!!!\n\n");
		}
	}
}
