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
#include <stdint.h>
#include <stdbool.h>

#include "coap_abstraction.h"
#include "lwm2m_core.h"
#include "lwm2m_acl_object.h"
#include "lwm2m_objects.h"


int LWM2M_TABLE30_OBJECTID =            1;
int LWM2M_TABLE30_OBJECTINSTANCE =      0;
ACLRight LWM2M_TABLE30_ACL_101 =        ACLRight_Read | ACLRight_Write | ACLRight_Execute | ACLRight_Delete;
int LWM2M_TABLE30_OWNERSERVERID =       101;

int LWM2M_TABLE31_OBJECTID =            1;
int LWM2M_TABLE31_OBJECTINSTANCE =      1;
ACLRight LWM2M_TABLE31_ACL_102 =        ACLRight_Read | ACLRight_Write | ACLRight_Execute | ACLRight_Delete;
int LWM2M_TABLE31_OWNERSERVERID =       102;

int LWM2M_TABLE32_OBJECTID =            3;
int LWM2M_TABLE32_OBJECTINSTANCE =      0;
ACLRight LWM2M_TABLE32_ACL_101 =        ACLRight_Read | ACLRight_Write | ACLRight_Execute | ACLRight_Delete;
ACLRight LWM2M_TABLE32_ACL_102 =        ACLRight_Read;
int LWM2M_TABLE32_OWNERSERVERID =       101;

int LWM2M_TABLE33_OBJECTID =            4;
int LWM2M_TABLE33_OBJECTINSTANCE =      0;
ACLRight LWM2M_TABLE33_ACL_101 =        ACLRight_Read;
ACLRight LWM2M_TABLE33_ACL_102 =        ACLRight_Read;
int LWM2M_TABLE33_OWNERSERVERID =       101;


void Lwm2m_AddACLObject(Lwm2mContextType * context, int instanceID, int aclObjectID, int aclObjectInstance, ACL * acl, int aclCount, unsigned short ownerServerID)
{
    Lwm2mCore_CreateObjectInstance(context, LWM2M_ACL_OBJECT, instanceID);
    int i = 0;
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_ACL_OBJECT, instanceID, 0, 0, &aclObjectID, sizeof(aclObjectID));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_ACL_OBJECT, instanceID, 1, 0, &aclObjectInstance, sizeof(aclObjectInstance));

    Lwm2mCore_CreateOptionalResource(context, LWM2M_ACL_OBJECT, instanceID, 2);
    for(i = 0; i < aclCount; i++)
    {
        short aclTemp = (short)(acl[i].Rights);
        Lwm2mCore_SetResourceInstanceValue(context, LWM2M_ACL_OBJECT,instanceID, 2, acl[i].ServerID, &aclTemp, sizeof(aclTemp));
    }

    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_ACL_OBJECT, instanceID, 3, 0, &ownerServerID, sizeof(ownerServerID));
}

void Lwm2m_RegisterACLObject(Lwm2mContextType * context)
{
    Lwm2mCore_RegisterObjectType(context, "LWM2MAccessControl" , LWM2M_ACL_OBJECT, MultipleInstancesEnum_Multiple, MandatoryEnum_Optional, &defaultObjectOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "ObjectID", LWM2M_ACL_OBJECT, 0, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly, &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "ObjectInstanceID", LWM2M_ACL_OBJECT, 1, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadOnly, &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "ACL", LWM2M_ACL_OBJECT, 2, AwaResourceType_Integer, MultipleInstancesEnum_Multiple, MandatoryEnum_Optional, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);
    Lwm2mCore_RegisterResourceType(context, "AccessControlOwner", LWM2M_ACL_OBJECT, 3, AwaResourceType_Integer, MultipleInstancesEnum_Single, MandatoryEnum_Mandatory, AwaResourceOperations_ReadWrite, &defaultResourceOperationHandlers);

    /// Populate ACL
    ACL table30[1] = {{101, LWM2M_TABLE30_ACL_101}};
    ACL table31[1] = {{102, LWM2M_TABLE31_ACL_102}};
    ACL table32[2] = {{101, LWM2M_TABLE32_ACL_101}, {102, LWM2M_TABLE32_ACL_102}};
    ACL table33[2] = {{101, LWM2M_TABLE33_ACL_101}, {102, LWM2M_TABLE33_ACL_102}};

    Lwm2m_AddACLObject(context, 0, LWM2M_TABLE30_OBJECTID, LWM2M_TABLE30_OBJECTINSTANCE, table30, 1, LWM2M_TABLE30_OWNERSERVERID);
    Lwm2m_AddACLObject(context, 1, LWM2M_TABLE31_OBJECTID, LWM2M_TABLE31_OBJECTINSTANCE, table31, 1, LWM2M_TABLE31_OWNERSERVERID);
    Lwm2m_AddACLObject(context, 2, LWM2M_TABLE32_OBJECTID, LWM2M_TABLE32_OBJECTINSTANCE, table32, 2, LWM2M_TABLE32_OWNERSERVERID);
    Lwm2m_AddACLObject(context, 3, LWM2M_TABLE33_OBJECTID, LWM2M_TABLE33_OBJECTINSTANCE, table33, 2, LWM2M_TABLE33_OWNERSERVERID);
}

