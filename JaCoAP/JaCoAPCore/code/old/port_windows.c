#include <stdio.h>
#include <winsock2.h>

#include "ja_types.h"
#include "ja_coap_if.h"

#pragma comment(lib, "Ws2_32.lib")
WSADATA         wsa;

#define DEFAULT_BUFLEN  512
#define PORT_MAX_EP     1

typedef struct _port_ep_
{
	SOCKET                  s;
	u8                      used;
	pfn_ep_data_received    data_received_cb;
} _port_ep_t;

static u8   ja_port_intl_get_free_ep(void);
static void ja_port_intl_init_ep(_port_ep_t* p_prot_ep);

_port_ep_t  gs_port_ep_list[PORT_MAX_EP];
u16         gs_port_ep_count = 0;
char        gs_buffer[DEFAULT_BUFLEN];

unsigned char ja_port_init(void)
{
	printf("\nInitialising Winsock...");

	if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 0;
	}

	printf("Initialised.\n");

	for(u8 i = 0; i < PORT_MAX_EP; i++) {
		ja_port_intl_init_ep(&gs_port_ep_list[i]);
	}

	gs_port_ep_count = 0;

	return 1;
}

u8 ja_port_create_endpoint(ja_coap_ep_t*   p_endpoint,
						   pu16            pu16_ep_hndl)
{
	SOCKET              s;
	struct sockaddr_in  server;
	u8                  free_ep_index;

	*pu16_ep_hndl = JA_COAP_EP_INVALID_HNDL;

	free_ep_index = ja_port_intl_get_free_ep();

	if(free_ep_index == 0xFF) {
		return 0;
	}

	ja_port_intl_init_ep(&gs_port_ep_list[free_ep_index]);

	//Create a socket
	if((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		printf("Could not create socket : %d", WSAGetLastError());

		return 0;
	}

	printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(p_endpoint->port);

	//Bind
	if(bind(s, (struct sockaddr*) &server, sizeof(server)) == SOCKET_ERROR) {
		printf("Bind failed with error code : %d", WSAGetLastError());
		return 0;
	}

	puts("Bind done");

	gs_port_ep_list[free_ep_index].s = s;
	gs_port_ep_list[free_ep_index].used = 1;
	gs_port_ep_list[free_ep_index].data_received_cb = p_endpoint->data_received_cb;
	*pu16_ep_hndl = free_ep_index;

	return 1;
}

u8 ja_port_read_data(void)
{
	int                 stat = 1;
	int                 slen;
	int                 time_out = 1000;
	int                 recv_len;
	struct sockaddr_in  si_other;

	slen = sizeof(si_other);

	memset(&gs_buffer[0], '\0', DEFAULT_BUFLEN);

	// Set Timeout for recv call
	if(setsockopt(gs_port_ep_list[0].s, SOL_SOCKET, SO_RCVTIMEO, (const char*) &time_out, sizeof(time_out))) {
		return 1;
	}

	if((recv_len = recvfrom(gs_port_ep_list[0].s, gs_buffer, DEFAULT_BUFLEN, 0, (struct sockaddr*) &si_other, &slen)) == SOCKET_ERROR) {

		if(WSAETIMEDOUT == WSAGetLastError()) {
			//printf( "socket recv timeout\n");
			stat = 1;
		}
		else {
			stat = 0;
			printf("socket receive error happened, error code %d\n", WSAGetLastError());
		}
	}

	if(stat && (recv_len > 0) && gs_port_ep_list[0].data_received_cb) {
		ja_coap_ep_t ep;

		ep.port = si_other.sin_port;
		ep.addr.net_addr.ip4.dword[0] = ntohl(si_other.sin_addr.S_un.S_addr);
		gs_port_ep_list[0].data_received_cb( &gs_buffer[0], (pu16)&recv_len, &ep);
	}

	return stat;
}

unsigned char ja_port_deinit(void)
{
	for(u8 i = 0; i < PORT_MAX_EP; i++) {
		if(gs_port_ep_list[i].used) {
			closesocket(gs_port_ep_list[i].s);
		}
	}

	printf("\nDeInitialising Winsock...");

	WSACleanup();

	printf("DeInitialised.\n");

	return 1;
}

static u8 ja_port_intl_get_free_ep(void)
{
	for(u8 i = 0; i < PORT_MAX_EP; i++) {
		if(!gs_port_ep_list[i].used) {
			return i;
		}
	}

	return 0xFF;
}

static void ja_port_intl_init_ep(_port_ep_t*     p_prot_ep)
{
	p_prot_ep->used = 0;
	p_prot_ep->data_received_cb = 0;
}

/************************MEMORY*****************************/
void* ja_coap_port_mem_alloc(u16 mem_size)
{
	void*   mem = malloc(mem_size);

	if(!mem) {
		printf("Failed to allocate memory");
		return 0;
	}

	return mem;
}

void ja_coap_port_mem_free(void* pv_mem_to_free)
{
	free(pv_mem_to_free);
}

void* ja_coap_port_mem_alloc_zero(u16 mem_size)
{
	void*   mem = calloc(mem_size, 1);

	if(!mem) {
		printf("Failed to allocate memory");
		return 0;
	}

	return mem;
}

u8 ja_coap_port_memcpy(void* dst, void* src, u16 length)
{
	memcpy(dst, src, length);

	return 0;
}
