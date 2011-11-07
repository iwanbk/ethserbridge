#ifndef _LIBSER_H
#define _LIBSER_H

#include <unistd.h>
#include <termios.h>
#include <stdint.h>
struct serdev_t {
	char name[30];
	int baudrate;
	struct termios oldtio;
	int fd;
};
int ser_open(struct serdev_t *serdev);
int ser_close(struct serdev_t *serdev);
ssize_t ser_safe_read(int fd, void *buf, size_t len);
ssize_t ser_safe_write(int fd, const void *buf, size_t size);

ssize_t serbridge_safe_write(int fd, const void *buf, size_t size);
ssize_t serbridge_safe_read(int fd, void *buf, uint16_t *pack_len);

#endif
