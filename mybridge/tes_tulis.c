#include "libser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
	struct serdev_t sd;
	ssize_t written;
	char str[] = "halo dunia begitu ndaadadafdakfahfajnhfakjhfjahfjahfkjanhfkjahfkjahfag hajdhjakhdkjahdkjahkjdahkdjhajhajfhagagfahfgahjfgajhgfajfafafaadadadadadadadadadadadadadadaddddddddddddddddddddddddddddddddddddddddddddddddddddddddd";

	if (argc != 2) {
		printf("./tes_tulis /dev/serial\n");
		exit(-1);
	}
	sd.baudrate = B9600;
	strcpy(sd.name, argv[1]);

	if (ser_open(&sd) < 0) {
		printf("can't open serial dev\n");
		exit(-1);
	}

	written = ser_safe_write(sd.fd, str, strlen(str));

	printf("strlen = %d, written = %d\n", strlen(str), written);
	return 0;
}
