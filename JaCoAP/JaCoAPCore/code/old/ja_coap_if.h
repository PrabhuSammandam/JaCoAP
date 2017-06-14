#ifndef __JA_COAP_IF_H__
#define __JA_COAP_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "ja_types.h"

	typedef struct ja_coap_ep_  ja_coap_ep_t;

	typedef void( *pfn_ep_data_received ) (pu8 pu8_buf, pu16 pu16_buf_len, ja_coap_ep_t * p_remote_ep);

	typedef struct ja_coap_net_addr_ip6_ {
		u8  byte[16];
		u16 word[8];
		u32 dword[4];
	} ja_coap_net_addr_ip6_t;

	typedef struct ja_coap_net_addr_ip4_ {
		u8  byte[4];
		u16 word[2];
		u32 dword[1];
	} ja_coap_net_addr_ip4_t;

	typedef struct ja_coap_net_addr_ {
		union {
			ja_coap_net_addr_ip6_t  ip6;
			ja_coap_net_addr_ip4_t  ip4;
		} net_addr;
		u8  type;
	} ja_coap_net_addr_t;

	typedef struct ja_coap_ep_ {
		ja_coap_net_addr_t      addr;
		u16                     port;
		pfn_ep_data_received    data_received_cb;
	} ja_coap_ep_t;

#define JA_COAP_EP_INVALID_HNDL 0xFFFF

	unsigned char   ja_port_init( );
	unsigned char   ja_port_deinit( );

	u8              ja_port_create_endpoint( ja_coap_ep_t* p_endpoint, pu16 pu16_ep_hndl );
	u8              ja_port_read_data();

	void*           ja_coap_port_mem_alloc( u16 mem_size );
	void            ja_coap_port_mem_free( void* pv_mem_to_free );
	void*           ja_coap_port_mem_alloc_zero( u16 mem_size );
	u8              ja_coap_port_memcpy( void* dst, void* src, u16 length );

#ifdef __cplusplus
}
#endif

#endif /*__JA_COAP_IF_H__*/
