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

/*
 * This example demonstrates how to retrieve the value of a resource instance
 * from a multi-instance resource (array) on the client using a GET operation.
 */

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include <awa/common.h>
#include <awa/client.h>

#define IPC_PORT (12345)
#define IPC_ADDRESS "127.0.0.1"
#define OPERATION_PERFORM_TIMEOUT 1000

int main(void)
{
    /* Create and initialise client session */
    AwaClientSession * session = AwaClientSession_New();

    /* Use default IPC configuration */
    AwaClientSession_Connect(session);

    /* Create GET operation */
    AwaClientGetOperation * operation = AwaClientGetOperation_New(session);
    
    /*
     * This example uses a custom object (ID 1000) and one of its resources (ID 105) which must
     * be created prior to using a set operation upon it. Refer to @example client-defineset-example.c
     * before using the resource we must create it and its object instance.
     */

    /* Add the resource path to retrieve to the GET operation */
    AwaClientGetOperation_AddPath(operation, "/1000/0/105");
    AwaClientGetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    /* A successful operation results in a GET response */
    const AwaClientGetResponse * response = AwaClientGetOperation_GetResponse(operation);

    /* Extract values from the response */
    const AwaIntegerArray * valueArray;
    AwaClientGetResponse_GetValuesAsIntegerArrayPointer(response, "/1000/0/105", &valueArray);

    AwaIntegerArrayIterator * iterator = AwaIntegerArray_NewIntegerArrayIterator(valueArray);
    while (AwaIntegerArrayIterator_Next(iterator))
    {
        printf("%zu : %" PRId64 "\n", AwaIntegerArrayIterator_GetIndex(iterator), AwaIntegerArrayIterator_GetValue(iterator));
    }

    /* Operations must be freed after use */
    AwaClientGetOperation_Free(&operation);

    AwaClientSession_Disconnect(session);
    AwaClientSession_Free(&session);
    return 0;
}
