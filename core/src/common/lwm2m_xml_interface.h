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


#ifndef LWM2M_XML_INTERFACE_H
#define LWM2M_XML_INTERFACE_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "lwm2m_object_store.h"
#include "lwm2m_types.h"
#include "lwm2m_definition.h"
#include "xmltree.h"
#include "ipc_session.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    int Sockfd;
    struct sockaddr FromAddr;
    int AddrLen;
    IPCSessionID SessionID;
    void * Context;
    void * Client;
} RequestInfoType;


typedef int (*XmlRequestHandler)(RequestInfoType *, TreeNode);

int xmlif_AddRequestHandler(const char * msgType, XmlRequestHandler handler);

char * xmlif_EncodeValue(AwaResourceType dataType, const char * buffer, int bufferLength);
int xmlif_DecodeValue(char ** dataValue, AwaResourceType dataType, const char * buffer, int bufferLength);

// Wrapper for socket.h sendto, for easy debugging
ssize_t xmlif_SendTo(int sockfd, const void *buf, size_t len, int flags,
                     const struct sockaddr *dest_addr, socklen_t addrlen);

// Initialise XML interface
int xmlif_init(void * context, int port);

// Blocking call to process data on the XML interface socket
int xmlif_process(int sockfd);

void xmlif_destroy(int sockfd);

TreeNode xmlif_GenerateConnectResponse(DefinitionRegistry * definitionRegistry, IPCSessionID sessionID);

TreeNode xmlif_ConstructObjectDefinitionNode(const DefinitionRegistry * definitions, const ObjectDefinition * objFormat, int objectID);

int xmlif_RegisterObjectFromIPCXML(Lwm2mContextType * context,
                                   TreeNode objectMetadataNode,
                                   ObjectOperationHandlers * objectOperationHandlers,
                                   ResourceOperationHandlers * resourceOperationHandlers,
                                   ResourceOperationHandlers * executeOperationHandlers);

typedef struct {
    size_t NumObjectsOK;
    size_t NumObjectsFailed;
    size_t NumResourcesOK;
    size_t NumResourcesFailed;
} DefinitionCount;

DefinitionCount xmlif_RegisterObjectFromDeviceServerXML(Lwm2mContextType * context,
                                                        TreeNode objectDefinitionNode,
                                                        ObjectOperationHandlers * objectOperationHandlers,
                                                        ResourceOperationHandlers * resourceOperationHandlers,
                                                        ResourceOperationHandlers * executeOperationHandlers);

#ifdef __cplusplus
}
#endif

#endif // LWM2M_XML_INTERFACE_H
