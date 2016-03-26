/**
 * \file
 *      CoAP session abstraction
 * \author
 *      Sean Kelly <seandtkelly@gmail.com>
 */

#ifndef CORE_SRC_ERBIUM_ER_SESSION_H_
#define CORE_SRC_ERBIUM_ER_SESSION_H_


#ifdef CONTIKI

#include "contiki-net.h"

typedef uip_ipaddr_t ipaddr_t;

#endif

#ifdef POSIX

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

typedef struct sockaddr_in ipaddr_t;

#endif


//typedef void(*session_send_data)(uint8_t * data)

typedef struct
{
    ipaddr_t ip_addr;
    uint16_t port;
    uint8_t * packet_buffer;
} coap_session;


void session_send_data(coap_session * session, size_t data_length);
void session_send_data_ptr(coap_session * session, uint8_t * data, size_t data_length);

#endif /* CORE_SRC_ERBIUM_ER_SESSION_H_ */
