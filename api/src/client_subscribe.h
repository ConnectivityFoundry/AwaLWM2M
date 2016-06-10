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


#ifndef CLIENT_SUBSCRIBE_H
#define CLIENT_SUBSCRIBE_H

#include <stdbool.h>

#include "awa/client.h"
#include "ipc.h"
#include "map.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _OperationCommon OperationCommon;
typedef struct _AwaClientSubscription AwaClientSubscription;

AwaError ClientNotification_Process(AwaClientSession * session, IPCMessage * notification);

AwaError ClientSubscribe_CallSubscribers(AwaClientSession * session, TreeNode contentNode);

OperationCommon * ClientSubscribeOperation_GetOperationCommon(const AwaClientSubscribeOperation * operation);

MapType * ClientSubscribeOperation_GetSubscribers(const AwaClientSubscribeOperation * operation);

ListType * ClientSubscription_GetOperations(AwaClientSubscription * subscription);

void ClientSubscription_RemoveSession(AwaClientSubscription * subscription);

#ifdef __cplusplus
}
#endif

#endif // CLIENT_SUBSCRIBE_H
