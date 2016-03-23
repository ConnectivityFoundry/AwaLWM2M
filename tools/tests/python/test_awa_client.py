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

# Tests related to client-side operations only

import unittest
import subprocess
import time
from collections import namedtuple

import overlord
import common
import config

import tools_common
from tools_common import CustomObject
from tools_common import CustomResource

from test_awa_client_define import client_define
from test_awa_client_get import client_get
from test_awa_client_set import client_set
from test_awa_client_delete import client_delete
from test_awa_client_subscribe import client_subscribe

class TestClient(tools_common.AwaTest):

    def test_set_get_single_resource_string(self):
        # test that a single string resource can be set and retrieved
        manufacturer = "ACME Corp."
        expectedStdout = "Object1000[/1000/0]:\n    Resource100[/1000/0/100]: %s\n" % (manufacturer,)
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/100=\"%s\"" % (manufacturer,))
        self.assertEqual(expectedCode, result.code)
        
        result = client_get(self.config, "/1000/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
    def test_set_get_single_resource_integer(self):
        # test that a single integer resource can be set and retrieved
        value = 3
        expectedStdout = "Object1000[/1000/0]:\n    Resource101[/1000/0/101]: %d\n" % (value,)
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "--verbose /1000/0/101=%i" % (value,))
        self.assertEqual(expectedCode, result.code)
        
        result = client_get(self.config, "/1000/0/101")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
    
    def test_set_get_single_resource_float(self):
        # test that a single float resource can be set and retrieved
        value = 3.5
        expectedStdout = "Object1000[/1000/0]:\n    Resource102[/1000/0/102]: %g\n" % (value,)
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/102=%f" % (value,))
        self.assertEqual(expectedCode, result.code)
        
        result = client_get(self.config, "/1000/0/102")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
    
    def test_set_get_single_resource_boolean_true(self):
        # test that a single boolean resource can be set and retrieved
        value = True

        expectedStdout = "Session IPC configured for UDP: address 127.0.0.1, port %d\nSession connected\nSet Boolean /1000/0/103 <- True\nSet operation completed successfully.\nSession disconnected\n" % (self.config.clientIpcPort,)
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "--verbose /1000/0/103=%r" % (value,))

        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
        expectedStdout = "Object1000[/1000/0]:\n    Resource103[/1000/0/103]: True\n"
        
        result = client_get(self.config, "/1000/0/103")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
    def test_set_get_single_resource_boolean_false(self):
        # test that a single boolean resource can be set and retrieved
        value = False

        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/103=%r" % (value,))

        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
        expectedStdout = "Object1000[/1000/0]:\n    Resource103[/1000/0/103]: %r\n" % (value,)
        
        result = client_get(self.config, "/1000/0/103")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
    def test_set_get_single_resource_boolean_numbers(self):
        # test correct values for setting boolean resource with integers: 0,1,2 (False,True,True)
        for value in range(3):
            expectedStdout = ""
            expectedStderr = ""
            expectedCode = 0
    
            result = client_set(self.config, "/1000/0/103=%d" % (value,))
    
            self.assertEqual(expectedStdout, result.stdout)
            self.assertEqual(expectedStderr, result.stderr)
            self.assertEqual(expectedCode, result.code)
            
            expectedStdout = "Object1000[/1000/0]:\n    Resource103[/1000/0/103]: %r\n" % (bool(value),)
            #print "Boolean expectedStdout: %s" % (expectedStdout,)
            result = client_get(self.config, "/1000/0/103")
            self.assertEqual(expectedStdout, result.stdout)
            self.assertEqual(expectedStderr, result.stderr)
            self.assertEqual(expectedCode, result.code)
        
    def test_set_get_single_resource_time(self):
        # test that a single time resource (64 bit integer) can be set and retrieved
        value = 1442972971
        expectedStdout = "Object1000[/1000/0]:\n    Resource104[/1000/0/104]: %d\n" % (value,)
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/104=%d" % (value,))
        self.assertEqual(expectedCode, result.code)
        
        result = client_get(self.config, "/1000/0/104")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
    
    def test_set_get_single_resource_objlink(self):
        # test that a single object link resource can be set and retrieved
        link = "/3/0"

        result = client_set(self.config, "/1000/0/106=\"%s\"" % (link,))
        self.assertEqual(0, result.code)
        
        expectedStdout = "Object1000[/1000/0]:\n    Resource106[/1000/0/106]: ObjectLink[%s]\n" % (link.strip('/').replace('/', ':'))
        expectedStderr = ""
        expectedCode = 0
        
        result = client_get(self.config, "/1000/0/106")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
    def test_set_single_resource_none_executable(self):
        # test that a single none resource (executable) cannot be set
        value = 12345
        expectedStdout = ""
        expectedStderr = "Resource /1000/0/107 is of type None and cannot be set\n"
        expectedCode = 0

        result = client_set(self.config, "/1000/0/107=%d" % (value,))
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
    def test_get_single_resource_none_executable(self):
        # test that a single none resource (executable) cannot be retrieved
        expectedStdout = "Object1000[/1000/0]:\n    Resource107[/1000/0/107]: [Executable]\n"
        expectedStderr = ""
        expectedCode = 0
        
        result = client_get(self.config, "/1000/0/107")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_set_get_multiple_resources_same_instance(self):
        # test that multiple resources from the same instance can be set and retrieved with a single command
        manufacturer = "ACME Corp."
        modelNumber = "1234567890"
        memoryFree = 55
        expectedStdout = \
"""Device[/3/0]:
    Manufacturer[/3/0/0]: %s
    ModelNumber[/3/0/1]: %s
    MemoryFree[/3/0/10]: %d
""" % (manufacturer, modelNumber, memoryFree)
        expectedStderr = ""
        expectedCode = 0

        client_set(self.config, "/3/0/0=\"%s\"" % (manufacturer,))
        client_set(self.config, "/3/0/1=\"%s\"" % (modelNumber,))
        client_set(self.config, "/3/0/10=%d" % (memoryFree,))

        result = client_get(self.config, "/3/0/0", "/3/0/1", "/3/0/10")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_set_get_multiple_resources_different_instances(self):
        # test that multiple resources from different instances can be set and retrieved with a single command
        manufacturer = "ACME Corp."
        modelNumber = "1234567890"
        expectedStdout = \
"""Device[/3/0]:
    Manufacturer[/3/0/0]: %s
Object1000[/1000/0]:
    Resource100[/1000/0/100]: %s
""" % (manufacturer, modelNumber)
        expectedStderr = ""
        expectedCode = 0

        client_set(self.config, "/3/0/0=\"%s\"" % (manufacturer,), "/1000/0/100=\"%s\"" % (modelNumber,))

        result = client_get(self.config, "/3/0/0", "/1000/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_set_get_invalid_value(self):
        # set MemoryFree (int) as string value. Currently will set the value to 0
        expectedStdout = "Device[/3/0]:\n    MemoryFree[/3/0/10]: 15\n"
        expectedStderr = ""
        expectedCode = 0
        
        result = client_get(self.config, "/3/0/10")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        
        result = client_set(self.config, "/3/0/10=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
        expectedStdout = "Device[/3/0]:\n    MemoryFree[/3/0/10]: 0\n"
        expectedStderr = ""
        expectedCode = 0
        
        result = client_get(self.config, "/3/0/10")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
    
    def test_set_get_delete_get(self):
        # test that we can no longer get the value of a resource after it has been deleted
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        
        result = client_set(self.config, "/1000/0/100=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
        expectedStdout = "Object1000[/1000/0]:\n    Resource100[/1000/0/100]: abc\n"
        expectedStderr = ""
        expectedCode = 0
        
        result = client_get(self.config, "/1000/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        
        result = client_delete(self.config, "/1000/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
        expectedStdout = ""
        expectedStderr = "AwaClientGetOperation_Perform failed\nFailed to retrieve /1000/0/100: AwaError_PathNotFound\n"
        expectedCode = 1
        
        result = client_get(self.config, "/1000/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_set_multiple_instances_on_single_instance_object(self):
        #test that we can't create multiple object instances on a single instance object
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        
        result = client_set(self.config, "/1000/0/100=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
        expectedStdout = ""
        expectedStderr = "AwaClientSetOperation_Perform failed\nFailed to set on path /1000/1: AwaError_CannotCreate\n"
        expectedCode = 1
        
        result = client_set(self.config, "--create /1000/1")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_set_multiple_instances_on_single_instance_resource(self):
        #test that we can't set multiple resource instances on a single instance resource
        expectedStdout = ""
        expectedStderr = "Error: resource /1000/0/100 is not an array; do not specify a resource instance ID\n"
        expectedCode = 0
        
        result = client_set(self.config, "/1000/0/100/0=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
        expectedStdout = ""
        expectedStderr = "Error: resource /1000/0/100 is not an array; do not specify a resource instance ID\n"
        expectedCode = 0
        
        result = client_set(self.config, "/1000/0/100/1=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
    def test_get_write_only_resource(self):
        # test that we can read from a write only resource - client should have power to do anything
        customObjects = (
            CustomObject("Object1001", 1001, False, "single", (
                    CustomResource("Resource100", 100, "string",  "single", "optional", "w"),
            )),
        )
        
        #define and set the write only object
        params = tools_common.create_define_command(customObjects)
        result = client_define(self.config, *params)
        self.assertEqual(0, result.code)
        result = client_set(self.config, "--create /1001/0 --create /1001/0/100 /1001/0/100=abc")
        self.assertEqual(0, result.code)
        
        expectedStdout = "Object1001[/1001/0]:\n    Resource100[/1001/0/100]: abc\n"
        expectedStderr = ""
        expectedCode = 0
        
        # attempt to get a write only object, should not fail on the client only.
        result = client_get(self.config, "/1001/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
    def test_set_read_only_resource(self):
        # test that we should still be able to set a read only resource (client should have permission to do anything)
        customObjects = (
            CustomObject("Object1001", 1001, False, "single", (
                    CustomResource("Resource100", 100, "string",  "single", "optional", "r"),
            )),
        )
        params = tools_common.create_define_command(customObjects)
        result = client_define(self.config, *params)
        self.assertEqual(0, result.code)
        
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        
        # set a read only object
        result = client_set(self.config, "--create /1001/0 --create /1001/0/100 /1001/0/100=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
        expectedStdout = "Object1001[/1001/0]:\n    Resource100[/1001/0/100]: abc\n"
        expectedStderr = ""
        expectedCode = 0
        
        # check that the value was set correctly
        result = client_get(self.config, "/1001/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    
    def test_subscribe_resource_single_change(self):
        # test that we can subscribe to a single change of value on a specified resource
        
        # open client subscribe subprocess. Only wait for a single change to the resource
        
        #import pdb; pdb.set_trace()
        
        port = self.config.clientIpcPort
        subscribeProcess = tools_common.run_non_blocking(tools_common.CLIENT_SUBSCRIBE, 
                                                    "--verbose --ipcPort %i --waitCount 1 --waitTime 5 /3/0/0" % (port,))
        
        # wait for subscribe process to start up
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session IPC configured for UDP: address 127.0.0.1, port %d" % (port,))
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session connected")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Subscribe /3/0/0 Change\n")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Waiting for 1 notifications over 5 seconds:\n")
        
        # do set command
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        
        result = client_set(self.config, "/3/0/0=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        # read subscribe output
        expectedStdout = "Notify 1:\nChanged: /3/0/0 Resource Modified:\nDevice[/3/0]:\n    Manufacturer[/3/0/0]: abc\nSession disconnected\n"
        expectedStderr = ""
        expectedCode = 0
        
        result = tools_common.non_blocking_get_run_result(subscribeProcess)
        
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
       
    def test_subscribe_multiple_resources_multiple_changes(self):
        # test that we can subscribe to a single change of value on 
        # one of the multiple resources specified
        
        # open client subscribe subprocess. Only wait for a single change to one of the resources
        port = self.config.clientIpcPort
        subscribeProcess = tools_common.run_non_blocking(tools_common.CLIENT_SUBSCRIBE, 
                                                    "--verbose --ipcPort %i --waitCount 1 --waitTime 5 /3/0/0 /3/0/1" % (port,))
        # wait for subscribe process to start up
        
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session IPC configured for UDP: address 127.0.0.1, port %d" % (port,))
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session connected")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Subscribe /3/0/0 Change\n")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Subscribe /3/0/1 Change\n")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Waiting for 1 notifications over 5 seconds:\n")
        
        # do set command and expect notifications from the subscribe subprocess
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        
        # set the first resource
        result = client_set(self.config, "/3/0/0=abc /3/0/1=def")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
        expectedStdout = "Notify 1:\nChanged: /3/0/0 Resource Modified:\nDevice[/3/0]:\n    Manufacturer[/3/0/0]: abc\nNotify 2:\nChanged: /3/0/1 Resource Modified:\nDevice[/3/0]:\n    ModelNumber[/3/0/1]: def\nSession disconnected\n"
        expectedStderr = ""
        expectedCode = 0
        
        result = tools_common.non_blocking_get_run_result(subscribeProcess)
        
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
       
    def test_subscribe_single_resource_multiple_changes(self):
        # test that we can subscribe to multiple changes of value on a specified resource
        num_changes = 2;
        # open client subscribe subprocess. Wait for multiple changes to the resource
        port = self.config.clientIpcPort
        subscribeProcess = tools_common.run_non_blocking(tools_common.CLIENT_SUBSCRIBE, 
                                                    "--verbose --ipcPort %i --waitCount %d --waitTime 5 /3/0/0" % (port, num_changes))
        
        time.sleep(0.25)
        # wait for subscribe process to start up
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session IPC configured for UDP: address 127.0.0.1, port %d" % (port,))
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session connected")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Subscribe /3/0/0 Change\n")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Waiting for %d notifications over 5 seconds:\n" % (num_changes,))
        
        # send the first set command
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        
        result = client_set(self.config, "/3/0/0=1")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
        # wait until we have received a notify before sending the second change
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Notify 1:\n")
        result = client_set(self.config, "/3/0/0=2")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
        # read subscribe output
        expectedStdout = "Changed: /3/0/0 Resource Modified:\nDevice[/3/0]:\n    Manufacturer[/3/0/0]: 1\nNotify 2:\nChanged: /3/0/0 Resource Modified:\nDevice[/3/0]:\n    Manufacturer[/3/0/0]: 2\nSession disconnected\n"
        expectedStderr = ""
        expectedCode = 0
        
        result = tools_common.non_blocking_get_run_result(subscribeProcess)
        
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
    def test_subscribe_single_resource_multiple_changes_skip_same_value(self):
        # test that we don't get a notify if we set a value to resource that already has that value
        port = self.config.clientIpcPort
        
        # open client subscribe subprocess. Wait for a change to the resource
        subscribeProcess = tools_common.run_non_blocking(tools_common.CLIENT_SUBSCRIBE, 
                                                    "--verbose --ipcPort %i --waitCount 1 --waitTime 5 /3/0/0" % (port,))
        time.sleep(0.25)
        # wait for subscribe process to start up
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session IPC configured for UDP: address 127.0.0.1, port %d" % (port,))
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session connected")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Subscribe /3/0/0 Change\n")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Waiting for 1 notifications over 5 seconds:\n")
        
        # do set command
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        
        result = client_set(self.config, "/3/0/0=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        # read subscribe output
        expectedStdout = "Notify 1:\nChanged: /3/0/0 Resource Modified:\nDevice[/3/0]:\n    Manufacturer[/3/0/0]: abc\nSession disconnected\n"
        expectedStderr = ""
        expectedCode = 0
        
        result = tools_common.non_blocking_get_run_result(subscribeProcess)
        
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
        # open a new client subscribe subprocess. Wait for a change to the resource
        subscribeProcess = tools_common.run_non_blocking(tools_common.CLIENT_SUBSCRIBE, 
                                                    "--verbose --ipcPort %i --waitCount 1 --waitTime 5 /3/0/0" % (port,))
        
        # wait for subscribe process to start up
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session IPC configured for UDP: address 127.0.0.1, port %d" % (port,))
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session connected")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Subscribe /3/0/0 Change\n")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Waiting for 1 notifications over 5 seconds:\n")
        
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        
        # do set command. This one should NOT cause a notify.
        result = client_set(self.config, "/3/0/0=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
        # do another set command. This one should cause a notify.
        result = client_set(self.config, "/3/0/0=def")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        # read subscribe output
        expectedStdout = "Notify 1:\nChanged: /3/0/0 Resource Modified:\nDevice[/3/0]:\n    Manufacturer[/3/0/0]: def\nSession disconnected\n"
        expectedStderr = ""
        expectedCode = 0
        
        result = tools_common.non_blocking_get_run_result(subscribeProcess)
        
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

if __name__ == "__main__":
	unittest.main()
