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
        expectedStdout = "    Resource100[1000/0/100]: %s\n" % (manufacturer,)
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/100=\"%s\"" % (manufacturer,))
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
"""    Timezone[3/0/15]: %s
    CurrentTime[3/0/13]: %d
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


    def test_server_write_client_get_multiple_resources_different_instances(self):
        # test that multiple resources from the different instances can be set on
        # the server and retrieved on the client with single commands
        timezone = "ACME Corp."
        modelNumber = "1234567890"
        expectedStdout = \
"""    Timezone[3/0/15]: %s
    Resource100[1000/0/100]: %s
""" % (timezone, modelNumber)
        expectedStderr = ""
        expectedCode = 0

        server_write(self.config, "/3/0/15=\"%s\"" % (timezone,), "/1000/0/100=\"%s\"" % (modelNumber,))

        result = client_get(self.config, "/3/0/15", "/1000/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_client_set_server_read_single_resource(self):
        # test that a single resource can be written on the client and retrieved on the server
        manufacturer = "ACME Corp."
        expectedStdout = "    Resource100[1000/0/100]: %s\n" % (manufacturer,)
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/100=\"%s\"" % (manufacturer,))
        self.assertEqual(0, result.code)

        result = server_read(self.config, "/1000/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_client_set_server_read_multiple_resources_same_instance(self):
        # test that multiple resources from the same instance can be set on
        # the client and retrieved on the server with single commands
        manufacturer = "ACME Corp."
        modelNumber = "1234567890"
        memoryFree = 55
        expectedStdout = \
"""    Manufacturer[3/0/0]: %s
    ModelNumber[3/0/1]: %s
    MemoryFree[3/0/10]: %d
""" % (manufacturer, modelNumber, memoryFree)
        expectedStderr = ""
        expectedCode = 0

        client_set(self.config, "/3/0/0=\"%s\"" % (manufacturer,),
                     "/3/0/1=\"%s\"" % (modelNumber,),
                     "/3/0/10=%d" % (memoryFree,))

        result = server_read(self.config, "/3/0/0", "/3/0/1", "/3/0/10")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_client_set_server_read_multiple_resources_different_instances(self):
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

        client_set(self.config, "/3/0/0=\"%s\"" % (manufacturer,), "/1000/0/100=\"%s\"" % (modelNumber,))

        result = server_read(self.config, "/3/0/0", "/1000/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_client_subscribe_resource_server_execute(self):
        # test that the client can subscribe to an executable resource and receive a notification
        # when that resource is executed by the server

        # open client subscribe subprocess. Only wait for a single execute of the resource
        subscribeProcess = tools_common.run_non_blocking(tools_common.CLIENT_SUBSCRIBE,
                                                    "--verbose --ipcPort %i --waitCount 1 /3/0/4" % (self.config.clientIpcPort,))

        # wait for subscribe process to start up
        self.assertEqual(subscribeProcess.stdout.readline(), "Subscribe /3/0/4 Execute\n")
        self.assertEqual(subscribeProcess.stdout.readline(), "Waiting for 1 notifications:\n")

        # do execute command
        expectedStdout = "Execute /3/0/4\n"
        expectedStderr = ""
        expectedCode = 0

        result = server_execute(self.config, "--verbose /3/0/4")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        # read subscribe output
        expectedStdout = "Execute 1:\n    Reboot[3/0/4]: \nNO DATA\nUnsubscribe /3/0/4\n"
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
        subscribeProcess = tools_common.run_non_blocking(tools_common.CLIENT_SUBSCRIBE,
                                                    "--verbose --ipcPort %i --waitCount 1 /3/0/4" % (self.config.clientIpcPort,))

        # wait for subscribe process to start up
        self.assertEqual(subscribeProcess.stdout.readline(), "Subscribe /3/0/4 Execute\n")
        self.assertEqual(subscribeProcess.stdout.readline(), "Waiting for 1 notifications:\n")

        # test we can execute a resource, specifying a payload of data
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        inputText = "QmFzZTY0IGlzIGEgZ2VuZXJpYyB0ZXJtIGZvciB"
        result = server_execute_stdin(self.config, inputText, "/3/0/4")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        # read subscribe output
        expectedStdout = "Execute 1:\n    Reboot[3/0/4]: \nDATA: length 39, QmFzZTY0IGlzIGEgZ2VuZXJpYyB0ZXJtIGZvciB\nUnsubscribe /3/0/4\n"
        expectedStderr = ""
        expectedCode = 0

        result = tools_common.non_blocking_get_run_result(subscribeProcess)

        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_client_subscribe_resource_server_multiple_execute_with_payload(self):
        # test that the client can subscribe to an executable resource and receive a notification
        # when that resource is executed by the server. Payload should be printed on the client

        # open client subscribe subprocess. Only wait for a single execute of the resource
        subscribeProcess = tools_common.run_non_blocking(tools_common.CLIENT_SUBSCRIBE,
                                                    "--verbose --ipcPort %i --waitCount 1 /3/0/4" % (self.config.clientIpcPort,))

        # wait for subscribe process to start up
        self.assertEqual(subscribeProcess.stdout.readline(), "Subscribe /3/0/4 Execute\n")
        self.assertEqual(subscribeProcess.stdout.readline(), "Waiting for 1 notifications:\n")

        # test we can execute a resource, specifying a payload of data
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        inputText = "QmFzZTY0IGlzIGEgZ2VuZXJpYyB0ZXJtIGZvciB"
        result = server_execute_stdin(self.config, inputText, "/3/0/4 /3/0/4")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        # read subscribe output
        expectedStdout = "Execute 1:\n    Reboot[3/0/4]: \nDATA: length 39, QmFzZTY0IGlzIGEgZ2VuZXJpYyB0ZXJtIGZvciB\nUnsubscribe /3/0/4\n"
        expectedStderr = ""
        expectedCode = 0

        result = tools_common.non_blocking_get_run_result(subscribeProcess)

        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_client_subscribe_resource_server_write(self):
        # test that the client can subscribe to a resource and receive a notification
        # when that resource is changed by the server through the write function

        # open client subscribe subprocess. Only wait for a single execute of the resource
        subscribeProcess = tools_common.run_non_blocking(tools_common.CLIENT_SUBSCRIBE,
                                                    "--verbose --ipcPort %i --waitCount 1 /3/0/15" % (self.config.clientIpcPort,))

        # wait for subscribe process to start up
        self.assertEqual(subscribeProcess.stdout.readline(), "Subscribe /3/0/15 Change\n")
        self.assertEqual(subscribeProcess.stdout.readline(), "Waiting for 1 notifications:\n")

        # do write command
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/3/0/15=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        # read subscribe output
        expectedStdout = "Notify 1:\n    Timezone[3/0/15]: abc\nUnsubscribe /3/0/15\n"
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
        self.assertEqual(observeProcess.stdout.readline(), "Observe /3/0/1\n")
        self.assertEqual(observeProcess.stdout.readline(), "Waiting for 1 notifications:\n")

        # do set command
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/3/0/1=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        # read subscribe output
        expectedStdout = "Notify 1:\n    ModelNumber[3/0/1]: abc\nClean up\n"
        expectedStderr = ""
        expectedCode = 0

        result = tools_common.non_blocking_get_run_result(observeProcess)

        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
