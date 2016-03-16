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

#include "types.h"
#include "error.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    AwaOperation_CreateObjectInstance,
    AwaOperation_DeleteObjectInstance,
    AwaOperation_Read,
    AwaOperation_Write,
    AwaOperation_Execute,
    AwaOperation_CreateResource,
    AwaOperation_DeleteResource,
} AwaOperation;

typedef enum
{
    AwaAccess_None = 0,
    AwaAccess_Read,
    AwaAccess_Write,
    AwaAccess_ReadWrite,
    AwaAccess_Execute,
} AwaAccess;

typedef struct _AwaStaticClient AwaStaticClient;

typedef struct
{
    struct
    {
        char ServerURI[255];
        bool Bootstrap;
        int SecurityMode;
        char PublicKey[255];
        char SecretKey[255];
        int ServerID;
        int HoldOffTime;
    } SecurityInfo;
  
    struct 
    {
        int ShortServerID;
        int LifeTime;
        int MinPeriod;
        int MaxPeriod;
        int DisableTimeout;
        bool Notification;
        char Binding[10];
    } ServerInfo;
} AwaFactoryBootstrapInfo;

typedef enum
{
    AwaLwm2mResult_Success = 200,
    AwaLwm2mResult_SuccessCreated = 201,
    AwaLwm2mResult_SuccessDeleted = 202,
    AwaLwm2mResult_SuccessChanged = 204,
    AwaLwm2mResult_SuccessContent = 205,

    AwaLwm2mResult_BadRequest = 400,
    AwaLwm2mResult_Unauthorized = 401,
    AwaLwm2mResult_Forbidden = 403,
    AwaLwm2mResult_NotFound = 404,
    AwaLwm2mResult_MethodNotAllowed = 405,

    AwaLwm2mResult_InternalError = 500,

    AwaLwm2mResult_OutOfMemory = 999,
    AwaLwm2mResult_AlreadyRegistered,
    AwaLwm2mResult_MismatchedRegistration,
    AwaLwm2mResult_AlreadyCreated,
    AwaLwm2mResult_Unsupported,
    AwaLwm2mResult_Unspecified = -1,

} AwaLwm2mResult;

typedef AwaLwm2mResult (*AwaStaticClientHandler)(AwaStaticClient * client, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, uint16_t * dataSize, bool * changed);

/************************************************************************************************************
 * Awa Static Client functions
 ************************************************************************************************************/

//Main process function
int AwaStaticClient_Process(AwaStaticClient * client);

//Initialisation functions
//AwaError AwaStaticClient_SetLogLevel(AwaStaticClient * client, DebugLevel level);
AwaError AwaStaticClient_SetEndPointName(AwaStaticClient * client, const char * endPointName);
AwaError AwaStaticClient_SetCOAPListenAddressPort(AwaStaticClient * client, const char * address, int port);
AwaError AwaStaticClient_SetBootstrapServerURI(AwaStaticClient * client, const char * bootstrapServerURI);
AwaError AwaStaticClient_SetFactoryBootstrapInformation(AwaStaticClient * client, const AwaFactoryBootstrapInfo * factoryBootstrapInformation);

AwaError AwaStaticClient_SetApplicationContext(AwaStaticClient * client, void * context);
void * AwaStaticClient_GetApplicationContext(AwaStaticClient * client);

AwaStaticClient * AwaStaticClient_New();
AwaError AwaStaticClient_Init(AwaStaticClient * client);
void AwaStaticClient_Free(AwaStaticClient ** client);

AwaError AwaStaticClient_RegisterObjectWithHandler(AwaStaticClient * client, const char * objectName, AwaObjectID objectID,
                                                   uint16_t minimumInstances, uint16_t maximumInstances,
                                                   AwaStaticClientHandler handler);

AwaError AwaStaticClient_RegisterObject(AwaStaticClient * client, const char * objectName, AwaObjectID objectID,
                                        uint16_t minimumInstances, uint16_t maximumInstances);

AwaError AwaStaticClient_RegisterResourceWithHandler(AwaStaticClient * client, const char * resourceName,
                                                     AwaObjectID objectID, AwaResourceID resourceID, AwaResourceType resourceType,
                                                     uint16_t minimumInstances, uint16_t maximumInstances, AwaAccess access,
                                                     AwaStaticClientHandler handler);

AwaError AwaStaticClient_RegisterResourceWithPointer(AwaStaticClient * client, const char * resourceName,
                                                     AwaObjectID objectID, AwaResourceID resourceID, AwaResourceType resourceType,
                                                     uint16_t minimumInstances, uint16_t maximumInstances, AwaAccess access,
                                                     void * dataPointer, size_t dataElementSize, size_t dataStepSize);

AwaError AwaStaticClient_RegisterResourceWithPointerArray(AwaStaticClient * client, const char * resourceName,
                                                          AwaObjectID objectID, AwaResourceID resourceID, AwaResourceType resourceType,
                                                          uint16_t minimumInstances, uint16_t maximumInstances, AwaAccess access,
                                                          void * dataPointers[], size_t dataElementSize);

void * AwaStaticClient_GetResourceInstancePointer(AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID);

AwaError AwaStaticClient_CreateResource(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID);
AwaError AwaStaticClient_CreateObjectInstance(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID);

AwaError AwaStaticClient_ResourceChanged(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID);
AwaError AwaStaticClient_ObjectInstanceChanged(AwaStaticClient * client, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID);

#ifdef __cplusplus
}
#endif

#endif /* AWA_STATIC_CLIENT_H */
