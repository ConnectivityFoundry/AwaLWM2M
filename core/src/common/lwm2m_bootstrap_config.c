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
#include <sys/types.h>

#include "lwm2m_bootstrap_config.h"
#include "lwm2m_core.h"
#include "lwm2m_debug.h"
#include "lwm2m_objects.h"

static bool ParseLine(BootstrapInfo * bootstrapInfo, char * line, size_t len)
{
    if (len == 0)
    {
        return false;
    }

    if (sscanf(line, "ServerURI=%254s", bootstrapInfo->SecurityInfo.ServerURI))
    {
        return true;
    }

    if (sscanf(line, "SecurityMode=%10d", &bootstrapInfo->SecurityInfo.SecurityMode))
    {
        return true;
    }

    if (sscanf(line, "PublicKey=%254s", bootstrapInfo->SecurityInfo.PublicKey))
    {
        return true;
    }

    if (sscanf(line, "SecretKey=%254s", bootstrapInfo->SecurityInfo.SecretKey))
    {
        return true;
    }

    if (sscanf(line, "ServerID=%10d", &bootstrapInfo->SecurityInfo.ServerID))
    {
        return true;
    }

    if (sscanf(line, "HoldOffTime=%10d", &bootstrapInfo->SecurityInfo.HoldOffTime))
    {
        return true;
    }

    if (sscanf(line, "ShortServerID=%10d", &bootstrapInfo->ServerInfo.ShortServerID))
    {
        return true;
    }

    if (sscanf(line, "Binding=%5s", bootstrapInfo->ServerInfo.Binding))
    {
        return true;
    }

    if (sscanf(line, "LifeTime=%10d", &bootstrapInfo->ServerInfo.LifeTime))
    {
        return true;
    }

    if (sscanf(line, "DefaultMinimumPeriod=%10d", &bootstrapInfo->ServerInfo.MinPeriod))
    {
        return true;
    }

    if (sscanf(line, "DefaultMaximumPeriod=%10d", &bootstrapInfo->ServerInfo.MaxPeriod))
    {
        return true;
    }

    if (sscanf(line, "DisableTimeout=%10d", &bootstrapInfo->ServerInfo.DisableTimeout))
    {
        return true;
    }

    int notificationAsInteger = 0;
    char notificationAsBooleanString[255];
    if (sscanf(line, "NotificationStoringWhenDisabledOrOffline=%10d", &notificationAsInteger))
    {
        bootstrapInfo->ServerInfo.Notification = notificationAsInteger != 0;
        return true;
    }
    else if (sscanf(line, "NotificationStoringWhenDisabledOrOffline=%254s", notificationAsBooleanString))
    {
        bootstrapInfo->ServerInfo.Notification = strcasecmp(notificationAsBooleanString, "true") == 0;
        return true;
    }

    Lwm2m_Error("Unable to parse entry in configuration file %s\n", line);
    return false;
}

const BootstrapInfo * BootstrapInformation_ReadConfigFile(const char * configFile)
{
    BootstrapInfo * bootstrapInfo = NULL;

    // Very basic config file parsing, with no error checking, simply
    // to allow an easy way to change settings without re-compiling
    if (configFile != NULL)
    {
        bootstrapInfo = (struct _BootstrapInfo *)malloc(sizeof(struct _BootstrapInfo));
        if (bootstrapInfo == NULL)
        {
            Lwm2m_Error("Out of memory\n");
            return NULL;
        }
        memset(bootstrapInfo, 0, sizeof(*bootstrapInfo));

        // Set sensible server info defaults
        bootstrapInfo->ServerInfo.ShortServerID = 0;
        bootstrapInfo->ServerInfo.LifeTime = 0;
        bootstrapInfo->ServerInfo.MinPeriod = 1;
        bootstrapInfo->ServerInfo.MaxPeriod = -1;
        bootstrapInfo->ServerInfo.DisableTimeout = 86400;
        bootstrapInfo->ServerInfo.Notification = true;
        strcpy(bootstrapInfo->ServerInfo.Binding, "U");

        FILE * f = fopen(configFile, "rt");
        if (f == NULL)
        {
            Lwm2m_Error("Configuration file not found: %s\n", configFile);
            free(bootstrapInfo);
            bootstrapInfo = NULL;
        }
        else
        {
            char * line = NULL;
            size_t len = 0;
            ssize_t count = 0;
            while (((count = getline(&line, &len, f)) != -1))
            {
                if (count > 0)
                {
                    int i;
                    for (i = 0; i < count; i++)
                    {
                        if ((line[i] == '\n') || (line[i] == '\r'))
                        {
                            line[i] = '\0';  // remove newline, or crnl from end of line
                        }
                    }
                    if (!ParseLine(bootstrapInfo, line, strlen(line)))
                    {
                        free(bootstrapInfo);
                        bootstrapInfo = NULL;
                        break;
                    }
                }
                free(line);
                line = NULL;  // must be NULL for getline() to allocate memory for the next line
            }
            free(line);  // getline may allocate memory for characters before EOF
            fclose(f);
        }
    }

    return bootstrapInfo;
}

void BootstrapInformation_Dump(const BootstrapInfo * bootstrapInfo)
{
    Lwm2m_Info("ServerURI            : %s\n", bootstrapInfo->SecurityInfo.ServerURI);
    Lwm2m_Info("SecurityMode         : %d\n", bootstrapInfo->SecurityInfo.SecurityMode);
    Lwm2m_Info("SecretKey            : %s\n", bootstrapInfo->SecurityInfo.SecretKey);
    Lwm2m_Info("PublicKey            : %s\n", bootstrapInfo->SecurityInfo.PublicKey);
    Lwm2m_Info("ServerID             : %d\n", bootstrapInfo->SecurityInfo.ServerID);
    Lwm2m_Info("HoldOffTime          : %d\n", bootstrapInfo->SecurityInfo.HoldOffTime);
    Lwm2m_Info("ShortServerID        : %d\n", bootstrapInfo->ServerInfo.ShortServerID);
    Lwm2m_Info("Binding              : %s\n", bootstrapInfo->ServerInfo.Binding);
    Lwm2m_Info("LifeTime             : %d\n", bootstrapInfo->ServerInfo.LifeTime);
    Lwm2m_Info("DisableTimeout       : %d\n", bootstrapInfo->ServerInfo.DisableTimeout);
    Lwm2m_Info("DefaultMinimumPeriod : %d\n", bootstrapInfo->ServerInfo.MinPeriod);
    Lwm2m_Info("DefaultMaximumPeriod : %d\n", bootstrapInfo->ServerInfo.MaxPeriod);
    Lwm2m_Info("NotificationStoringWhenDisabledOrOffline : %d\n", bootstrapInfo->ServerInfo.Notification);
}

void BootstrapInformation_DeleteBootstrapInfo(const BootstrapInfo * bootstrapInfo)
{
    free((void *)bootstrapInfo);
}

AwaObjectInstanceID FindNextFreeObjectInstanceID(Lwm2mContextType * context, AwaObjectID ObjectID, AwaObjectInstanceID StartObjectInstanceID)
{
    AwaObjectInstanceID result = AWA_INVALID_ID;

    if (StartObjectInstanceID != AWA_INVALID_ID)
    {
        result = StartObjectInstanceID;
    }

    while(Lwm2mCore_Exists(context, ObjectID, result, AWA_INVALID_ID))
    {
        result++;
    }

    if (result > AWA_MAX_ID)
    {
        result = AWA_INVALID_ID;
    }

    return result;
}

int BootstrapInformation_Apply(Lwm2mContextType * context, const BootstrapInfo * bootstrapInfo)
{
    AwaObjectInstanceID instanceID = FindNextFreeObjectInstanceID(context, LWM2M_SECURITY_OBJECT, 1);
    Lwm2mCore_CreateObjectInstance(context, LWM2M_SECURITY_OBJECT, instanceID);

#ifdef LWM2M_BOOTSTRAP
    Lwm2mCore_CreateOptionalResource(context, LWM2M_SECURITY_OBJECT, instanceID, LWM2M_SECURITY_OBJECT_SERVER_URI);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_SECURITY_OBJECT, instanceID, LWM2M_SECURITY_OBJECT_BOOTSTRAP_SERVER);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_SECURITY_OBJECT, instanceID, LWM2M_SECURITY_OBJECT_SECURITY_MODE);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_SECURITY_OBJECT, instanceID, LWM2M_SECURITY_OBJECT_PUBLIC_KEY);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_SECURITY_OBJECT, instanceID, LWM2M_SECURITY_OBJECT_SECRET_KEY);
#endif

    Lwm2mCore_CreateOptionalResource(context, LWM2M_SECURITY_OBJECT, instanceID, LWM2M_SECURITY_OBJECT_SHORT_SERVER_ID);
    Lwm2mCore_CreateOptionalResource(context, LWM2M_SECURITY_OBJECT, instanceID, LWM2M_SECURITY_OBJECT_CLIENT_HOLD_OFF);


    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SECURITY_OBJECT, instanceID, LWM2M_SECURITY_OBJECT_SERVER_URI, 0,
                                       bootstrapInfo->SecurityInfo.ServerURI,     strlen(bootstrapInfo->SecurityInfo.ServerURI));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SECURITY_OBJECT, instanceID, LWM2M_SECURITY_OBJECT_BOOTSTRAP_SERVER, 0,
                                       &bootstrapInfo->SecurityInfo.Bootstrap,    sizeof(bootstrapInfo->SecurityInfo.Bootstrap));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SECURITY_OBJECT, instanceID, LWM2M_SECURITY_OBJECT_SECURITY_MODE, 0,
                                       &bootstrapInfo->SecurityInfo.SecurityMode, sizeof(bootstrapInfo->SecurityInfo.SecurityMode));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SECURITY_OBJECT, instanceID, LWM2M_SECURITY_OBJECT_PUBLIC_KEY, 0,
                                       bootstrapInfo->SecurityInfo.PublicKey,     strlen(bootstrapInfo->SecurityInfo.PublicKey));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SECURITY_OBJECT, instanceID, LWM2M_SECURITY_OBJECT_SECRET_KEY, 0,
                                       bootstrapInfo->SecurityInfo.SecretKey,     strlen(bootstrapInfo->SecurityInfo.SecretKey));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SECURITY_OBJECT, instanceID, LWM2M_SECURITY_OBJECT_SHORT_SERVER_ID, 0,
                                       &bootstrapInfo->SecurityInfo.ServerID,     sizeof(bootstrapInfo->SecurityInfo.ServerID));
    Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SECURITY_OBJECT, instanceID, LWM2M_SECURITY_OBJECT_CLIENT_HOLD_OFF, 0,
                                       &bootstrapInfo->SecurityInfo.HoldOffTime,  sizeof(bootstrapInfo->SecurityInfo.HoldOffTime));
    instanceID++;

    if (!bootstrapInfo->SecurityInfo.Bootstrap)
    {
        // This LWM2M Object provides the data related to a LWM2M Server.
        // A Bootstrap Server has no such an Object Instance associated to it.
        AwaObjectInstanceID securityInstanceID = FindNextFreeObjectInstanceID(context, LWM2M_SERVER_OBJECT, 0);

        Lwm2mCore_CreateObjectInstance(context, LWM2M_SERVER_OBJECT, securityInstanceID);

#ifdef LWM2M_BOOTSTRAP
        Lwm2mCore_CreateOptionalResource(context, LWM2M_SERVER_OBJECT, securityInstanceID, LWM2M_SERVER_OBJECT_SHORT_SERVER_ID);
        Lwm2mCore_CreateOptionalResource(context, LWM2M_SERVER_OBJECT, securityInstanceID, LWM2M_SERVER_OBJECT_LIFETIME);
        Lwm2mCore_CreateOptionalResource(context, LWM2M_SERVER_OBJECT, securityInstanceID, LWM2M_SERVER_OBJECT_NOTIFICATION_STORING);
        Lwm2mCore_CreateOptionalResource(context, LWM2M_SERVER_OBJECT, securityInstanceID, LWM2M_SERVER_OBJECT_BINDING);
#endif

        Lwm2mCore_CreateOptionalResource(context, LWM2M_SERVER_OBJECT, securityInstanceID, LWM2M_SERVER_OBJECT_MINIMUM_PERIOD);
        Lwm2mCore_CreateOptionalResource(context, LWM2M_SERVER_OBJECT, securityInstanceID, LWM2M_SERVER_OBJECT_MAXIMUM_PERIOD);
        Lwm2mCore_CreateOptionalResource(context, LWM2M_SERVER_OBJECT, securityInstanceID, LWM2M_SERVER_OBJECT_DISABLE_TIMEOUT);

        Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SERVER_OBJECT, securityInstanceID, LWM2M_SERVER_OBJECT_SHORT_SERVER_ID,0,
                                           &bootstrapInfo->ServerInfo.ShortServerID,   sizeof(bootstrapInfo->SecurityInfo.ServerID));
        Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SERVER_OBJECT, securityInstanceID, LWM2M_SERVER_OBJECT_LIFETIME,0,
                                           &bootstrapInfo->ServerInfo.LifeTime,        sizeof(bootstrapInfo->ServerInfo.LifeTime));
        Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SERVER_OBJECT, securityInstanceID, LWM2M_SERVER_OBJECT_MINIMUM_PERIOD,0,
                                           &bootstrapInfo->ServerInfo.MinPeriod,       sizeof(bootstrapInfo->ServerInfo.MinPeriod));
        Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SERVER_OBJECT, securityInstanceID, LWM2M_SERVER_OBJECT_MAXIMUM_PERIOD,0,
                                           &bootstrapInfo->ServerInfo.MaxPeriod,       sizeof(bootstrapInfo->ServerInfo.MaxPeriod));
        Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SERVER_OBJECT, securityInstanceID, LWM2M_SERVER_OBJECT_DISABLE_TIMEOUT,0,
                                           &bootstrapInfo->ServerInfo.DisableTimeout,  sizeof(bootstrapInfo->ServerInfo.DisableTimeout));
        Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SERVER_OBJECT, securityInstanceID, LWM2M_SERVER_OBJECT_NOTIFICATION_STORING,0,
                                           &bootstrapInfo->ServerInfo.Notification,    sizeof(bootstrapInfo->ServerInfo.Notification));
        Lwm2mCore_SetResourceInstanceValue(context, LWM2M_SERVER_OBJECT, securityInstanceID, LWM2M_SERVER_OBJECT_BINDING,0,
                                           bootstrapInfo->ServerInfo.Binding,          strlen(bootstrapInfo->ServerInfo.Binding));
        securityInstanceID ++;
    }

    return 0;
}


