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


#ifndef LWM2M_ATTRIBUTES_H
#define LWM2M_ATTRIBUTES_H

#include <stdint.h>
#include <stdbool.h>
#include "lwm2m_list.h"
#include "lwm2m_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    AttributeTypeEnum_MinimumPeriod,
    AttributeTypeEnum_MaximumPeriod,
    AttributeTypeEnum_GreaterThan,
    AttributeTypeEnum_LessThan,
    AttributeTypeEnum_Step,
    AttributeTypeEnum_Cancel, // Lwm2m spec 1.0 20141126-C - "Cancel" attribute to cancel observation.

    AttributeTypeEnum_LAST,  // SENTINEL
} AttributeTypeEnum;

typedef struct
{
    struct ListHead list; // prev/next pointers

    int MinimumPeriod;  // CoRE param "pmin", default: 1 second, restarted for each notification
    int MaximumPeriod;  // CoRE param "pmax"
    float GreaterThan;  // CoRE param "gt"
    float LessThan;     // CoRE param "lt"
    float Step;         // CoRE param "stp"

    bool Valid[AttributeTypeEnum_LAST];

    int ShortServerID;
    ObjectIDType ObjectID;
    ObjectInstanceIDType ObjectInstanceID;
    ResourceIDType ResourceID;

} NotificationAttributes;

typedef struct
{
    uint8_t Dimension;    // Number of instantiations for a multiple-instance resource
} PropertyAttributes;

typedef struct
{
    AttributeTypeEnum Type;
    const char * Name;
    const char * CoRELinkParam;
    AwaResourceType ValueType;

} AttributeCharacteristics;

typedef struct
{
    struct ListHead ServerNotificationAttributes;
} AttributeStore;

const AttributeCharacteristics * Lwm2mAttributes_GetAttributeCharacteristics(char * coreLinkParam);

AttributeStore * AttributeStore_Create(void);
void AttributeStore_Destroy(AttributeStore * store);
NotificationAttributes * AttributeStore_LookupNotificationAttributes(AttributeStore * store, int shortServerID, ObjectIDType objectID,
                                                                     ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID);

#ifdef __cplusplus
}
#endif

#endif // LWM2M_ATTRIBUTES_H
