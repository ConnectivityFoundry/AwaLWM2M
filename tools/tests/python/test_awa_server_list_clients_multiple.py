#/************************************************************************************************************************
# Copyright (c) 2016, Imagination Technologies Limited and/or its affiliated group companies.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
# following conditions are met:
#     1. Redistributions of source code must retain the above copyright notice, this list of conditions and the
#        following disclaimer.
#     2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
#        following disclaimer in the documentation and/or other materials provided with the distribution.
#     3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote
#        products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
# USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#************************************************************************************************************************/

# Tests and daemon setup related to listing multiple clients on a server

import unittest
import time

import overlord
import common
import tools_common
import config

from test_awa_server_list_clients import server_list_clients


class TestSpawnMultipleClients(unittest.TestCase):
    """TestCase class that spawns LWM2M Server and multiple Client daemons before each test, then kills them afterwards."""

    NUM_CLIENTS = 5

    def setUp(self):
        self.config = config.Config(tools_common.DefaultTestConfiguration)

        # start server first
        self._serverDaemon = overlord.LWM2MServerDaemon(self.config.serverIpcPort,
                                                        self.config.serverAddress,
                                                        self.config.serverCoapPort,
                                                        self.config.serverLogFile)
        self.addCleanup(self._serverDaemon.terminate)
        self._serverDaemon.spawn()

        self._clientDaemons = []

        for i in range(self.NUM_CLIENTS):
            print "Creating client %i" % (i,)
            clientEndpointName = self.config.clientEndpointName + str(i);
            clientIPCPort = self.config.clientIpcPort + i;
            clientCoapPort = self.config.clientCoapPort + i;

            client = overlord.LWM2MClientDaemon(clientIPCPort,
                                                clientCoapPort,
                                                self.config.clientLogFile,
                                                clientEndpointName,
                                                self.config.bootstrapConfigFile,
                                                self.config.objectDefinitionsFile)
            self.addCleanup(client.terminate)
            self._clientDaemons.append(client)
            client.spawn()
            # wait for client to register with server
            common.waitForClient(clientEndpointName, self.config.serverIpcPort)

class TestListMultipleClients(TestSpawnMultipleClients):

    def setUp(self):
        super(TestListMultipleClients, self).setUp()

    def test_list_clients_no_args_multiple_clients(self):
        # test that if we run 5 clients, the server can list them all
        expectedStdout = "Client: TestClient0\n\nClient: TestClient1\n\nClient: TestClient2\n\nClient: TestClient3\n\nClient: TestClient4\n\n"
        expectedStderr = ""
        expectedCode = 0

        code, stdout, stderr = server_list_clients(self.config)
        self.assertEqual(expectedStdout, stdout)
        self.assertEqual(expectedStderr, stderr)
        self.assertEqual(expectedCode, code)

    def test_list_clients_object_values(self):
        # test that if we run 5 clients, the server can list each client and their values
        expectedStdout = "Client: TestClient0\n  /1/0     LWM2MServer\n  /2/0     LWM2MAccessControl\n  /2/1     LWM2MAccessControl\n  /2/2     LWM2MAccessControl\n  /2/3     LWM2MAccessControl\n  /3/0     Device\n  /4       ConnectivityMonitoring\n  /5       FirmwareUpdate\n  /6       Location\n  /7       ConnectivityStatistics\n\nClient: TestClient1\n  /1/0     LWM2MServer\n  /2/0     LWM2MAccessControl\n  /2/1     LWM2MAccessControl\n  /2/2     LWM2MAccessControl\n  /2/3     LWM2MAccessControl\n  /3/0     Device\n  /4       ConnectivityMonitoring\n  /5       FirmwareUpdate\n  /6       Location\n  /7       ConnectivityStatistics\n\nClient: TestClient2\n  /1/0     LWM2MServer\n  /2/0     LWM2MAccessControl\n  /2/1     LWM2MAccessControl\n  /2/2     LWM2MAccessControl\n  /2/3     LWM2MAccessControl\n  /3/0     Device\n  /4       ConnectivityMonitoring\n  /5       FirmwareUpdate\n  /6       Location\n  /7       ConnectivityStatistics\n\nClient: TestClient3\n  /1/0     LWM2MServer\n  /2/0     LWM2MAccessControl\n  /2/1     LWM2MAccessControl\n  /2/2     LWM2MAccessControl\n  /2/3     LWM2MAccessControl\n  /3/0     Device\n  /4       ConnectivityMonitoring\n  /5       FirmwareUpdate\n  /6       Location\n  /7       ConnectivityStatistics\n\nClient: TestClient4\n  /1/0     LWM2MServer\n  /2/0     LWM2MAccessControl\n  /2/1     LWM2MAccessControl\n  /2/2     LWM2MAccessControl\n  /2/3     LWM2MAccessControl\n  /3/0     Device\n  /4       ConnectivityMonitoring\n  /5       FirmwareUpdate\n  /6       Location\n  /7       ConnectivityStatistics\n\n"

        expectedStderr = ""
        expectedCode = 0

        code, stdout, stderr = server_list_clients(self.config, "-o")
        self.assertEqual(expectedStdout, stdout)
        self.assertEqual(expectedStderr, stderr)
        self.assertEqual(expectedCode, code)


class TestListClientsAfterClientDisconnect(TestSpawnMultipleClients):

    def setUp(self):
        super(TestListClientsAfterClientDisconnect, self).setUp()

    def test_list_clients_after_client_disconnect(self):
        # test that if we run 5 clients and kill the first (TestClient0), only the 4 remaining clients should be printed
        try:
            self._clientDaemons[0].terminate()
            print "Killed self._clientDaemons[0]"
        except AttributeError:
            print "Failed to kill self._clientDaemons[0]"
            pass

        expectedStdout = "Client: TestClient1\n\nClient: TestClient2\n\nClient: TestClient3\n\nClient: TestClient4\n\n"
        expectedStderr = ""
        expectedCode = 0

        code, stdout, stderr = server_list_clients(self.config)
        self.assertEqual(expectedStdout, stdout)
        self.assertEqual(expectedStderr, stderr)
        self.assertEqual(expectedCode, code)
