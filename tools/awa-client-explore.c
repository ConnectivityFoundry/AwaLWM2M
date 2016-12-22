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


// Get one or more resources from an Objects defined
//
//  Usage: awa-client-explore [OPTIONS]... [PATHS]...
//
//    -h, --help                 Print help and exit
//    -V, --version              Print version and exit
//    -v, --verbose              Increase program verbosity  (default=off)
//    -d, --debug                Increase program verbosity  (default=off)
//    -p, --ipcPort=PORT         Connect to Client IPC port  (default=`12345')
//    -a, --ipcAddress              IP address of client (default='127.0.0.1')
//    -q, --quiet                Print values only (quiet)  (default=off)
//
//  Specify one or more object, object instance and resource paths
//  in the format "/O/I/R", separated by spaces. For example:
//
// /3 /4 /4/0/7 /5/0/0 /5\n"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <inttypes.h>

#include "awa/common.h"
#include "awa/client.h"
#include "awa-client-explore_cmdline.h"
#include "tools_common.h"

struct TargetEntry
{
    struct TargetEntry *Next;
    AwaObjectID ObjectID;
    AwaResourceID ResourceID;
};
typedef struct TargetEntry TargetEntry;


static bool IsObjectTargetEntry(TargetEntry *entry)
{
    return IsIDValid(entry->ObjectID)
        && !IsIDValid(entry->ResourceID);
}

static bool IsResourceTargetEntry(TargetEntry *entry)
{
    return IsIDValid(entry->ObjectID)
        && IsIDValid(entry->ResourceID);
}

// TargetEntrys can point to objects or resources.
// Transform each path into a pair of objectID and resourceID
static TargetEntry* CreateTargetList(AwaClientSession *session, char **inputs, unsigned count)
{
    TargetEntry *head = NULL;
    TargetEntry *tail = NULL;
    int i;

    for (i = 0; i < count; ++i)
    {
        Target *target = CreateTarget(inputs[i]);
        if (!target)
        {
            continue;
        }

        TargetEntry *entry = malloc(sizeof(TargetEntry));
        if (!entry)
        {
            Error("Failed to allocate memory for target entry\n");
            FreeTarget(&target);
            continue;
        }
        entry->Next = NULL;

        if (head == NULL)
        {
            head = entry;
            tail = entry;
        }
        else
        {
            tail->Next = entry;
            tail = entry;
        }

        if (AwaClientSession_PathToIDs(session, target->Path, &entry->ObjectID, NULL, &entry->ResourceID) != AwaError_Success)
        {
            FreeTarget(&target);
            continue;
        }
        if (!IsIDValid(entry->ObjectID))
        {
            Error("Path %s is not valid\n", target->Path);
        }

        FreeTarget(&target);
    }

    return head;
}

static void ReleaseTargetEntryList(TargetEntry *entries)
{
    while (entries)
    {
        TargetEntry *tmp = entries->Next;
        free(entries);
        entries = tmp;
    }
}

static void MarkDuplicatesAsInvalid(TargetEntry *entries)
{
    while (entries)
    {
        if (IsIDValid(entries->ObjectID))
        {
            TargetEntry *ptr = entries->Next;
            while (ptr)
            {
                if (entries->ObjectID == ptr->ObjectID
                &&  entries->ResourceID == ptr->ResourceID)
                {
                    ptr->ObjectID = AWA_INVALID_ID;
                }

                ptr = ptr->Next;
            }
        }

        entries = entries->Next;
    }
}

// If a TargetEntry is an object, then all resources of this object will get printed.
// This means that any TargetEntrys that is a resource of this object should not
// be printed. Mark it as invalid so that PrintTargetEntrys ignores it.
static void MarkDuplicateResourcesAsInvalid(TargetEntry *entries)
{
    TargetEntry *currrent = entries;
    while (currrent)
    {
        if (IsObjectTargetEntry(currrent))
        {
            TargetEntry *ptr = entries;
            while (ptr)
            {
                if (IsResourceTargetEntry(ptr)
                &&  ptr->ObjectID == currrent->ObjectID)
                {
                    ptr->ObjectID = AWA_INVALID_ID;
                }
                ptr = ptr->Next;
            }
        }

        currrent = currrent->Next;
    }
}

static void PrintTargets(AwaClientSession *session, TargetEntry *entries, OutputFormat format)
{
    while (entries)
    {
        if (IsIDValid(entries->ObjectID))
        {
            AwaObjectID lastObjectID = AWA_INVALID_ID;
            const AwaObjectDefinition *objectDefinition = AwaClientSession_GetObjectDefinition(session, entries->ObjectID);
            PrintDefinitionTarget(objectDefinition, format, entries->ObjectID, entries->ResourceID, &lastObjectID);

            // All resources from the same object must be printed one after the other to avoid
            // printing header/footer of the same object more than once. This would trigger an
            // AwaError_AlreadyDefined when loading objet definitions in awa_clientd.
            if (IsResourceTargetEntry(entries))
            {
                TargetEntry *ptr = entries->Next;
                while (ptr)
                {
                    if (IsIDValid(ptr->ResourceID)
                    &&  entries->ObjectID == ptr->ObjectID)
                    {
                        PrintDefinitionTarget(objectDefinition, format, ptr->ObjectID, ptr->ResourceID, &lastObjectID);

                        // Overwrite object ID to avoid printing it again
                        ptr->ObjectID = AWA_INVALID_ID;
                    }

                    ptr = ptr->Next;
                }

                PrintObjectDefinitionFooter(objectDefinition, format);
            }
        }
        entries = entries->Next;
    }
}

static void Explore(AwaClientSession *session, char **inputs, unsigned count, OutputFormat format)
{
    if (session)
    {
        TargetEntry *entries = CreateTargetList(session, inputs, count);
        MarkDuplicatesAsInvalid(entries);
        MarkDuplicateResourcesAsInvalid(entries);
        PrintTargets(session, entries, format);
        ReleaseTargetEntryList(entries);
    }
}

int main(int argc, char ** argv)
{
    int result = 0;
    struct gengetopt_args_info ai;
    AwaClientSession * session = NULL;
    OutputFormat outputFormat = OutputFormat_None;

    if (cmdline_parser(argc, argv, &ai) != 0)
    {
        exit(1);
    }

    g_logLevel = ai.debug_given ? 2 : (ai.verbose_given ? 1 : 0);
    AwaLog_SetLevel(ai.debug_given ? AwaLogLevel_Debug : (ai.verbose_given ? AwaLogLevel_Verbose : AwaLogLevel_Warning));

    if(ai.export_given)
    {
        outputFormat = OutputFormat_DeviceServerXML;
    }
    else if(ai.quiet_given)
    {
        outputFormat = OutputFormat_PlainTextQuiet;
    }
    else
    {
        outputFormat = OutputFormat_PlainTextVerbose;
    }

    // Establish Awa Session with the daemon
    session = Client_EstablishSession(ai.ipcAddress_arg, ai.ipcPort_arg);
    if (session != NULL)
    {
        if (ai.inputs_num == 0)
        {
            // Print all defined objects and resources
            AwaObjectDefinitionIterator * iterator = AwaClientSession_NewObjectDefinitionIterator(session);
            if (iterator != NULL)
            {
                PrintAllObjectDefinitions(iterator, ai.quiet_given);
            }
            else
            {
                Error("Could not create definition iterator\n");
                result = 1;
            }
        }
        else
        {
            Explore(session, ai.inputs, ai.inputs_num, outputFormat);
        }
    }
    else
    {
        Error("Failed to establish Awa Session\n");
        result = 1;
    }

    // Clean up
    Client_ReleaseSession(&session);
    cmdline_parser_free(&ai);
    return result;
}