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


#include <gtest/gtest.h>
#include <string>
#include <stdio.h>
#include <unistd.h>

#include "../support.h"
#include "main_cmdline.h"

GTEST_API_ int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);

    // extra arguments
    struct gengetopt_args_info ai;
    if (cmdline_parser(argc, argv, &ai) != 0)
    {
        cmdline_parser_free(&ai);
        exit(1);
    }

    Awa::global::SetGlobalDefaults();

    Awa::global::logLevel = ai.logLevel_given ? ai.logLevel_arg : 1;

    Awa::global::clientIpcPort = ai.clientIpcPort_given ? ai.clientIpcPort_arg : Awa::defaults::clientIpcPort;
    Awa::global::spawnClientDaemon = Awa::global::clientIpcPort == Awa::defaults::clientIpcPort;
    std::cout << "Using Client IPC Port: " << Awa::global::clientIpcPort << std::endl;

    Awa::global::serverIpcPort = ai.serverIpcPort_given ? ai.serverIpcPort_arg : Awa::defaults::serverIpcPort;
    Awa::global::spawnServerDaemon = Awa::global::serverIpcPort == Awa::defaults::serverIpcPort;
    std::cout << "Using Server IPC Port: " << Awa::global::serverIpcPort << std::endl;

    Awa::global::clientLocalCoapPort = ai.clientLocalCoapPort_given ? ai.clientLocalCoapPort_arg : Awa::defaults::clientLocalCoapPort;
    std::cout << "Using Client local CoAP Port: " << Awa::global::clientLocalCoapPort << std::endl;
    Awa::global::serverCoapPort = ai.serverCoapPort_given ? ai.serverCoapPort_arg : Awa::defaults::serverCoapPort;
    std::cout << "Using Server CoAP Port: " << Awa::global::serverCoapPort << std::endl;
    Awa::global::bootstrapServerCoapPort = ai.bootstrapServerCoapPort_given ? ai.bootstrapServerCoapPort_arg : Awa::defaults::bootstrapServerCoapPort;
    std::cout << "Using Bootstrap Server CoAP Port: " << Awa::global::bootstrapServerCoapPort << std::endl;

    Awa::global::coapClientPath = ai.coapClientPath_arg;
    Awa::global::clientDaemonPath = ai.clientDaemonPath_arg;
    Awa::global::serverDaemonPath = ai.serverDaemonPath_arg;
    Awa::global::bootstrapServerDaemonPath = ai.bootstrapDaemonPath_arg;
    Awa::global::bootstrapServerConfig = ai.bootstrapConfig_arg;
    Awa::global::objectDefinitionsFile = ai.objectDefinitions_arg;

    Awa::global::timeout = ai.defaultTimeout_given ? ai.defaultTimeout_arg : Awa::defaults::timeout;

    auto result = RUN_ALL_TESTS();
    cmdline_parser_free(&ai);
    return result;
}
