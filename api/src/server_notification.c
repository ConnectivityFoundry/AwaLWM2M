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


#include <stdio.h>
#include <stdlib.h>

#include "server_notification.h"
#include "awa/server.h"
#include "awa/common.h"
#include "log.h"
#include "memalloc.h"
#include "path_iterator.h"
#include "path.h"
#include "utils.h"
#include "arrays.h"
#include "ipc.h"
#include "xml.h"
#include "observe_operation.h"

AwaError ServerNotification_Process(AwaServerSession * session, IPCMessage * notification)
{
    AwaError result = AwaError_Success;
    if (notification)
    {
        TreeNode contentNode = IPCMessage_GetContentNode(notification);
        if (contentNode != NULL)
        {
            TreeNode clientsNode = Xml_Find(contentNode, "Clients");
            if (clientsNode != NULL)
            {
                result = ServerObserveOperation_CallObservers(session, clientsNode);
            }
            else
            {
                result = LogErrorWithEnum(AwaError_IPCError, "Clients node not found");
            }
        }
        else
        {
            result = LogErrorWithEnum(AwaError_IPCError, "Content node not found");
        }
    }
    return result;
}


