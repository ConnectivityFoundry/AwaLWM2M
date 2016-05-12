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

#include <xmltree.h>

#include "lwm2m_server_xml_registered_entity_tree.h"
#include "../../api/src/path.h"
#include "../../api/src/objects_tree.h"

TreeNode BuildRegisteredEntityTree(const Lwm2mClientType * client)
{
    // returns an <Objects> node containing all registered entities as <Object> and <ObjectInstance> nodes.
    TreeNode objectsTree = ObjectsTree_New();

    // build object/instance list
    struct ListHead * j;
    ListForEach(j, &client->ObjectList)
    {
        ObjectListEntry * entry = ListEntry(j, ObjectListEntry, list);

        char path[MAX_PATH_LENGTH] = { 0 };
        if (Path_MakePath(path, MAX_PATH_LENGTH, entry->ObjectID, entry->InstanceID, AWA_INVALID_ID) == AwaError_Success)
        {
            ObjectsTree_AddPath(objectsTree, path, NULL);
        }
    }
    return objectsTree;
}

