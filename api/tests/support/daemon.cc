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

#include <memory>

#include "daemon.h"

namespace Awa {

namespace global {

// initialization values are subject to Static Initialization Order Fiasco
int clientIpcPort = 0;
int serverIpcPort = 0;
int clientLocalCoapPort = 0;
int serverCoapPort = 0;
int bootstrapServerCoapPort = 0;
bool spawnClientDaemon = false;
bool spawnServerDaemon = false;
bool spawnBootstrapServerDaemon = false;
const char * coapClientPath = nullptr;
const char * clientDaemonPath = nullptr;
const char * serverDaemonPath = nullptr;
const char * bootstrapServerDaemonPath = nullptr;
const char * bootstrapServerConfig = nullptr;
const char * clientEndpointName = nullptr;
const char * clientLogFile = nullptr;
const char * serverLogFile = nullptr;
const char * bootstrapServerLogFile = nullptr;
const char * objectDefinitionsFile = nullptr;
int timeout = 0;

// initialise globals with a function to avoid Static Initialization Order Fiasco
void SetDaemonGlobalDefaults(void)
{
    global::clientIpcPort = defaults::clientIpcPort;
    global::serverIpcPort = defaults::serverIpcPort;
    global::clientLocalCoapPort = defaults::clientLocalCoapPort;
    global::serverCoapPort = defaults::serverCoapPort;
    global::spawnClientDaemon = true;
    global::spawnServerDaemon = true;
    global::spawnBootstrapServerDaemon = true;
    global::coapClientPath = defaults::coapClientPath;
    global::clientDaemonPath = defaults::clientDaemonPath;
    global::serverDaemonPath = defaults::serverDaemonPath;
    global::bootstrapServerDaemonPath = defaults::bootstrapServerDaemonPath;
    global::bootstrapServerConfig = defaults::bootstrapServerConfig;
    global::clientEndpointName = defaults::clientEndpointName;
    global::clientLogFile = defaults::clientLogFile;
    global::serverLogFile = defaults::serverLogFile;
    global::bootstrapServerLogFile = defaults::bootstrapServerLogFile;
    global::objectDefinitionsFile = defaults::objectDefinitionsFile;
}

} // namespace global

} // namespace Awa
