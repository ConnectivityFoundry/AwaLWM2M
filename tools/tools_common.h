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


#ifndef TOOLS_COMMON_H
#define TOOLS_COMMON_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "awa/client.h"
#include "awa/server.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __GNUC__
#  define __attribute__(X)
#endif // __GNUC__


// IPC timeout (milliseconds)
#define OPERATION_PERFORM_TIMEOUT        (5000)

#define FLOW_DEVICE_MGMT_KEY_ID_NOT_USED (-1)
#define FLOW_DEVICE_MGMT_KEY_ID_MAX      (65535)

#define FLOW_OBJECT                      (20000)
#define FLOW_ACCESS                      (20001)


extern int g_logLevel;
extern bool g_signal;


// Used to keep track of a command-line target item
typedef struct
{
    char * Path;
    int ResourceInstanceID;
} Target;

// Used to determine which type of values-related response has been received
typedef enum
{
    ResponseType_GetResponse = 0,
    ResponseType_ChangeSet,
    ResponseType_ReadResponse,
} ResponseType;

//Format of output
typedef enum
{
    OutputFormat_None = 0,
    OutputFormat_PlainTextVerbose,
    OutputFormat_PlainTextQuiet,
    OutputFormat_DeviceServerXML
} OutputFormat;


// Logging Support:

// Level 1 includes both warnings on stderr and verbose output on stdout
#define Error(...)   Log(0, stderr, __VA_ARGS__)
#define Warning(...) Log(1, stderr, __VA_ARGS__)
#define Verbose(...) Log(1, stdout, __VA_ARGS__)
#define Debug(...)   Log(2, stdout, __VA_ARGS__)

void Log(int level, FILE * outFd, const char * format, ...) __attribute__ ((format (printf, 3, 4)));


// Signal handler
void INThandler(int sig);

const char * OirToPath(char * path, size_t pathLen, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID);

Target * CreateTarget(const char * arg);

void FreeTarget(Target ** targetPtr);

bool IsIDValid(int id);

const char * ResourceTypeToString(AwaResourceType type);
const char * DeviceServerXML_ResourceTypeToString(AwaResourceType type);
const char * ResourceOperationToString(AwaResourceOperations operation);
const char * DeviceServerXML_ResourceOperationToString(AwaResourceOperations operation);

bool IsArrayType(AwaResourceType type);

bool IsTargetValid(const char * target);


// Client Session functions
AwaClientSession * Client_EstablishSession(const char * address, unsigned int port);
void Client_ReleaseSession(AwaClientSession ** session);

char * Client_GetValue(const AwaClientSession * session, const Target * target, const char * arg);
bool Client_IsObjectTarget(const AwaClientSession * session, const Target * target);
bool Client_IsObjectInstanceTarget(const AwaClientSession * session, const Target * target);
bool Client_IsResourceTarget(const AwaClientSession * session, const Target * target);
bool Client_IsResourceInstanceTarget(const AwaClientSession * session, const Target * target);

bool Client_IsResourceDefined(const AwaClientSession * session, const char * path);
const AwaResourceDefinition * Client_GetResourceDefinition(const AwaClientSession * session, const char * path);
AwaResourceType Client_GetResourceType(const AwaClientSession * session, const char * path);
AwaResourceOperations Client_GetResourceOperations(const AwaClientSession * session, const char * path);
bool Client_IsMultiValuedResource(const AwaClientSession * session, const char * path);

int Client_AddPathToCString(char ** cstring, const char * path, const AwaClientSession * session,
                            void * response, ResponseType responseType, bool quiet,
                            AwaObjectID * lastObjectID, AwaObjectInstanceID * lastObjectInstanceID);


// Server Session functions
AwaServerSession * Server_EstablishSession(const char * address, unsigned int port);
void Server_ReleaseSession(AwaServerSession ** session);

char * Server_GetValue(const AwaServerSession * session, const Target * target, const char * arg);
bool Server_IsObjectTarget(const AwaServerSession * session, const Target * target);
bool Server_IsObjectInstanceTarget(const AwaServerSession * session, const Target * target);
bool Server_IsResourceTarget(const AwaServerSession * session, const Target * target);
bool Server_IsResourceInstanceTarget(const AwaServerSession * session, const Target * target);

bool Server_IsResourceDefined(const AwaServerSession * session, const char * path);
const AwaResourceDefinition * Server_GetResourceDefinition(const AwaServerSession * session, const char * path);
AwaResourceType Server_GetResourceType(const AwaServerSession * session, const char * path);

int Server_AddPathToCString(char ** cstring, const char * path, const AwaServerSession * session,
                            void * response, ResponseType responseType, bool quiet,
                            AwaObjectID * lastObjectID, AwaObjectInstanceID * lastObjectInstanceID, int resourceInstanceID);

int GetNextTargetResourceInstanceIDFromPath(Target ** targets, int numTargets, const char * path, int * index);


// Definition Printing functions
void PrintAllObjectDefinitions(AwaObjectDefinitionIterator * defineIterator, OutputFormat format);
void PrintDefinitionTarget(const AwaObjectDefinition * objectDefinition, OutputFormat format, AwaObjectID objectID, AwaResourceID resourceID, AwaObjectID * lastObjectIDPrinted);
void PrintObjectDefinitionHeader(const AwaObjectDefinition * objectDefinition, OutputFormat format);
void PrintObjectDefinitionFooter(const AwaObjectDefinition * objectDefinition, OutputFormat outputFormat);
void PrintObjectDefinition(const AwaObjectDefinition * objectDefinition, OutputFormat format);
void PrintResourceDefinition(const AwaResourceDefinition * resourceDefinition, OutputFormat format, AwaObjectID objectID);


#ifdef __cplusplus
}
#endif

#endif // TOOLS_COMMON_H
