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
#include <stdbool.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <inttypes.h>

#include "lwm2m_types.h"
#include "lwm2m_limits.h"
#include "lwm2m_observers.h"
#include "lwm2m_attributes.h"
#include "lwm2m_core.h"
#include "lwm2m_list.h"
#include "lwm2m_debug.h"
#include "lwm2m_util.h"
#include "lwm2m_result.h"
#include "lwm2m_security_object.h"
#include "lwm2m_server_object.h"

static Lwm2mObserverType * LookupObserver(void * ctxt, AddressType * addr, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    Lwm2mContextType * context = (Lwm2mContextType *) ctxt;
    struct ListHead * i, *n;
    ListForEachSafe(i, n, Lwm2mCore_GetObserverList(context))
    {
        Lwm2mObserverType * observer = ListEntry(i, Lwm2mObserverType, list);

        if ((observer->ObjectID == objectID) &&
            (observer->ObjectInstanceID == objectInstanceID) &&
            (observer->ResourceID == resourceID) &&
            (memcmp(&observer->Address, addr, sizeof(AddressType)) == 0))
        {
            return observer;
        }
    }
    return NULL;
}

static bool NotificationAttributesValid(AttributeTypeEnum attributeType, NotificationAttributes * attributes)
{
    return (attributes != NULL) && attributes->Valid[attributeType];
}

static NotificationAttributes * GetHighestValidAttributesForType(AttributeTypeEnum attributeType, NotificationAttributes * resourceAttributes,
                                                                 NotificationAttributes * objectInstanceAttributes, NotificationAttributes * objectAttributes)
{
    if (NotificationAttributesValid(attributeType, resourceAttributes))
    {
        return resourceAttributes;
    }
    else if (NotificationAttributesValid(attributeType, objectInstanceAttributes))
    {
        return objectInstanceAttributes;
    }
    else if (NotificationAttributesValid(attributeType, objectAttributes))
    {
        return objectAttributes;
    }
    else
    {
        return NULL;
    }
}

void Lwm2m_MarkObserversChanged(void * ctxt, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID,
                                ResourceIDType resourceID, const void * newValue, size_t newValueLength)
{
    Lwm2mContextType * context = (Lwm2mContextType *) ctxt;
    struct ListHead * observerItem;
    ListForEach(observerItem, Lwm2mCore_GetObserverList(context))
    {
        Lwm2mObserverType * observer = ListEntry(observerItem, Lwm2mObserverType, list);

        if ((observer->ObjectID == objectID) &&
            ((observer->ObjectInstanceID == objectInstanceID) || (observer->ObjectInstanceID == -1)) &&
            ((observer->ResourceID == resourceID) || (observer->ResourceID == -1)))
        {

            int shortServerID = Lwm2mSecurity_GetShortServerID(context, &observer->Address);

            NotificationAttributes * resourceAttributes = (resourceID == -1) ? NULL :
                    AttributeStore_LookupNotificationAttributes(Lwm2mCore_GetAttributes(context), shortServerID, objectID, objectInstanceID, resourceID);
            NotificationAttributes * objectInstanceAttributes = (objectInstanceID == -1) ? NULL :
                    AttributeStore_LookupNotificationAttributes(Lwm2mCore_GetAttributes(context), shortServerID, objectID, objectInstanceID, -1);
            NotificationAttributes * objectAttributes = AttributeStore_LookupNotificationAttributes(Lwm2mCore_GetAttributes(context), shortServerID, objectID, -1, -1);

            ResourceDefinition * definition = Definition_LookupResourceDefinition(Lwm2mCore_GetDefinitions(context), objectID, resourceID);

            bool passedAttributeChecks = false;
            if ((definition != NULL) && (!IS_MULTIPLE_INSTANCE(definition)) && (observer->OldValue != NULL) && (newValue != NULL))
            {
                switch (definition->Type)
                {
                    case AwaResourceType_Integer: // no-break
                    case AwaResourceType_Float:   // no-break
                    case AwaResourceType_Time:
                    {
                        NotificationAttributes * greaterThanAttributes = GetHighestValidAttributesForType(AttributeTypeEnum_GreaterThan, resourceAttributes,
                                                                                                          objectInstanceAttributes, objectAttributes);
                        NotificationAttributes * lessThanAttributes = GetHighestValidAttributesForType(AttributeTypeEnum_LessThan, resourceAttributes,
                                                                                                       objectInstanceAttributes, objectAttributes);
                        NotificationAttributes * stepAttributes = GetHighestValidAttributesForType(AttributeTypeEnum_Step, resourceAttributes,
                                                                                                   objectInstanceAttributes, objectAttributes);

                        switch (definition->Type)
                        {
                            // FIXME: Remove duplication if possible
                            case AwaResourceType_Integer: // no-break
                            case AwaResourceType_Time:
                            {
                                AwaInteger oldValueAsInteger = observer->OldValueLength == sizeof(AwaInteger) ? *((AwaInteger *)observer->OldValue) : 0;
                                AwaInteger newValueAsInteger = newValueLength == sizeof(AwaInteger) ? *((AwaInteger *)newValue) : 0;

                                if ((greaterThanAttributes != NULL) &&
                                        ((oldValueAsInteger > greaterThanAttributes->GreaterThan) == (newValueAsInteger > greaterThanAttributes->GreaterThan)))
                                {
                                    Lwm2m_Error("/%d/%d/%d changed but did not cross over threshold high value; not notifying observer for server %d", objectID, objectInstanceID, resourceID, shortServerID);
                                }
                                else if ((lessThanAttributes != NULL) &&
                                        ((oldValueAsInteger > lessThanAttributes->LessThan) == (newValueAsInteger > lessThanAttributes->LessThan)))
                                {
                                    Lwm2m_Error("/%d/%d/%d changed but did not cross over threshold low value; not notifying observer for server %d", objectID, objectInstanceID, resourceID, shortServerID);
                                }
                                else if ((stepAttributes != NULL) && stepAttributes->Step > labs(oldValueAsInteger - newValueAsInteger))
                                {
                                    Lwm2m_Error("/%d/%d/%d changed but not by the step amount (Old value = %" PRId64 ", new value = %" PRId64 "); not notifying observer for server %d", objectID, objectInstanceID, resourceID, oldValueAsInteger, newValueAsInteger, shortServerID);
                                }
                                else
                                {
                                    passedAttributeChecks = true;
                                }
                                break;
                            }
                            case AwaResourceType_Float:
                            {
                                AwaFloat oldValueAsFloat = observer->OldValueLength == sizeof(AwaInteger) ? *((AwaFloat *)observer->OldValue) : 0;
                                AwaFloat newValueAsFloat = newValueLength == sizeof(AwaInteger) ? *((AwaFloat *)newValue) : 0;

                                if ((greaterThanAttributes != NULL) &&
                                        ((oldValueAsFloat > greaterThanAttributes->GreaterThan) == (newValueAsFloat > greaterThanAttributes->GreaterThan)))
                                {
                                    Lwm2m_Error("/%d/%d/%d changed but did not cross over threshold high value; not notifying observer for server %d", objectID, objectInstanceID, resourceID, shortServerID);
                                }
                                else if ((lessThanAttributes != NULL) &&
                                        ((oldValueAsFloat > lessThanAttributes->LessThan) == (newValueAsFloat > lessThanAttributes->LessThan)))
                                {
                                    Lwm2m_Error("/%d/%d/%d changed but did not cross over threshold low value; not notifying observer for server %d", objectID, objectInstanceID, resourceID, shortServerID);
                                }
                                else if ((stepAttributes != NULL) && stepAttributes->Step > labs(oldValueAsFloat - newValueAsFloat))
                                {
                                    Lwm2m_Error("/%d/%d/%d changed but not by the step amount (Old value = %f, new value = %f); not notifying observer for server %d", objectID, objectInstanceID, resourceID, oldValueAsFloat, newValueAsFloat, shortServerID);
                                }
                                else
                                {
                                    passedAttributeChecks = true;
                                }
                                break;
                            }
                            default:
                                Lwm2m_Error("Unsupported resource type for checking gt/lt/stp attributes: %d\n", definition->Type);
                                break;
                        }
                        break;
                    }
                    default:
                        // Other resource types do not support stp/gt/lt attributes
                        passedAttributeChecks = true;
                        break;
                    }
            }
            else
            {
                if (observer->OldValue != NULL && resourceID != -1)
                {
                    Lwm2m_Error("No resource definition for /%d/%d/%d\n", objectID, objectInstanceID, resourceID);
                }
                else
                {
                    passedAttributeChecks = true;
                }
            }

            if (passedAttributeChecks)
            {
                Lwm2m_Debug("All attributes checked out for server %d, Will notify change to /%d/%d/%d when possible.\n", shortServerID, objectID, objectInstanceID, resourceID);
                observer->Changed = true;

                if (observer->OldValue != NULL)
                {
                    free(observer->OldValue);
                    observer->OldValue = NULL;
                }

                if (newValue != NULL)
                {
                    observer->OldValue = malloc(newValueLength);
                    observer->OldValueLength = newValueLength;
                    memcpy(observer->OldValue, newValue, newValueLength);
                }
            }
        }
    }
}

int Lwm2m_RemoveAllObserversForOIR(void * ctxt, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    Lwm2mContextType * context = (Lwm2mContextType *) ctxt;
    struct ListHead * observerItem, *next;
    ListForEachSafe(observerItem, next, Lwm2mCore_GetObserverList(context))
    {
        Lwm2mObserverType * observer = ListEntry(observerItem, Lwm2mObserverType, list);

        if (observer->ObjectID == objectID &&
            observer->ObjectInstanceID == objectInstanceID &&
            observer->ResourceID == resourceID)
        {

           ListRemove(&observer->list);

           free(observer->OldValue);
           free(observer->ContextData);
           free(observer);
           return 0;
        }
    }
    return -1;
}

void Lwm2m_FreeObservers(void * ctxt)
{
    Lwm2mContextType * context = (Lwm2mContextType *) ctxt;
    struct ListHead * observerItem, *n;
    ListForEachSafe(observerItem, n, Lwm2mCore_GetObserverList(context))
    {
        Lwm2mObserverType * observer = ListEntry(observerItem, Lwm2mObserverType, list);

        ListRemove(observerItem);
        free(observer->OldValue);
        free(observer->ContextData);
        free(observer);
    }
}

int Lwm2m_Observe(void * ctxt, AddressType * addr, const char * token, int tokenLength, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID,
                  ResourceIDType resourceID, AwaContentType contentType, Lwm2mNotificationCallback callback, void * ContextData)
{
    Lwm2mContextType * context = (Lwm2mContextType *) ctxt;
    int result = 0;

    Lwm2mObserverType * observer;

    // If the client is already on the list of observers, the server must
    // not add it a second time but replace or update the existing entry.
    observer = LookupObserver(context, addr, objectID, objectInstanceID, resourceID);
    if (observer == NULL)
    {
        observer = (Lwm2mObserverType *) malloc(sizeof(Lwm2mObserverType));
        if (observer == NULL)
        {
            Lwm2m_Error("Error allocating memory\n");
            result = -1;
            goto error;
        }

        memset(observer, 0, sizeof(*observer));
        ListAdd(&observer->list, Lwm2mCore_GetObserverList(context));
    }
    else
    {
        free(observer->OldValue);
        free(observer->ContextData);
    }

    observer->OldValue = NULL;
    observer->OldValueLength = 0;
    observer->ObjectID = objectID;
    observer->ObjectInstanceID = objectInstanceID;
    observer->ResourceID = resourceID;
    observer->ContentType = contentType;
    observer->LastUpdate = 0;
    observer->Callback = callback;
    observer->ContextData = ContextData;
    observer->Changed = false;
    observer->TokenLength = tokenLength;
    observer->Sequence = 1;
    memcpy(&observer->Token, token, tokenLength);
    memcpy(&observer->Address, addr, sizeof(AddressType));

    // The old value buffer must be created when the observation begins,
    // otherwise attributes can't be checked on the first modification of a resource value.
    if (resourceID != -1)
    {
        ResourceDefinition * resourceDefinition = Definition_LookupResourceDefinition(Lwm2mCore_GetDefinitions(context), objectID, resourceID);

        if ((resourceDefinition != NULL) && (!IS_MULTIPLE_INSTANCE(resourceDefinition)))
        {
            const void * oldValue = NULL;
            size_t oldValueLength = 0;

            Lwm2mCore_GetResourceInstanceValue(context, objectID, objectInstanceID, resourceID, 0, &oldValue, &oldValueLength);

            if ((oldValueLength > 0) && (oldValue != NULL))
            {
                observer->OldValueLength = oldValueLength;
                observer->OldValue = malloc(observer->OldValueLength);
                memcpy(observer->OldValue, oldValue, observer->OldValueLength);
            }
        }
    }

error:
    return result;
}

int Lwm2m_CancelObserve(void * ctxt, AddressType * addr, ObjectIDType objectID, ObjectInstanceIDType objectInstanceID, ResourceIDType resourceID)
{
    Lwm2mContextType * context = (Lwm2mContextType *) ctxt;
    Lwm2mObserverType * observer = LookupObserver(context, addr, objectID, objectInstanceID, resourceID);
    if (observer != NULL)
    {
        ListRemove(&observer->list);
        free(observer->OldValue);
        free(observer->ContextData);
        free(observer);
        return 0;
    }
    return -1;
}

void Lwm2m_UpdateObservers(void * ctxt)
{
    Lwm2mContextType * context = (Lwm2mContextType *) ctxt;
    uint32_t now = Lwm2mCore_GetTickCountMs();

    struct ListHead * observerItem, *n;
    ListForEachSafe(observerItem, n, Lwm2mCore_GetObserverList(context))
    {
        Lwm2mObserverType * observer = ListEntry(observerItem, Lwm2mObserverType, list);

        int shortServerID = Lwm2mSecurity_GetShortServerID(context, &observer->Address);

        NotificationAttributes * resourceAttributes = observer->ResourceID == -1? NULL : AttributeStore_LookupNotificationAttributes(Lwm2mCore_GetAttributes(context), shortServerID, observer->ObjectID, observer->ObjectInstanceID, observer->ResourceID);
        NotificationAttributes * objectInstanceAttributes = observer->ObjectInstanceID == -1? NULL : AttributeStore_LookupNotificationAttributes(Lwm2mCore_GetAttributes(context), shortServerID, observer->ObjectID, observer->ObjectInstanceID, -1);
        NotificationAttributes * objectAttributes = AttributeStore_LookupNotificationAttributes(Lwm2mCore_GetAttributes(context), shortServerID, observer->ObjectID, -1, -1);

        NotificationAttributes * minimumPeriodAttributes = GetHighestValidAttributesForType(AttributeTypeEnum_MinimumPeriod, resourceAttributes, objectInstanceAttributes, objectAttributes);
        int minimumPeriod = minimumPeriodAttributes != NULL? minimumPeriodAttributes->MinimumPeriod : Lwm2mServerObject_GetDefaultMinimumPeriod(context, shortServerID);

        NotificationAttributes * maximumPeriodAttributes = GetHighestValidAttributesForType(AttributeTypeEnum_MaximumPeriod, resourceAttributes, objectInstanceAttributes, objectAttributes);
        int maximumPeriod = maximumPeriodAttributes != NULL? maximumPeriodAttributes->MaximumPeriod : Lwm2mServerObject_GetDefaultMaximumPeriod(context, shortServerID);

        uint32_t elapsed = now - observer->LastUpdate;

        if ((observer->Changed && (elapsed > ((uint32_t)minimumPeriod * 1000))) ||
            ((maximumPeriod != -1) && (elapsed > ((uint32_t)maximumPeriod * 1000))))
        {
            observer->Sequence ++;
            observer->Callback(context, &observer->Address, observer->Sequence, (const char*)&observer->Token, observer->TokenLength, 
                               observer->ObjectID, observer->ObjectInstanceID, observer->ResourceID, observer->ContentType, observer->ContextData);
            observer->Changed = false;
            observer->LastUpdate = now;
        }
    }
}
