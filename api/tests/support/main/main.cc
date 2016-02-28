
#include <gtest/gtest.h>
#include <string>
#include <stdio.h>

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

    auto result = RUN_ALL_TESTS();
    cmdline_parser_free(&ai);
    return result;
}
