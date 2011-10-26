#include "lwip/mem.h"

#include "bridge_helper.h"

#define PBUF_BUFLEN	1514
/**
 * build pbuf
 */

struct pbuf *bridge_pbuf_build(char *buf, u16_t len)
{
	struct pbuf *p, *q;
	char *bufptr;

	/* cek len */
	if (len > PBUF_BUFLEN) {
		printf("input too large\n");
		mem_free(buf);
		/* drop packet */
		return NULL;
	}

	/* Allocate a pbuf chain of pbufs from the pool. */
	p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
  
	if(p != NULL) {
    /* We iterate over the pbuf chain until we have read the entire
       packet into the pbuf. */
		bufptr = &buf[0];
		for(q = p; q != NULL; q = q->next) {
			/* Read enough bytes to fill this pbuf in the chain. The
			 * available data in the pbuf is given by the q->len
			 * variable. */
			/* read data into(q->payload, q->len); */
			memcpy(q->payload, bufptr, q->len);
			bufptr += q->len;	
			q->tot_len = len;
		}
	} else {
		/* drop packet(); */
	}

	return p;  
}
