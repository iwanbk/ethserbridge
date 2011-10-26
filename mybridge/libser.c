#include "libser.h"
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

/**
 * Open serial device
 *
 * @return fd
 */
int ser_open(struct serdev_t *serdev)
{
	int fd;
	struct termios newtio;

	fd = open(serdev->name, O_RDWR | O_NOCTTY);
	if (fd < 0)
		return -1;

	tcgetattr(fd, &serdev->oldtio);	/* save current port settings */

	bzero(&newtio, sizeof(newtio));

	newtio.c_cflag = serdev->baudrate | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;	/* non canonical , no echo */

	newtio.c_cc[VTIME] = 10;
	newtio.c_cc[VMIN] = 0;

	tcflush(fd, TCIOFLUSH);	/* clear both queued input & output */
	tcsetattr(fd, TCSANOW, &newtio);	/* make the change immediately */

	serdev->fd = fd;
	return fd;
}

/**
 * close serial device
 * - restore old setting
 * - close the device
 */
int ser_close(struct serdev_t *serdev)
{
	return close(serdev->fd);
}

ssize_t ser_safe_read(int fd, void *buf, size_t len)
{
	size_t left = len;
	ssize_t res;
	uint8_t *p = buf;

	while (left > 0) {
		res = read(fd, p, left);
		if (res <= 0)
			break;
		left -= res;
		p += res;
	}
	return len - left;
}

ssize_t ser_safe_write(int fd, const void *buf, size_t size)
{
	const uint8_t *p = buf;
	ssize_t left = size, nb;

	while (left > 0) {
		nb = write(fd, p, left);

		if (nb <= 0)
			break;
		p += nb;
		left -= nb;
	}
	return size - left;
}
