/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *      CoAP module for reliable transport
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include "string.h"
#include "../common/lwm2m_list.h"
#include "er-coap-transactions.h"

/*---------------------------------------------------------------------------*/
//MEMB(transactions_memb, coap_transaction_t, COAP_MAX_OPEN_TRANSACTIONS);
//LIST(transactions_list);


struct ListHead transactions_list = {0};

//static struct process *transaction_handler_process = NULL;

/*---------------------------------------------------------------------------*/
/*- Internal API ------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
//void
//coap_register_as_transaction_handler()
//{
//  transaction_handler_process = PROCESS_CURRENT();
//}

void coap_init_transactions(void)
{
	ListInit(&transactions_list);
}

coap_transaction_t * coap_new_transaction(NetworkSocket * networkSocket, uint16_t mid, NetworkAddress * remoteAddress)
{
    coap_transaction_t * t = (coap_transaction_t *)malloc(sizeof(*t)); //memb_alloc(&transactions_memb);
    if(t)
    {
        memset(t, 0, sizeof(coap_transaction_t));
        t->mid = mid;
        t->retrans_counter = 0;
        t->networkSocket = networkSocket;
        t->remoteAddress = remoteAddress;

        ListAdd(&t->list, &transactions_list); /* list itself makes sure same element is not added twice */
    }

    return t;
}
/*---------------------------------------------------------------------------*/
void coap_send_transaction(coap_transaction_t *t)
{
    PRINTF("Sending transaction %u\n", t->mid);


    //coap_send_message(&t->addr, t->port, t->packet, t->packet_len);
    if (NetworkSocket_Send(t->networkSocket, t->remoteAddress, t->packet, t->packet_len))
    {
        t->sent = true;
        if(COAP_TYPE_CON ==
                ((COAP_HEADER_TYPE_MASK & t->packet[0]) >> COAP_HEADER_TYPE_POSITION))
        {
            if(t->retrans_counter < COAP_MAX_RETRANSMIT)
            {
                /* not timed out yet */
                PRINTF("Keeping transaction %u\n", t->mid);

                if(t->retrans_counter == 0)
                {
    //                t->retrans_timer.timer.interval =
    //                        COAP_RESPONSE_TIMEOUT_TICKS + (random_rand()
    //                                %
    //                                (clock_time_t)
    //                                COAP_RESPONSE_TIMEOUT_BACKOFF_MASK);
    //                PRINTF("Initial interval %f\n",
    //                        (float)t->retrans_timer.timer.interval / CLOCK_SECOND);
                }
                else
                {
    //                t->retrans_timer.timer.interval <<= 1;  /* double */
    //                PRINTF("Doubled (%u) interval %f\n", t->retrans_counter,
    //                        (float)t->retrans_timer.timer.interval / CLOCK_SECOND);
                }

    //            PROCESS_CONTEXT_BEGIN(transaction_handler_process);
    //            etimer_restart(&t->retrans_timer);        /* interval updated above */
    //            PROCESS_CONTEXT_END(transaction_handler_process);

                t = NULL;
            }
            else
            {
                /* timed out */
                PRINTF("Timeout\n");
                restful_response_handler callback = t->callback;
                void *callback_data = t->callback_data;

                /* handle observers */
                //coap_remove_observer_by_client(t->session);	// TODO - restore when observe supported

                coap_clear_transaction(&t);

                if(callback)
                {
                    callback(callback_data, NULL);
                }
            }
        } else {
            coap_clear_transaction(&t);
        }
    }
    else
    {
        PRINTF("Failed to sened transaciton %u\n", t->mid);
        t->sent = false;
    }
}
/*---------------------------------------------------------------------------*/
void coap_clear_transaction(coap_transaction_t **t)
{
    if(t && *t)
    {
        PRINTF("Freeing transaction %u: %p\n", (*t)->mid, (*t));

        //etimer_stop(&t->retrans_timer);
        ListRemove(&(*t)->list);
        free(*t);
        *t = NULL;
    }
}

coap_transaction_t * coap_get_transaction_by_mid(uint16_t mid)
{
    struct ListHead * i = NULL;

    ListForEach(i, &transactions_list)
    {
        struct coap_transaction * t = ListEntry(i, struct coap_transaction, list);

        if(t->mid == mid) {
            PRINTF("Found transaction for MID %u: %p\n", t->mid, t);
            return t;
        }
    }
    return NULL;
}
/*---------------------------------------------------------------------------*/
void coap_check_transactions()
{
    struct ListHead * current = NULL;
    struct ListHead * next = NULL;

    ListForEachSafe(current ,next, &transactions_list)
    {

        coap_transaction_t *t = ListEntry(current, struct coap_transaction, list);

        //if(t->retrans_timer == 0)
        if (!t->sent)
        {
            //++(t->retrans_counter);
            //PRINTF("Retransmitting %u (%u)\n", t->mid, t->retrans_counter);
            coap_send_transaction(t);
        }
    }

}
/*---------------------------------------------------------------------------*/
