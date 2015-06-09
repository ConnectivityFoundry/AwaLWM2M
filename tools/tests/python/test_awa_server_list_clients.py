# Tests related to basic list client operations

import unittest

import overlord
import common
import tools_common

def server_list_clients(config, *args):
    return tools_common.run_server(config, tools_common.SERVER_LIST_CLIENTS, *args)

class TestListSingleClient(tools_common.AwaTest):

    def test_list_clients_no_args_one_client(self):
        # test that we should be able to list a single client's
        # endpoint name when only that one is connected
        expectedStdout = "  1 TestClient \n"
        expectedStderr = ""
        expectedCode = 0

        code, stdout, stderr = server_list_clients(self.config)
        self.assertEqual(expectedStdout, stdout)
        self.assertEqual(expectedStderr, stderr)
        self.assertEqual(expectedCode, code)


class TestListNoClients(tools_common.AwaTest):

    def setUp(self):
        common.setConfigurationClass(tools_common.NoClientTestConfiguration)
        super(TestListNoClients, self).setUp()

    def test_list_clients_no_args_no_clients(self):
        # test that when no clients are connected, a suitable response
        # should be returned when the client list is requested
        expectedStdout = "No clients connected.\n"
        expectedStderr = ""
        expectedCode = 0

        code, stdout, stderr = server_list_clients(self.config)
        self.assertEqual(expectedStdout, stdout)
        self.assertEqual(expectedStderr, stderr)
        self.assertEqual(expectedCode, code)


class TestListClientsBasic(tools_common.BasicTestCase):

    @unittest.skip("test_list_clients_no_args_no_server skipped - fix hang (timeout?)")
    def test_list_clients_no_args_no_server(self):
        # test the list-clients tool when no server is running.
        # currently hangs
        expectedStdout = "Couldn't connect to FlowDeviceMgmtServer"
        expectedStderr = ""
        expectedCode = 0

        code, stdout, stderr = server_list_clients(self.config)
        self.assertEqual(expectedStdout, stdout)
        self.assertEqual(expectedStderr, stderr)
        self.assertEqual(expectedCode, code)

