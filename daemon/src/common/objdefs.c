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

#include "objdefs.h"
#include "lwm2m_debug.h"
#include "xmltree.h"
#include "lwm2m_xml_interface.h"

// defined differently by client and server:
extern DefinitionCount xmlif_ParseObjDefDeviceServerXml(Lwm2mContextType * context, TreeNode content);

static int LoadObjectDefinitionsFromFile(Lwm2mContextType * context, const char * filename);

int LoadObjectDefinitionsFromFiles(Lwm2mContextType * context, const char ** filenames, size_t numFilenames)
{
    int result = 0;
    int i;
    for (i = 0; i < numFilenames; ++i)
    {
        if (LoadObjectDefinitionsFromFile(context, filenames[i]) != 0)
        {
            Lwm2m_Error("Failed to load object definitions from file \'%s\'\n", filenames[i]);
            result = 1;
            break;
        }
    }
    return result;
}

static int LoadObjectDefinitionsFromFile(Lwm2mContextType * context, const char * filename)
{
    DefinitionCount count = { 0 };
    int result = 0;

    Lwm2m_Info("Load definitions: from \'%s\'\n", filename);

    FILE *f = fopen(filename, "rb");
    if (f != NULL)
    {
        if (fseek(f, 0, SEEK_END) == 0)
        {
            long pos = ftell(f);
            if (pos >= 0)
            {
                if (fseek(f, 0, SEEK_SET) == 0)
                {
                    uint8_t * doc = malloc((size_t)pos);
                    if (doc != NULL)
                    {
                        size_t nmemb = fread(doc, (size_t)pos, 1, f);
                        if (nmemb == 1)
                        {
                            Lwm2m_Debug("Parsing %s, %ld bytes\n", filename, pos);
                            TreeNode objectDefinitionsNode = TreeNode_ParseXML(doc, (uint32_t)pos, true);
                            count = xmlif_ParseObjDefDeviceServerXml(context, objectDefinitionsNode);
                            result = 0;
                            Tree_Delete(objectDefinitionsNode);
                            free(doc);
                        }
                        else
                        {
                            perror("fread");
                            result = -1;
                            free(doc);
                        }
                    }
                    else
                    {
                        Lwm2m_Error("Out of memory\n");
                        result = -1;
                    }
                }
                else
                {
                    perror("fseek");
                    result = -1;
                }
            }
            else
            {
                perror("ftell");
                result = -1;
            }
        }
        else
        {
            perror("fseek");
            result = -1;
        }
        fclose(f);
    }
    else
    {
        perror("fopen");
        result = -1;
    }

    if (result == 0)
    {
        // regard any failures as fatal

        if (count.NumObjectsFailed > 0) {
            Lwm2m_Error("%zu object definition%s failed\n", count.NumObjectsFailed, count.NumObjectsFailed != 1 ? "s" : "" );
            result = -1;
        }
        if (count.NumResourcesFailed > 0) {
            Lwm2m_Error("%zu resource definition%s failed\n", count.NumResourcesFailed, count.NumResourcesFailed != 1 ? "s" : "");
            result = -1;
        }
        Lwm2m_Info("Load definitions: %zu object%s and %zu resource%s defined\n", count.NumObjectsOK, count.NumObjectsOK != 1 ? "s" : "", count.NumResourcesOK, count.NumResourcesOK != 1 ? "s" : "");

        // also regard nothing defined as failure
        if (count.NumObjectsOK == 0 && count.NumResourcesOK == 0)
        {
            Lwm2m_Error("No objects or resources defined\n");
            result = -1;
        }
    }

    if (result < 0)
    {
        Lwm2m_Error("Load definitions: failed\n");
    }
    return result;
}

