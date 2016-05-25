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

#include "ipc_session.h"

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "lwm2m_debug.h"
#include "lwm2m_list.h"
#include "lwm2m_util.h"

// This number is used to space out session IDs. It is multiplied by the process ID to generate the Session ID.
// It is fairly arbitrary but large enough to provide a reasonable numerical distance between adjacent process IDs.
#define SUITABLY_LARGE_NUMBER (7487)

typedef struct
{
    int Sockfd;
    struct sockaddr FromAddr;
    int AddrLen;

} IPCChannel;

struct _IPCSession
{
    struct ListHead list;
    IPCSessionID SessionID;
    IPCChannel RequestChannel;
    IPCChannel NotifyChannel;
};

static struct ListHead sessionList;


void IPCSession_Init(void)
{
    ListInit(&sessionList);
}

void IPCSession_Shutdown(void)
{
    struct ListHead * i, * n;
    ListForEachSafe(i, n, &sessionList)
    {
        IPCSession * session = ListEntry(i, IPCSession, list);
        if (session != NULL)
        {
            free(session);
        }
    }
}

static IPCSession * FindSessionByID(IPCSessionID sessionID)
{
    IPCSession * result = NULL;
    struct ListHead * i;
    ListForEach(i, &sessionList)
    {
        IPCSession * session = ListEntry(i, IPCSession, list);
        if (session != NULL)
        {
            if (session->SessionID == sessionID)
            {
                result = session;
                break;
            }
        }
    }
    return result;
}

int IPCSession_New(IPCSessionID sessionID)
{
    int result = -1;
    if (FindSessionByID(sessionID) == NULL)
    {
        // add new session record
        IPCSession * session = malloc(sizeof(*session));
        if (session != NULL)
        {
            memset(session, 0, sizeof(*session));
            session->SessionID = sessionID;
            ListAdd(&session->list, &sessionList);
            result = 0;
        }
        else
        {
            Lwm2m_Error("Out of memory\n");
            result = -1;
        }
    }
    else
    {
        Lwm2m_Error("Session ID %d is already in use\n", sessionID);
        result = -1;
    }
    return result;
}

static void AddChannel(IPCChannel * channel, int sockfd, const struct sockaddr * fromAddr, int addrLen)
{
    channel->Sockfd = sockfd;
    channel->FromAddr = *fromAddr;
    channel->AddrLen = addrLen;
}

int IPCSession_AddRequestChannel(IPCSessionID sessionID, int sockfd, const struct sockaddr * fromAddr, int addrLen)
{
    int result = -1;
    IPCSession * session = NULL;
    if ((session = FindSessionByID(sessionID)) != NULL)
    {
        AddChannel(&session->RequestChannel, sockfd, fromAddr, addrLen);
        result = 0;
    }
    else
    {
        Lwm2m_Error("No session with ID %d found\n", sessionID);
        result = -1;
    }
    return result;
}

int IPCSession_GetRequestChannel(IPCSessionID sessionID, int * sockfd, const struct sockaddr ** fromAddr, int * addrLen)
{
    int result = -1;
    IPCSession * session = NULL;
    if ((session = FindSessionByID(sessionID)) != NULL)
    {
        if ((sockfd != NULL) && (fromAddr != NULL) && (addrLen != NULL))
        {
            *sockfd = session->RequestChannel.Sockfd;
            *fromAddr = &session->RequestChannel.FromAddr;
            *addrLen = session->RequestChannel.AddrLen;
            result = 0;
        }
        else
        {
            Lwm2m_Error("NULL parameter - could not return notification channel information\n");
            result = -1;
        }
    }
    else
    {
        Lwm2m_Error("No session with ID %d found\n", sessionID);
        result = -1;
    }
    return result;
}

int IPCSession_AddNotifyChannel(IPCSessionID sessionID, int sockfd, const struct sockaddr * fromAddr, int addrLen)
{
    int result = -1;
    IPCSession * session = NULL;
    if ((session = FindSessionByID(sessionID)) != NULL)
    {
        AddChannel(&session->NotifyChannel, sockfd, fromAddr, addrLen);
        result = 0;
    }
    else
    {
        Lwm2m_Error("No session with ID %d found\n", sessionID);
        result = -1;
    }
    return result;
}


int IPCSession_GetNotifyChannel(IPCSessionID sessionID, int * sockfd, const struct sockaddr ** fromAddr, int * addrLen)
{
    int result = -1;
    IPCSession * session = NULL;
    if ((session = FindSessionByID(sessionID)) != NULL)
    {
        if ((sockfd != NULL) && (fromAddr != NULL) && (addrLen != NULL))
        {
            *sockfd = session->NotifyChannel.Sockfd;
            *fromAddr = &session->NotifyChannel.FromAddr;
            *addrLen = session->NotifyChannel.AddrLen;
            result = 0;
        }
        else
        {
            Lwm2m_Error("NULL parameter - could not return notification channel information\n");
            result = -1;
        }
    }
    else
    {
        Lwm2m_Error("No session with ID %d found\n", sessionID);
        result = -1;
    }
    return result;
}

IPCSessionID IPCSession_AssignSessionID(void)
{
    static IPCSessionID sessionID = -1;
    if (sessionID == -1)
    {
        sessionID = getpid() * SUITABLY_LARGE_NUMBER;
    }
    return sessionID++;
}

bool IPCSession_IsValid(IPCSessionID sessionID)
{
    return FindSessionByID(sessionID);
}

void IPCSession_Dump(void)
{
    struct ListHead * i;
    ListForEach(i, &sessionList)
    {
        IPCSession * session = ListEntry(i, IPCSession, list);
        if (session != NULL)
        {
            printf("Session ID %d:\n", session->SessionID);
#ifndef CONTIKI
            printf("  Request Channel: Sockfd %d, FromAddr %s, AddrLen %d\n", session->RequestChannel.Sockfd, Lwm2mCore_DebugPrintSockAddr(&session->RequestChannel.FromAddr), session->RequestChannel.AddrLen);
            printf("  Notify Channel: Sockfd %d, FromAddr %s, AddrLen %d\n", session->NotifyChannel.Sockfd, Lwm2mCore_DebugPrintSockAddr(&session->NotifyChannel.FromAddr), session->NotifyChannel.AddrLen);
#endif
        }
    }
}
