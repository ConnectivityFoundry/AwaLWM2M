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
        expectedStdout = "Client: TestClient\n\n"
        expectedStderr = ""
        expectedCode = 0
        result = server_list_clients(self.config)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedCode, result.code)


class TestListNoClients(common.SpawnDaemonsTestCase):

    def setUp(self):
        common.setConfigurationClass(tools_common.NoClientTestConfiguration)
        super(TestListNoClients, self).setUp()

    def test_list_clients_no_args_no_clients(self):
        # test that when no clients are connected, a suitable response
        # should be returned when the client list is requested
        expectedStdout = "No clients connected.\n"
        expectedStderr = ""
        expectedCode = 0

        result = server_list_clients(self.config)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)


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

