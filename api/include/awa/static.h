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


#ifndef AWA_STATIC_H
#define AWA_STATIC_H

#include "awa/common.h"
#include "awa/error.h"

typedef enum
{
    AwaOperation_CreateObjectInstance,
    AwaOperation_DeleteObjectInstance,
    AwaOperation_WriteResourceInstance,
    AwaOperation_ReadResourceInstance,
    AwaOperation_DeleteResource,
    AwaOperation_ExecuteResource,
} AwaOperation;

typedef struct _AwaStaticClient AwaStaticClient;

typedef int (*AwaStaticClientHandler)(AwaStaticClient * client, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, uint16_t * dataSize, bool * changed);

/************************************************************************************************************
 * Awa Static Client functions
 ************************************************************************************************************/

//Main process function
int AwaStaticClient_Process(AwaStaticClient * core);

//Initialisation functions
AwaError AwaStaticClient_SetLogLevel(AwaStaticClient * client, DebugLevel level);
AwaError AwaStaticClient_SetEndPointName(AwaStaticClient * client, const char * EndPointName);
AwaError AwaStaticClient_SetCOAPPort(AwaStaticClient * client, int port);
AwaError AwaStaticClient_SetBootstrapServerURI(AwaStaticClient * client, const char * bootstrapServerURI);
AwaError AwaStaticClient_SetFactorBootstrapInformation(AwaStaticClient * client, const char * factoryBootstrapInformation);

AwaStaticClient * AwaStaticClient_New();
AwaError AwaStaticClient_Init(AwaStaticClient * client);

AwaError AwaStaticClient_RegisterObjectWithHandler(AwaStaticClient * client, const char * objectName, AwaObjectID objectID,
                                                     uint16_t minimumInstances, uint16_t maximumInstances,
                                                     AwaStaticClientHandler handler);

AwaError AwaStaticClient_RegisterObject(AwaStaticClient * client, const char * objectName, AwaObjectID objectID,
                                          uint16_t minimumInstances, uint16_t maximumInstances);

AwaError AwaStaticClient_RegisterResourceWithHandler(AwaStaticClient * client, const char * objectName,
                                                       AwaObjectID objectID, AwaResourceID resourceID, AwaResourceType resourceType,
                                                       uint16_t minimumInstances, uint16_t maximumInstances, Operations operations,
                                                       AwaStaticClientHandler handler);

AwaError AwaStaticClient_RegisterResourceWithUniformData(AwaStaticClient * client, const char * objectName,
                                                           AwaObjectID objectID, AwaResourceID resourceID, AwaResourceType resourceType,
                                                           uint16_t minimumInstances, uint16_t maximumInstances, Operations operations,
                                                           void * dataPointer, size_t dataElementSize, size_t dataStepSize);

AwaError AwaStaticClient_RegisterResourceWithArbitraryData(AwaStaticClient * client, const char * objectName,
                                                             AwaObjectID objectID, AwaResourceID resourceID, AwaResourceType resourceType,
                                                             uint16_t minimumInstances, uint16_t maximumInstances, Operations operations,
                                                             void * dataPointers[], size_t dataElementSize);

void * AwaStaticClient_GetResourceInstancePointer(AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID);

AwaError AwaStaticClient_CreateResource(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID);
AwaError AwaStaticClient_CreateObjectInstance(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstance);

AwaError AwaStaticClient_ResourceChanged(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID);
AwaError AwaStaticClient_ObjectInstanceChanged(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID);


#endif /* AWA_STATIC_CLIENT_H */
