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


#include <stddef.h>
#include <stdlib.h>

#include "lwm2m_debug.h"
#include "lwm2m_serdes.h"
#include "lwm2m_prettyprint.h"
#include "lwm2m_tlv.h"
#include "lwm2m_plaintext.h"
#ifndef CONTIKI
  #include "lwm2m_json.h"
#endif
#include "lwm2m_opaque.h"

typedef struct
{
    AwaContentType Type;
    const SerialiserDeserialiser * Serdes;
} SerdesMapEntry;

/**
 * Map content type to serialiser/deserialisers
 */
const static SerdesMapEntry serdesList[] =
{

#ifndef CONTIKI
#ifdef WITH_JSON
        { AwaContentType_ApplicationOmaLwm2mJson,   &jsonSerDes      },
#endif // WITH_JSON
#endif // CONTIKI
        { AwaContentType_ApplicationOmaLwm2mTLV,    &tlvSerDes       },
        { AwaContentType_ApplicationPlainText,      &plainTextSerDes },
        { AwaContentType_ApplicationOctetStream,    &opaqueSerDes    },

        // Mapping for old types
        { AwaContentType_ApplicationOmaLwm2mText,   &plainTextSerDes },
#ifndef CONTIKI
#ifdef WITH_JSON
        { AwaContentType_ApplicationJson,           &jsonSerDes      },
#endif // WITH_JSON
#endif // CONTIKI
        { AwaContentType_ApplicationOmaLwm2mOpaque, &opaqueSerDes    },
};
#define NUM_SERIALISERS (sizeof(serdesList)/sizeof(SerdesMapEntry))

static SerialiserDeserialiser * GetSerialiserDeserialiser(AwaContentType type)
{
    int i;
    // lookup serialiser in serialiser list.
    for (i = 0; i < NUM_SERIALISERS; i++)
    {
        if (serdesList[i].Type == type)
            return (SerialiserDeserialiser*)serdesList[i].Serdes;
    }
    return NULL;
}

int SerialiseObject(AwaContentType type, Lwm2mTreeNode * node, ObjectIDType objectID, char * buffer, int len)
{
    SerialiserDeserialiser * serdes = GetSerialiserDeserialiser(type);
    if ((serdes != NULL) && serdes->SerialiseObject)
    {
        SerdesContext serdesContext = NULL;
        return serdes->SerialiseObject(&serdesContext, node, objectID, (uint8_t*)buffer, len);
    }
    Lwm2m_Error("Serialiser not found for type %d\n", type);
    return -1;
}

int SerialiseObjectInstance(AwaContentType type, Lwm2mTreeNode * node, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, char * buffer, int len)
{
    SerialiserDeserialiser * serdes = GetSerialiserDeserialiser(type);
    if ((serdes != NULL) && serdes->SerialiseObjectInstance)
    {
        SerdesContext serdesContext = NULL;
        return serdes->SerialiseObjectInstance(&serdesContext, node, objectID, objectInstanceID, (uint8_t*)buffer, len);
    }
    Lwm2m_Error("Serialiser not found for type %d\n", type);
    return -1;
}

int SerialiseResource(AwaContentType type, Lwm2mTreeNode * node, ObjectIDType objectID,
                      ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, char * buffer, int len)
{
    SerialiserDeserialiser * serdes = GetSerialiserDeserialiser(type);
    if ((serdes != NULL) && serdes->SerialiseResource)
    {
        SerdesContext serdesContext = NULL;
        return serdes->SerialiseResource(&serdesContext, node, objectID, objectInstanceID, resourceID, (uint8_t*)buffer, len);
    }
    Lwm2m_Error("Serialiser not found for type %d\n", type);
    return -1;
}

int DeserialiseObject(AwaContentType type, Lwm2mTreeNode ** dest, const DefinitionRegistry * registry,
                      ObjectIDType objectID, const char * buffer, int bufferLen)
{
    SerialiserDeserialiser * serdes = GetSerialiserDeserialiser(type);
    if ((serdes != NULL) && serdes->DeserialiseObject)
    {
        SerdesContext serdesContext = NULL;
        return serdes->DeserialiseObject(&serdesContext, dest, registry, objectID, (const uint8_t*)buffer, bufferLen);
    }
    Lwm2m_Error("Deserialiser not found for type %d\n", type);
    return -1;
}

int DeserialiseObjectInstance(int type, Lwm2mTreeNode ** dest, const DefinitionRegistry * registry, ObjectIDType objectID,
                              ObjectInstanceIDType objectInstanceID, const char * buffer, int bufferLen)
{
    SerialiserDeserialiser * serdes = GetSerialiserDeserialiser(type);
    if ((serdes != NULL) && serdes->DeserialiseObjectInstance)
    {
        SerdesContext serdesContext = NULL;
        return serdes->DeserialiseObjectInstance(&serdesContext, dest, registry, objectID, objectInstanceID, (const uint8_t*)buffer, bufferLen);
    }
    Lwm2m_Error("Deserialiser not found for type %d\n", type);
    return -1;
}

int DeserialiseResource(int type, Lwm2mTreeNode ** dest, const DefinitionRegistry * registry, ObjectIDType objectID,
                        ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID, const char * buffer, int bufferLen)
{
    SerialiserDeserialiser * serdes = GetSerialiserDeserialiser(type);
    if ((serdes != NULL) && serdes->DeserialiseResource)
    {
        SerdesContext serdesContext = NULL;
        return serdes->DeserialiseResource(&serdesContext, dest, registry, objectID, objectInstanceID, resourceID, (const uint8_t*)buffer, bufferLen);
    }
    Lwm2m_Error("Deserialiser not found for type %d\n", type);
    return -1;
}

