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
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "lwm2m_attributes.h"
#include "lwm2m_types.h"
#include "lwm2m_debug.h"
#include "lwm2m_result.h"

const static AttributeCharacteristics AttributeCharacteristicsTable[] =
{
    {.Type = AttributeTypeEnum_MinimumPeriod,   .Name = "Minimum Period",  .CoRELinkParam = "pmin",    .ValueType = AwaResourceType_Integer},
    {.Type = AttributeTypeEnum_MaximumPeriod,   .Name = "Maximum Period",  .CoRELinkParam = "pmax",    .ValueType = AwaResourceType_Integer},
    {.Type = AttributeTypeEnum_GreaterThan,     .Name = "Greater Than",    .CoRELinkParam = "gt",      .ValueType = AwaResourceType_Float},
    {.Type = AttributeTypeEnum_LessThan,        .Name = "Less Than",       .CoRELinkParam = "lt",      .ValueType = AwaResourceType_Float},
    {.Type = AttributeTypeEnum_Step,            .Name = "Step",            .CoRELinkParam = "stp",     .ValueType = AwaResourceType_Float},
    {.Type = AttributeTypeEnum_Cancel,          .Name = "Cancel",          .CoRELinkParam = "cancel",  .ValueType = AwaResourceType_None},
};

const AttributeCharacteristics * Lwm2mAttributes_GetAttributeCharacteristics(char * coreLinkParam)
{
    const AttributeCharacteristics * characteristics = NULL;
    int i = 0;
    for (; i < sizeof(AttributeCharacteristicsTable) / sizeof(AttributeCharacteristicsTable[0]); i++)
    {
        if (strcmp(coreLinkParam, AttributeCharacteristicsTable[i].CoRELinkParam) == 0)
        {
            characteristics = &AttributeCharacteristicsTable[i];
            break;
        }
    }
    return characteristics;
}

static NotificationAttributes * LookupNotificationAttributes(struct ListHead * attributesList, int shortServerID, ObjectIDType objectID,
                                                             ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    NotificationAttributes * result = NULL;
    if (attributesList != NULL)
    {
        struct ListHead * i;
        ListForEach(i, attributesList)
        {
            NotificationAttributes * attributes = ListEntry(i, NotificationAttributes, list);
            if ((attributes != NULL) &&
                (attributes->ShortServerID == shortServerID) &&
                (attributes->ObjectID == objectID) &&
                (attributes->ObjectInstanceID == objectInstanceID) &&
                (attributes->ResourceID == resourceID))
            {
                result = attributes;
                break;
            }
        }
        if (result == NULL)
        {
            // doesn't exist yet, so create
            result = malloc(sizeof(NotificationAttributes));
            memset(result, 0, sizeof(NotificationAttributes));
            memset(result->Valid, 0, sizeof(result->Valid));
            result->ObjectID = objectID;
            result->ObjectInstanceID = objectInstanceID;
            result->ResourceID = resourceID;
            result->ShortServerID = shortServerID;
            ListAdd(&result->list, attributesList);
        }
    }
    return result;
}

static void DestroyAttributesList(struct ListHead * serverNotificationAttributesList)
{
    if (serverNotificationAttributesList != NULL)
    {
        struct ListHead * i, * n;
        ListForEachSafe(i, n, serverNotificationAttributesList)
        {
            NotificationAttributes * attributes = ListEntry(i, NotificationAttributes, list);
            if (attributes != NULL)
            {
                free(attributes);
            }
        }
    }
}

AttributeStore * AttributeStore_Create(void)
{
    AttributeStore * store = (AttributeStore *)malloc(sizeof(AttributeStore));
    if (store == NULL)
    {
        AwaResult_SetResult(AwaResult_OutOfMemory);
        return NULL;
    }

    memset(store, 0, sizeof(AttributeStore));

    ListInit(&store->ServerNotificationAttributes);

    AwaResult_SetResult(AwaResult_Success);
    return store;
}

void AttributeStore_Destroy(AttributeStore * store)
{
    if (store != NULL)
    {
        // loop through all objects and free them
        DestroyAttributesList(&store->ServerNotificationAttributes);
        free(store);
    }
}

NotificationAttributes * AttributeStore_LookupNotificationAttributes(AttributeStore * store, int shortServerID, ObjectIDType objectID,
                                                                     ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    NotificationAttributes * attributes = NULL;
    if (store != NULL)
    {
        attributes = LookupNotificationAttributes(&store->ServerNotificationAttributes, shortServerID, objectID, objectInstanceID, resourceID);
    }
    return attributes;
}
