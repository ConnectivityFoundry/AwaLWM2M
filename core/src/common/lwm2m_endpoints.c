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
#include <string.h>
#include <stdint.h>

#include "lwm2m_list.h"
#include "lwm2m_debug.h"
#include "lwm2m_endpoints.h"
#include "coap_abstraction.h"

int Lwm2mEndPoint_InitEndPointList(ResourceEndPointList * endPointList)
{
    ListInit(&endPointList->EndPoint);
    return 0;
}

int Lwm2mEndPoint_DestroyEndPointList(ResourceEndPointList * endPointList)
{
    struct ListHead * i, *n;
    ListForEachSafe(i, n, &endPointList->EndPoint)
    {
        ResourceEndPoint * endPoint = ListEntry(i, ResourceEndPoint, list);

        ListRemove(i);

        coap_DeregisterUri(endPoint->Path);

        free(endPoint->Root);
        free(endPoint->Path);
        free(endPoint);
    }
    return 0;
}

/* Magic POST & PUT handling, look up "ancestors" in the path. i.e
 * if /3/0/1 is supplied, first checks /3/0/1 exists if not checks for /3/0
 * if that doesn't exist checks for /3
 */
ResourceEndPoint * Lwm2mEndPoint_FindResourceEndPointAncestors(ResourceEndPointList * endPointList, const char * path)
{
    char * element = strdup(path);

    struct ListHead * i;
    while (strlen(element) > 0)
    {
        ListForEach(i, &endPointList->EndPoint)
        {
            ResourceEndPoint * endPoint = ListEntry(i, ResourceEndPoint, list);
            if (!strcmp(endPoint->Path, element))
            {
                free(element);
                return endPoint;
            }
        }
        // strip path back to previous "/"
        char * pos = strrchr(element, '/');
        if (pos != NULL)
        {
            *pos = 0; // null terminate at last "/"
        }
        else
        {
            break;
        }
    }
    free(element);
    return NULL;
}

ResourceEndPoint * Lwm2mEndPoint_FindResourceEndPoint(ResourceEndPointList * endPointList, const char * path)
{
    struct ListHead * i;
    ListForEach(i, &endPointList->EndPoint)
    {
        ResourceEndPoint * endPoint = ListEntry(i, ResourceEndPoint, list);
        if (!strcmp(endPoint->Path, path))
        {
            return endPoint;
        }
    }
    return NULL;
}

int Lwm2mEndPoint_AddResourceEndPoint(ResourceEndPointList * endPointList, const char * path, EndpointHandlerFunction handler)
{
    int result = -1;
    ResourceEndPoint * endPoint = Lwm2mEndPoint_FindResourceEndPoint(endPointList, path);

    if (endPoint == NULL)
    {
        endPoint = malloc(sizeof(ResourceEndPoint));
        if (endPoint != NULL)
        {
            endPoint->Root = strdup("/");
            endPoint->Path = strdup(path);
            endPoint->Handler = handler;

            ListAdd(&endPoint->list, &endPointList->EndPoint);

            coap_RegisterUri(path);
            result = 0;
        }
        else
        {
            Lwm2m_Error("Unable to allocate memory for Resource end point %s\n", path);
            result = -1;
        }
    }
    else
    {
        // Already exists
        result = 0;
    }

    return result;
}

int Lwm2mEndPoint_RemoveResourceEndPoint(ResourceEndPointList * endPointList, const char * path)
{
    int result = -1;
    ResourceEndPoint * endPoint = Lwm2mEndPoint_FindResourceEndPoint(endPointList, path);

    if (endPoint != NULL)
    {
        ListRemove(&endPoint->list);

        coap_DeregisterUri(path);

        free(endPoint->Root);
        free(endPoint->Path);
        free(endPoint);

        result = 0;
    }

    return result;
}
