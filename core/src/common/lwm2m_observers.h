/************************************************************************************************************************
 Copyright (c) 2016, Imagination Technologies Limited and/or its affiliated group companies.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 following conditions are met:
     1. Redistributions of source code must retain the above copyright notice, this list of conditions and the
        following disclaimer.
     2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
        following disclaimer in the documentation and/or other materials provided with the distribution.
     3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote
        products derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
************************************************************************************************************************/


#ifndef LWM2M_OBSERVERS_H
#define LWM2M_OBSERVERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lwm2m_types.h"
#include "lwm2m_attributes.h"
#include "lwm2m_list.h"

typedef int (*Lwm2mNotificationCallback)(void * context, AddressType *, int, const char *, int, ObjectIDType, ObjectInstanceIDType, ResourceIDType, AwaContentType, void * ContextData);

typedef struct
{
    struct ListHead list;
    uint32_t LastUpdate;
    ObjectIDType ObjectID;
    ObjectInstanceIDType ObjectInstanceID;
    ResourceIDType ResourceID;
    AwaContentType ContentType;
    AddressType Address;
    Lwm2mNotificationCallback Callback;
    void * ContextData;
    bool Changed;
    char Token[8];                         // CoAP message token for notification
    int TokenLength;                       // Length of CoAP message token
    int Sequence;
    void * OldValue;                       // For Integer and Float datatypes only, used for notification attributes.
    size_t OldValueLength;
} Lwm2mObserverType;

// Send out pending notifications to any observers of objects, object instances and resources.
void Lwm2m_UpdateObservers(void * ctxt);

void Lwm2m_FreeObservers(void * ctxt);

// Set the changed bit for each observer in the observerList that matches the specified object / instance / resource
void Lwm2m_MarkObserversChanged(void * ctxt, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, const void * newValue, size_t newValueLength);

int Lwm2m_Observe(void * ctxt, AddressType * addr, const char * token, int tokenLength, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID,
                  ResourceIDType resourceID, AwaContentType contentType, Lwm2mNotificationCallback callback, void * ContextData);
int Lwm2m_CancelObserve(void * ctxt, AddressType * addr, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);

/* Free any observers in the observer list. this is called when a DELETE operation occurs for a specified object instance/resource.
 * Note that this will happen silently and will not notify the watcher that this has happened.
 */
int Lwm2m_RemoveAllObserversForOIR(void * ctxt, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);

#ifdef __cplusplus
}
#endif

#endif // LWM2M_OBSERVERS_H
