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
                                                        self.config.serverCoapPort,
                                                        self.config.serverLogFile)
        self._serverDaemon.spawn()

        self._clientDaemons = []

        for i in range(self.NUM_CLIENTS):
            print "Creating client %i" % (i,)
            clientEndpointName = self.config.clientEndpointName + str(i);
            clientIPCPort = self.config.clientIpcPort + i;
            clientCOAPPort = self.config.clientCoapPort + i;

            client = overlord.LWM2MClientDaemon(clientIPCPort,
                                                clientCOAPPort,
                                                self.config.clientLogFile,
                                                clientEndpointName,
                                                self.config.bootstrapConfigFile)
            self._clientDaemons.append(client)
            client.spawn()
            # wait for client to register with server
            common.waitForClient(clientEndpointName, self.config.serverIpcPort)

    def doCleanups(self):
        # kill clients first
        for client in self._clientDaemons:
            try:
                client.terminate()
            except AttributeError:
                pass

        try:
            self._serverDaemon.terminate()
        except AttributeError:
            pass


class TestListMultipleClients(TestSpawnMultipleClients):

    def setUp(self):
        super(TestListMultipleClients, self).setUp()

    def test_list_clients_no_args_multiple_clients(self):
        # test that if we run 5 clients, the server can list them all
        expectedStdout = "  1 TestClient0 \n  2 TestClient1 \n  3 TestClient2 \n  4 TestClient3 \n  5 TestClient4 \n"
        expectedStderr = ""
        expectedCode = 0

        code, stdout, stderr = server_list_clients(self.config)
        self.assertEqual(expectedStdout, stdout)
        self.assertEqual(expectedStderr, stderr)
        self.assertEqual(expectedCode, code)

    def test_list_clients_object_values(self):
        # test that if we run 5 clients, the server can list each client and their values
        expectedStdout = "  1 TestClient0 <0/1>,<1/1>,<2/0>,<3/0>,<4/0>,<7>,<5>,<6>\n  2 TestClient1 <0/1>,<1/1>,<2/0>,<3/0>,<4/0>,<7>,<5>,<6>\n  3 TestClient2 <0/1>,<1/1>,<2/0>,<3/0>,<4/0>,<7>,<5>,<6>\n  4 TestClient3 <0/1>,<1/1>,<2/0>,<3/0>,<4/0>,<7>,<5>,<6>\n  5 TestClient4 <0/1>,<1/1>,<2/0>,<3/0>,<4/0>,<7>,<5>,<6>\n"
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

        expectedStdout = "  1 TestClient1 \n  2 TestClient2 \n  3 TestClient3 \n  4 TestClient4 \n"
        expectedStderr = ""
        expectedCode = 0

        code, stdout, stderr = server_list_clients(self.config)
        self.assertEqual(expectedStdout, stdout)
        self.assertEqual(expectedStderr, stderr)
        self.assertEqual(expectedCode, code)
