#ifndef BRIDGE_ETH_SER_H
#define BRIDGE_ETH_SER_H
#include "lwip/list.h"

/* data yang tersimpan dalam bridge buffer list */
struct bridge_data{
	struct list_head list;
	u16_t	len;
	u8_t	*payload;
};

err_t bridge_eth_ser_init();

err_t bridge_e2s_queue(struct pbuf *p);

struct bridge_data *bridge_e2s_dequeue();

err_t bridge_data_destroy(struct bridge_data *bd);

void bridge_print_pbuf(struct pbuf *p);


#endif
