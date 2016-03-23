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

# Tests related to server-client interaction

import unittest
import subprocess
import time

import overlord
import common
from collections import namedtuple

import tools_common
from tools_common import CustomObject
from tools_common import CustomResource

from test_awa_client_define import client_define
from test_awa_client_get import client_get
from test_awa_client_set import client_set
from test_awa_client_delete import client_delete
from test_awa_client_subscribe import client_subscribe
from test_awa_server_define import server_define
from test_awa_server_delete import server_delete
from test_awa_server_execute import server_execute
from test_awa_server_execute import server_execute_stdin
from test_awa_server_list_clients import server_list_clients
from test_awa_server_observe import server_observe
from test_awa_server_read import server_read
from test_awa_server_write import server_write


class TestClientServer(tools_common.AwaTest):

    def test_server_write_client_get_single_resource(self):
        # test that a single resource can be written on the server and retrieved on the client
        manufacturer = "ACME Corp."
        expectedStdout = "Object1000[/1000/0]:\n    Resource100[/1000/0/100]: %s\n" % (manufacturer,)
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/100=\"%s\"" % (manufacturer,))
        self.assertEqual("", result.stderr)
        self.assertEqual("", result.stdout)
        self.assertEqual(0, result.code)

        result = client_get(self.config, "/1000/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_server_write_client_get_multiple_resources_same_instance(self):
        # test that multiple resources from the same instance can be set on
        # the server and retrieved on the client with single commands
        timezone = "ACME Corp."
        currentTime = 123456789
        expectedStdout = \
"""Device[/3/0]:
    Timezone[/3/0/15]: %s
    CurrentTime[/3/0/13]: %d
""" % (timezone, currentTime)
        expectedStderr = ""
        expectedCode = 0

        server_write(self.config,
                     "/3/0/15=\"%s\"" % (timezone,),
                     "/3/0/13=%d" % (currentTime,))

        result = client_get(self.config, "/3/0/15", "/3/0/13")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    @unittest.skip("Multiple writes in a single request are not supported")
    def test_server_write_client_get_multiple_resources_different_instances_single_write(self):
        # test that multiple resources from the different instances can be set on
        # the server and retrieved on the client with single commands
        timezone = "ACME Corp."
        modelNumber = "1234567890"
        expectedStdout = \
"""Device[/3/0]:
    Timezone[/3/0/15]: %s
Object1000[/1000/0]:
    Resource100[/1000/0/100]: %s
""" % (timezone, modelNumber)
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/3/0/15=\"%s\"" % (timezone,), "/1000/0/100=\"%s\"" % (modelNumber,))
        self.assertEqual(0, result.code)

        result = client_get(self.config, "/3/0/15", "/1000/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
    def test_server_write_client_get_multiple_resources_different_instances(self):
        # test that multiple resources from the different instances can be set on
        # the server and retrieved on the client with single commands
        timezone = "ACME Corp."
        modelNumber = "1234567890"
        expectedStdout = \
"""Device[/3/0]:
    Timezone[/3/0/15]: %s
Object1000[/1000/0]:
    Resource100[/1000/0/100]: %s
""" % (timezone, modelNumber)
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/3/0/15=\"%s\"" % (timezone,))
        self.assertEqual(0, result.code)
        
        result = server_write(self.config, "/1000/0/100=\"%s\"" % (modelNumber,))
        self.assertEqual(0, result.code)

        result = client_get(self.config, "/3/0/15", "/1000/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_client_set_server_read_single_resource(self):
        # test that a single resource can be written on the client and retrieved on the server
        manufacturer = "ACME Corp."
        expectedStdout = "Object1000[/1000/0]:\n    Resource100[/1000/0/100]: %s\n" % (manufacturer,)
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/100=\"%s\"" % (manufacturer,))
        self.assertEqual(0, result.code)

        result = server_read(self.config, "/1000/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    @unittest.skip("Reading multiple resources in single request is currently unsupported")
    def test_client_set_server_read_multiple_resources_same_instance(self):
        # test that multiple resources from the same instance can be set on
        # the client and retrieved on the server with single commands
        manufacturer = "ACME Corp."
        memoryFree = 55
        temperature = 24.6
        expectedStdout = \
"""    Manufacturer[1000/0/100]: %s
    ModelNumber[1000/0/101]: %s
    MemoryFree[1000/0/102]: %d
""" % (manufacturer, memoryFree, temperature)
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/100=\"%s\"" % (manufacturer,),
                     "/1000/0/101=\"%d\"" % (memoryFree,),
                     "/1000/0/102=%f" % (temperature,))
        
        result = server_read(self.config, "/1000/0/100", "/1000/0/101", "/1000/0/102")
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedCode, result.code)

    @unittest.skip("Multiple reads in single operation currently unsupported")
    def test_client_set_server_read_multiple_resources_different_instances_single_read_operation(self):
        # test that multiple resources from the different instances can be set on
        # the client and retrieved on the server with single commands
        manufacturer = "ACME Corp."
        modelNumber = "1234567890"
        expectedStdout = \
"""    Manufacturer[3/0/0]: %s
    Resource100[1000/0/100]: %s
""" % (manufacturer, modelNumber)
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/3/0/0=\"%s\"" % (manufacturer,), "/1000/0/100=\"%s\"" % (modelNumber,))
        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertEqual(0, result.code)

        result = server_read(self.config, "/3/0/0", "/1000/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_client_subscribe_resource_server_execute(self):
        # test that the client can subscribe to an executable resource and receive a notification
        # when that resource is executed by the server

        # open client subscribe subprocess. Only wait for a single execute of the resource
        port = self.config.clientIpcPort
        subscribeProcess = tools_common.run_non_blocking(tools_common.CLIENT_SUBSCRIBE,
                                                    "--verbose --ipcPort %i --waitCount 1 /3/0/4" % (port,))

        # wait for subscribe process to start up
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session IPC configured for UDP: address 127.0.0.1, port %d" % (port,))
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session connected")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Subscribe /3/0/4 Execute\n")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Waiting for 1 notifications:\n")

        # test we can execute a resource, specifying no payload
        expectedStdout = "Target /3/0/4 executed successfully\n"
        expectedStderr = ""
        expectedCode = 0
        result = server_execute(self.config, "/3/0/4")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        # read subscribe output
        expectedStdout = "Execute 1:\nNO DATA\nSession disconnected\n"
        expectedStderr = ""
        expectedCode = 0

        result = tools_common.non_blocking_get_run_result(subscribeProcess)

        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_client_subscribe_resource_server_execute_with_payload(self):
        # test that the client can subscribe to an executable resource and receive a notification
        # when that resource is executed by the server. Payload should be printed on the client

        # open client subscribe subprocess. Only wait for a single execute of the resource
        port = self.config.clientIpcPort
        subscribeProcess = tools_common.run_non_blocking(tools_common.CLIENT_SUBSCRIBE,
                                                    "--verbose --ipcPort %i --waitCount 1 /3/0/4" % (port,))

        # wait for subscribe process to start up
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session IPC configured for UDP: address 127.0.0.1, port %d" % (port,))
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session connected")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Subscribe /3/0/4 Execute\n")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Waiting for 1 notifications:\n")

        # test we can execute a resource, specifying a payload of data
        expectedStdout = "Target /3/0/4 executed successfully\n"
        expectedStderr = ""
        expectedCode = 0
        inputText = "QmFzZTY0IGlzIGEgZ2VuZXJpYyB0ZXJtIGZvciB"
        result = server_execute_stdin(self.config, inputText, "/3/0/4")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        hexBytes = ""
        for c in inputText:
            hexBytes += c.encode("hex") + " "

        # read subscribe output
        expectedStdout = "Execute 1:\nDATA: length 39, payload: [" + hexBytes +"]\nSession disconnected\n"
        expectedStderr = ""
        expectedCode = 0

        result = tools_common.non_blocking_get_run_result(subscribeProcess)

        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    @unittest.skip("Multiple executions in a single request is currently supported")
    def test_client_subscribe_resource_server_multiple_execute_with_payload(self):
        self.assertTrue(False)

    def test_client_subscribe_resource_server_write(self):
        # test that the client can subscribe to a resource and receive a notification
        # when that resource is changed by the server through the write function

        # open client subscribe subprocess. Only wait for a single execute of the resource
        port = self.config.clientIpcPort
        subscribeProcess = tools_common.run_non_blocking(tools_common.CLIENT_SUBSCRIBE,
                                                    "--verbose --ipcPort %i --waitCount 1 /3/0/15" % (port,))

        # wait for subscribe process to start up
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session IPC configured for UDP: address 127.0.0.1, port %d" % (port,))
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session connected")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Subscribe /3/0/15 Change\n")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Waiting for 1 notifications:\n")
        
        # do write command
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/3/0/15=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        # read subscribe output
        expectedStdout = "Notify 1:\nChanged: /3/0/15 Resource Modified:\nDevice[/3/0]:\n    Timezone[/3/0/15]: abc\nSession disconnected\n"
        expectedStderr = ""
        expectedCode = 0

        result = tools_common.non_blocking_get_run_result(subscribeProcess)

        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_server_observe_resource_client_set(self):
        # test that the server can observe changes to a resource and receive a notification
        # when that resource is changed by the client through the set function

        # open client subscribe subprocess. Only wait for a single execute of the resource
        port = self.config.serverIpcPort
        clientEndpointName = self.config.clientEndpointName
        observeProcess = tools_common.run_non_blocking(tools_common.SERVER_OBSERVE,
                                                    "--verbose --ipcPort %i --clientID %s --waitCount 1 /3/0/1" % (port, clientEndpointName))

        # wait for observe process to start up
        self.assertEqual(tools_common.strip_prefix(observeProcess.stdout.readline()), "Session IPC configured for UDP: address 127.0.0.1, port %d" % (port,))
        self.assertEqual(tools_common.strip_prefix(observeProcess.stdout.readline()), "Session connected")
        self.assertEqual(tools_common.strip_prefix(observeProcess.stdout.readline()), "Observe /3/0/1\n")
        self.assertEqual(tools_common.strip_prefix(observeProcess.stdout.readline()), "Waiting for 1 notifications:\n")
        self.assertEqual(tools_common.strip_prefix(observeProcess.stdout.readline()), "Notify 0 from clientID TestClient:\n")
        self.assertEqual(tools_common.strip_prefix(observeProcess.stdout.readline()), "Changed: /3/0/1 Resource Modified:\n")

        # do set command
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/3/0/1=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        # read subscribe output
        expectedStdout = "Device[/3/0]:\n    ModelNumber[/3/0/1]: Awa Client\nNotify 1 from clientID TestClient:\nChanged: /3/0/1 Resource Modified:\nDevice[/3/0]:\n    ModelNumber[/3/0/1]: abc\nSession disconnected\n"
        expectedStderr = ""
        expectedCode = 0

        result = tools_common.non_blocking_get_run_result(observeProcess)

        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
