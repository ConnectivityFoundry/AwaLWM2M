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

#include "static_api_support.h"

namespace Awa {

namespace global {


} // namespace global

namespace detail {

} // namespace detail

namespace StaticClient {
int staticClientProcessBootstrapTimeout = 10;

}

AwaResult handler(AwaStaticClient * context, AwaOperation operation, AwaObjectID objectID, AwaObjectInstanceID objectInstanceID, AwaResourceID resourceID, AwaResourceInstanceID resourceInstanceID, void ** dataPointer, size_t * dataSize, bool * changed)
{
    AwaStaticClient * client = (AwaStaticClient *)context;
    AwaResult result = AwaResult_InternalError;

    if (global::logLevel == AwaLogLevel_Debug)
        std::cout << "Handler for " << operation << std::endl;

    void * callback = AwaStaticClient_GetApplicationContext(client);

    if (callback)
    {
        auto * callbackClass = static_cast<StaticClientCallbackPollCondition*>(callback);
        result = callbackClass->handler(context, operation, objectID, objectInstanceID, resourceID, resourceInstanceID, dataPointer, dataSize, changed);
    }

    if (global::logLevel == AwaLogLevel_Debug)
        std::cout << "Handler result " << result << std::endl;

    return result;
}

void * do_write_operation(void * attr)
{
    AwaServerWriteOperation * writeOperation = (AwaServerWriteOperation *)attr;
    AwaServerWriteOperation_Perform(writeOperation, global::clientEndpointName, defaults::timeout);
    return 0;
}

void * do_read_operation(void * attr)
{
    AwaServerReadOperation * readOperation = (AwaServerReadOperation *)attr;
    AwaServerReadOperation_Perform(readOperation, defaults::timeout);
    return 0;
}

void * do_execute_operation(void * attr)
{
    AwaServerExecuteOperation * executeOperation = (AwaServerExecuteOperation *)attr;
    AwaServerExecuteOperation_Perform(executeOperation, defaults::timeout);
    return 0;
}


} // namespace Awa
