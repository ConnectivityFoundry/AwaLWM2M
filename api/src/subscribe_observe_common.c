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


#include "error.h"
#include "path.h"
#include "xml.h"
#include "log.h"
#include "memalloc.h"
#include "subscribe_observe_common.h"
#include "lwm2m_result.h"
#include "lwm2m_xml_serdes.h"
#include "lwm2m_xml_interface.h"
#include "utils.h"
#include "changeset.h"
#include "objects_tree.h"

AwaError SubscribeObserveCommon_CallObservers(MapType * observers, AwaChangeSet * changeSet, CallObserversCallback callback)
{
    AwaError result = AwaError_Unspecified;
    char * path = NULL;

    uint32_t objectIndex = 0;
    TreeNode objectNode = NULL;

    TreeNode objectsTree = ChangeSet_GetObjectsTree(changeSet);
    while ((objectNode = Xml_FindFrom(objectsTree, "Object", &objectIndex)) != NULL)
    {
        int objectID = xmlif_GetInteger(objectNode, "Object/ID");
        if (objectID != AWA_INVALID_ID)
        {
            uint32_t objectInstanceIndex = 0;
            TreeNode objectInstanceNode = NULL;
            while ((objectInstanceNode = Xml_FindFrom(objectNode, "ObjectInstance", &objectInstanceIndex)) != NULL)
            {
                int objectInstanceID = xmlif_GetInteger(objectInstanceNode, "ObjectInstance/ID");
                if (objectInstanceID != AWA_INVALID_ID)
                {
                    uint32_t resourceIndex = 0;
                    TreeNode resourceNode = NULL;
                    while ((resourceNode = Xml_FindFrom(objectInstanceNode, "Resource", &resourceIndex)) != NULL)
                    {
                        int resourceID = xmlif_GetInteger(resourceNode, "Resource/ID");
                        if (resourceID != AWA_INVALID_ID)
                        {
                            msprintf(&path, "/%d/%d/%d", objectID, objectInstanceID, resourceID);
                            callback(observers, path, changeSet);
                            Awa_MemSafeFree(path);
                        }
                        else
                        {
                            result = LogErrorWithEnum(AwaError_IPCError,"Object ID is not specified");
                            goto error;
                        }
                    }
                    msprintf(&path, "/%d/%d", objectID, objectInstanceID);
                    callback(observers, path, changeSet);
                    Awa_MemSafeFree(path);
                }
                else
                {
                    result = LogErrorWithEnum(AwaError_IPCError,"Object ID is not specified");
                    goto error;
                }
            }
            msprintf(&path, "/%d", objectID);
            callback(observers, path, changeSet);
            Awa_MemSafeFree(path);
        }
        else
        {
            result = LogErrorWithEnum(AwaError_IPCError,"Object ID is not specified");
            goto error;
        }
    }
    error:
        return result;
}

