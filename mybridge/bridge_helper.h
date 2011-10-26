#ifndef BRIDGE_HELPER_H
#define BRIDGE_HELPER_H
#include "lwip/opt.h"
#include "lwip/pbuf.h"

struct pbuf *bridge_pbuf_build(char *buf, u16_t len);

#endif
