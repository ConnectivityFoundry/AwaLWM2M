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


#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <inttypes.h>

#include "lwm2m_util.h"
#include "lwm2m_list.h"
#include "lwm2m_debug.h"

ObjectInstanceResourceKey UriToOir(const char * uri)
{
    ObjectInstanceResourceKey key = {-1,-1,-1};

    if (sscanf(uri, "%5d/%5d/%5d", &key.ObjectID, &key.InstanceID, &key.ResourceID) == 0)
    {
        sscanf(uri, "/%5d/%5d/%5d", &key.ObjectID, &key.InstanceID, &key.ResourceID);
    }

    return key;
}

const char * OirToUri(ObjectInstanceResourceKey key)
{
    static char buffer[64];

    memset(buffer, 0,  sizeof(buffer));

    if ((key.ResourceID != -1) && (key.InstanceID != -1))
    {
        sprintf(buffer, "%d/%d/%d", key.ObjectID, key.InstanceID, key.ResourceID);
    }
    else if (key.InstanceID != -1)
    {
        sprintf(buffer, "%d/%d", key.ObjectID, key.InstanceID);
    }
    else
    {
        sprintf(buffer, "%d", key.ObjectID);
    }

    return &buffer[0];
}

int8_t ptrToInt8(void * ptr)
{
    int8_t temp = 0;
    memcpy(&temp,ptr,sizeof(int8_t));
    return temp;
}

int16_t ptrToInt16(void * ptr)
{
    int16_t temp = 0;
    memcpy(&temp,ptr,sizeof(int16_t));
    return temp;
}

int32_t ptrToInt32(void * ptr)
{
    int32_t temp = 0;
    memcpy(&temp,ptr,sizeof(int32_t));
    return temp;
}

int64_t ptrToInt64(void * ptr)
{
    int64_t temp = 0;
    memcpy(&temp,ptr,sizeof(int64_t));
    return temp;
}

static int Lwm2mCore_CountQueryPairs(const char * query)
{
    int count = 0;
    if (query != NULL)
    {
        char * str = strdup(query);

        const char delim[] = "&?";

        char * token = strtok(str, delim);

        while (token != NULL)
        {
            count++;
            token = strtok(NULL, delim);
        }
        free(str);
    }
    return count;
}

QueryPair * Lwm2mCore_SplitQuery(const char * query, int * numPairs)
{
    *numPairs = Lwm2mCore_CountQueryPairs(query);
    QueryPair * pairs = NULL;
    if (*numPairs > 0)
    {
        pairs = malloc(*numPairs * sizeof(QueryPair));
        if (pairs != NULL)
        {
            int count = 0;
            char * str = strdup(query);

            const char delim[] = "&?";

            char * token = strtok(str, delim);

            while (token != NULL)
            {
                QueryPair * pair = &pairs[count];

                char * equals = strchr(token, '=');
                if (equals == NULL)
                {
                    pair->Key = strdup(token);
                    pair->Value = NULL;
                }
                else
                {
                    int equalsPos = (int)(equals - token);

                    pair->Key = (char*) malloc((equalsPos+1) * sizeof(char));
                    memcpy(pair->Key, token, equalsPos);
                    pair->Key[equalsPos] = '\0';
                    pair->Value = strdup(equals + 1);
                    if (strlen(pair->Value) == 0)
                        pair->Value = NULL;
                }
                token = strtok(NULL, delim);
                count++;
            }
            free(str);
        }
        else
        {
            Lwm2m_Error("Failed to allocate memory for Lwm2mCore_SplitUpQuery");
        }
    }
    return pairs;
}

void Lwm2mCore_FreeQueryPairs(QueryPair * pairs, int numPairs)
{
    int i;
    if (pairs == NULL)
    {
        return;
    }
    for (i = 0; i < numPairs; i++)
    {
        QueryPair * pair = &pairs[i];
        free(pair->Key);
        free(pair->Value);
    }
    free(pairs);
}

