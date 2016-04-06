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

# Tests related to server-side operations only

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
from test_awa_server_define import server_define
from test_awa_server_delete import server_delete
from test_awa_server_execute import server_execute
from test_awa_server_execute import server_execute_stdin
from test_awa_server_list_clients import server_list_clients
from test_awa_server_observe import server_observe
from test_awa_server_read import server_read
from test_awa_server_write import server_write

class TestServer(tools_common.AwaTest):

    def test_read_write(self):
        # test that a single resource can be set and retrieved
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/3/0/15=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        expectedStdout = "Device[/3/0]:\n    Timezone[/3/0/15]: abc\n"
        expectedStderr = ""
        expectedCode = 0

        result = server_read(self.config, "/3/0/15")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_read_write_single_resource_string(self):
        # test that a single string resource can be set and retrieved
        manufacturer = "ACME Corp."
        expectedStdout = "Object1000[/1000/0]:\n    Resource100[/1000/0/100]: %s\n" % (manufacturer,)
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/100=\"%s\"" % (manufacturer,))
        self.assertEqual(expectedCode, result.code)

        result = server_read(self.config, "/1000/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_read_write_single_resource_integer(self):
        # test that a single integer resource can be set and retrieved
        value = 3
        expectedStdout = "Object1000[/1000/0]:\n    Resource101[/1000/0/101]: %d\n" % (value,)
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "--verbose /1000/0/101=%d" % (value,))
        self.assertEqual(expectedCode, result.code)

        result = server_read(self.config, "/1000/0/101")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_read_write_single_resource_float(self):
        # test that a single float resource can be set and retrieved
        value = 3.5
        expectedStdout = "Object1000[/1000/0]:\n    Resource102[/1000/0/102]: %.1f\n" % (value,)
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/102=%f" % (value,))
        self.assertEqual(expectedCode, result.code)

        result = server_read(self.config, "/1000/0/102")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_read_write_single_resource_boolean_true(self):
        # test that a single boolean resource can be set and retrieved
        value = True

        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/103=%r" % (value,))

        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        expectedStdout = "Object1000[/1000/0]:\n    Resource103[/1000/0/103]: %r\n" % (value,)

        result = server_read(self.config, "/1000/0/103")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_read_write_single_resource_boolean_false(self):
        # test that a single boolean resource can be set and retrieved
        value = False

        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/103=%r" % (value,))

        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        expectedStdout = "Object1000[/1000/0]:\n    Resource103[/1000/0/103]: %r\n" % (value,)

        result = server_read(self.config, "/1000/0/103")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_read_write_single_resource_boolean_numbers(self):
        # test correct values for setting boolean resource with integers: 0,1,2 (False,True,True)
        for value in range(3):
            expectedStdout = ""
            expectedStderr = ""
            expectedCode = 0

            result = server_write(self.config, "/1000/0/103=%d" % (value,))

            self.assertEqual(expectedStdout, result.stdout)
            self.assertEqual(expectedStderr, result.stderr)
            self.assertEqual(expectedCode, result.code)

            expectedStdout = "Object1000[/1000/0]:\n    Resource103[/1000/0/103]: %r\n" % (bool(value),)
            #print "Boolean expectedStdout: %s" % (expectedStdout,)
            result = server_read(self.config, "/1000/0/103")
            self.assertEqual(expectedStdout, result.stdout)
            self.assertEqual(expectedStderr, result.stderr)
            self.assertEqual(expectedCode, result.code)

    def test_read_write_single_resource_time(self):
        # test that a single time resource (64 bit integer) can be set and retrieved
        value = 1442972971
        expectedStdout = "Object1000[/1000/0]:\n    Resource104[/1000/0/104]: %d\n" % (value,)
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/104=%d" % (value,))
        self.assertEqual(expectedCode, result.code)

        result = server_read(self.config, "/1000/0/104")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_read_write_single_resource_objlink(self):
        # test that a single object link resource can be set and retrieved
        link = "/3/0"
        expectedStdout = "Object1000[/1000/0]:\n    Resource106[/1000/0/106]: ObjectLink[%s]\n" % (link.strip('/').replace('/', ':'))
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/106=\"%s\"" % (link,))
        self.assertEqual(expectedCode, result.code)

        result = server_read(self.config, "/1000/0/106")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_write_single_resource_none(self):
        # test that a single none resource (executable) cannot be set
        value = 12345
        expectedStdout = ""
        expectedStderr = "Resource /1000/0/107 is of type None and cannot be set\n"
        expectedCode = 1

        result = server_write(self.config, "/1000/0/107=%d" % (value,))
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedCode, result.code)

    def test_read_single_resource_none(self):
        # test that a single none resource (executable) cannot be retrieved
        expectedStdout = ""
        expectedStderr = "AwaServerReadOperation_Perform failed\nFailed to read from path /1000/0/107: AwaLWM2MError_BadRequest\n"
        expectedCode = 1

        result = server_read(self.config, "/1000/0/107")
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedCode, result.code)

    def test_read_write_multiple_resources_same_instance(self):
        # test that multiple resources from the same instance can be set and retrieved with a single command
        timezone = "ACME Corp."
        currentTime = 123456789

        result = server_write(self.config,
                     "/3/0/15=\"%s\"" % (timezone,),
                     "/3/0/13=%d" % (currentTime,))
        
        self.assertEqual("", result.stderr)
        self.assertEqual("", result.stdout)
        self.assertEqual(0, result.code)

        expectedStdout = "Device[/3/0]:\n    Timezone[/3/0/15]: %s\n"% (timezone, )
        expectedStderr = ""
        expectedCode = 0
        #result = server_read(self.config, "/3/0/15", "/3/0/13")
        result = server_read(self.config, "/3/0/15")
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedCode, result.code)
        
        expectedStdout = "Device[/3/0]:\n    CurrentTime[/3/0/13]: %s\n"% (currentTime, )
        
        result = server_read(self.config, "/3/0/13")
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedCode, result.code)

    @unittest.skip("Write/Read on different object instances in a single request is not supported")
    def test_read_write_multiple_resources_different_instances(self):
        # test that multiple resources from different instances can be set and retrieved with a single command
        timezone = "ACME Corp."
        modelNumber = "1234567890"
        expectedStdout = \
"""    Timezone[3/0/15]: %s
    Resource100[1000/0/100]: %s
""" % (timezone, modelNumber)
        expectedStderr = ""
        expectedCode = 0

        server_write(self.config, "/3/0/15=\"%s\"" % (timezone,), "/1000/0/100=\"%s\"" % (modelNumber,))

        result = server_read(self.config, "/3/0/15", "/1000/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_read_write_invalid_value(self):
        # first set an initial value for an integer resource.
        # Then set the integer resource with a string value. Currently will set the value to 0

        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/101=12345")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        expectedStdout = "Object1000[/1000/0]:\n    Resource101[/1000/0/101]: 12345\n"
        expectedStderr = ""
        expectedCode = 0

        result = server_read(self.config, "/1000/0/101")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/101=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        expectedStdout = "Object1000[/1000/0]:\n    Resource101[/1000/0/101]: 0\n"
        expectedStderr = ""
        expectedCode = 0

        result = server_read(self.config, "/1000/0/101")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_read_write_delete_read(self):
        # test that we can no longer read the value of a resource after it has been deleted
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/100=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        expectedStdout = "Object1000[/1000/0]:\n    Resource100[/1000/0/100]: abc\n"
        expectedStderr = ""
        expectedCode = 0

        result = server_read(self.config, "/1000/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_delete(self.config, "/1000/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        expectedStdout = ""
        expectedStderr = "AwaServerReadOperation_Perform failed\nFailed to read from path /1000/0/100: AwaLWM2MError_NotFound\n"
        expectedCode = 1

        result = server_read(self.config, "/1000/0/100")
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedCode, result.code)

    def test_set_multiple_instances_on_single_instance_object(self):
        #test that we can't create multiple object instances on a single instance object
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/100=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        expectedStdout = ""
        expectedStderr = "AwaServerWriteOperation_Perform failed\nFailed to write to path /1000/1/100: AwaLWM2MError_MethodNotAllowed\n"
        expectedCode = 1

        result = server_write(self.config, "/1000/1/100=abc")
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedCode, result.code)

    def test_set_multiple_instances_on_single_instance_resource(self):
        #test that we can't set multiple resource instances on a single instance resource
        expectedStdout = ""
        expectedStderr = "Error: resource /1000/0/100 is not an array; do not specify a resource instance ID\n"
        expectedCode = 1

        result = server_write(self.config, "/1000/0/100/0=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_read_write_only_resource(self):
        # test that we can't read from a write only resource
        customObjects = (
            CustomObject("Object1001", 1001, False, "single", (
                    CustomResource("Resource100", 100, "string",  "single", "mandatory", "w"),
            )),
        )

        #define and set the write only object
        params = tools_common.create_define_command(customObjects)
        result = client_define(self.config, *params)
        self.assertEqual(0, result.code)
        result = server_define(self.config, *params)
        self.assertEqual(0, result.code)

        result = server_write(self.config, "--create /1001/0")
        self.assertEqual("", result.stderr)
        self.assertEqual("", result.stdout)
        self.assertEqual(0, result.code)

        result = server_write(self.config, "/1001/0/100=abc")
        self.assertEqual("", result.stderr)
        self.assertEqual("", result.stdout)
        self.assertEqual(0, result.code)
        
        expectedStdout = ""
        expectedStderr = "AwaServerReadOperation_Perform failed\nFailed to read from path /1001/0/100: AwaLWM2MError_MethodNotAllowed\n"
        expectedCode = 1

        # attempt to read a write only object, should fail
        result = server_read(self.config, "/1001/0/100")
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedCode, result.code)

    def test_write_read_only_resource(self):
        # test that we can't write to a read only resource
        customObjects = (
            CustomObject("Object1001", 1001, False, "single", (
                    CustomResource("Resource100", 100, "string",  "single", "optional", "r"),
            )),
        )
        params = tools_common.create_define_command(customObjects)
        result = client_define(self.config, *params)
        self.assertEqual(0, result.code)
        result = server_define(self.config, *params)
        self.assertEqual(0, result.code)

        # FlowDeviceMgmtServer_Write failed : (-11) The requested operation is not permitted\n
        expectedStdout = ""
        expectedStderr = "AwaServerWriteOperation_Perform failed\nFailed to write to path /1001/0/100: AwaLWM2MError_MethodNotAllowed\n"
        expectedCode = 1

        # set a read only object
        result = server_write(self.config, "/1001/0/100=abc")
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedCode, result.code)

    def test_delete_custom_object_instance(self):
        # test we can delete a custom object instance
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/100=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_delete(self.config, "/1000/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_server_read_deleted_instance(self):
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/100=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        result = server_delete(self.config, "/1000/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        expectedStdout = ""
        expectedStderr = "AwaServerReadOperation_Perform failed\nFailed to read from path /1000/0/100: AwaLWM2MError_NotFound\n"
        expectedCode = 1

        result = server_read(self.config, "/1000/0/100")
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedCode,   result.code)

    @unittest.skip("Cannot execute multiple resources in a single command")
    def test_execute_multiple_resources(self):
        # test we can execute multiple executable resources with one command
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        #first create a custom object instance by setting a writable resource inside it
        result = server_write(self.config, "/1000/0/100=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        #run both executable resources
        result = server_execute(self.config, "/3/0/4 /1000/0/107")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
