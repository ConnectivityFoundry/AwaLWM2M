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


#ifndef LWM2M_SERDES_H
#define LWM2M_SERDES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "lwm2m_core.h"
#include "lwm2m_definition.h"
#include "lwm2m_types.h"
#include "lwm2m_tree_node.h"

typedef void * SerdesContext;

typedef struct
{
    int (*SerialiseObject)(SerdesContext * serdesContext, Lwm2mTreeNode * node, ObjectIDType objectID, uint8_t * buffer, int len);
    int (*SerialiseObjectInstance)(SerdesContext * serdesContext, Lwm2mTreeNode * node, ObjectIDType objectID,
                                   ObjectInstanceIDType objectInstanceID, uint8_t * buffer, int len);
    int (*SerialiseResource)(SerdesContext * serdesContext, Lwm2mTreeNode * node, ObjectIDType objectID,
                             ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, uint8_t * buffer, int len);

    int (*DeserialiseObject)(SerdesContext * serdesContext, Lwm2mTreeNode ** dest, const DefinitionRegistry * registry, ObjectIDType objectID, const uint8_t * buffer, int len);
    int (*DeserialiseObjectInstance)(SerdesContext * serdesContext, Lwm2mTreeNode ** dest, const DefinitionRegistry * registry,
                                     ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, const uint8_t * buffer, int len);
    int (*DeserialiseResource)(SerdesContext * serdesContext, Lwm2mTreeNode ** dest,  const DefinitionRegistry * registry,
                               ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, const uint8_t * buffer, int len);

} SerialiserDeserialiser;

int SerialiseObject(AwaContentType type, Lwm2mTreeNode * node, ObjectIDType objectID, char * buffer, int len);
int SerialiseObjectInstance(AwaContentType type, Lwm2mTreeNode * node, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, char * buffer, int len);
int SerialiseResource(AwaContentType type, Lwm2mTreeNode * node, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, char * buffer, int len);

int DeserialiseObject(AwaContentType type, Lwm2mTreeNode ** dest, const DefinitionRegistry * registry, ObjectIDType objectID, const char * buffer, int bufferLen);
int DeserialiseObjectInstance(AwaContentType type, Lwm2mTreeNode ** dest, const DefinitionRegistry * registry, ObjectIDType objectID,
                              ObjectInstanceIDType objectInstanceID, const char * buffer, int bufferLen);
int DeserialiseResource(AwaContentType type, Lwm2mTreeNode ** dest, const DefinitionRegistry * registry, ObjectIDType objectID,
                        ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, const char * buffer, int bufferLen);

#ifdef __cplusplus
}
#endif

#endif // LWM2M_SERDES_H

