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

# Tests related to basic server write operations

import unittest
import time

import common
import tools_common
import test_awa_client_define
import test_awa_server_define
from tools_common import SERVER_WRITE

def server_write(config, *args):
    return tools_common.run_server_with_client(config, SERVER_WRITE, *args)

def server_write_basic(config, *args):
    return tools_common.run_server(config, SERVER_WRITE, *args)
 

class TestWrite(tools_common.AwaTest):

    def test_write(self):
        # test that on a successful write we receive no output and the return code is zero
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/3/0/15=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def test_write_multiple(self):
        # test that on a successful write we receive no output and the return code is zero
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/3/0/15=abc /3/0/14=def")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_write_no_instance(self):
        # test that we cannot write a value to an object type
        expectedStdout = ""
        expectedStderr = "Resource or Resource Instance must be specified: /9001\n"#AwaError_OperationInvalid: No paths specified\nAwaServerWriteOperation_Perform failed\nAwaError_ResponseInvalid: response is NULL\n"
        expectedCode = 1
        
        result = server_write(self.config, "/9001")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_write_no_resource(self):
        # test that we cannot write a value to an object type
        expectedStdout = ""
        expectedStderr = "Resource or Resource Instance must be specified: /9001/2\n"#AwaError_OperationInvalid: No paths specified\nAwaServerWriteOperation_Perform failed\nAwaError_ResponseInvalid: response is NULL\n"
        expectedCode = 1
        
        result = server_write(self.config, "/9001/2")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def test_write_no_value(self):
        # test that we cannot set a resource without specifying a value
        expectedStdout = ""
        expectedStderr = "A value must be specified: /9001/2/3\n"#AwaError_OperationInvalid: No paths specified\nAwaServerWriteOperation_Perform failed\nAwaError_ResponseInvalid: response is NULL\n"
        expectedCode = 1
        
        result = server_write(self.config, "/9001/2/3")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def test_write_no_value_2(self):
        # test that we cannot set a resource without specifying a value
        expectedStdout = ""
        expectedStderr = "A value must be specified: /9001/2/3/4\n"#AwaError_OperationInvalid: No paths specified\nAwaServerWriteOperation_Perform failed\nAwaError_ResponseInvalid: response is NULL\n"
        expectedCode = 1
        
        result = server_write(self.config, "/9001/2/3/4")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_write_non_existent_resource(self):
        # test that we cannot set a value of a resource from a non-existent resource or object type
        expectedStdout = ""
        expectedStderr = "/9001/2/3 is not defined\n"#AwaError_OperationInvalid: No paths specified\nAwaServerWriteOperation_Perform failed\nAwaError_ResponseInvalid: response is NULL\n"
        expectedCode = 1
        
        result = server_write(self.config, "/9001/2/3/4=5")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_write_empty_value(self):
        # test that we must specify a value when doing a set
        expectedStdout = ""
        expectedStderr = "A value must be specified: /3/0/10=\n"#AwaError_OperationInvalid: No paths specified\nAwaServerWriteOperation_Perform failed\nAwaError_ResponseInvalid: response is NULL\n"
        expectedCode = 1
        
        result = server_write(self.config, "/3/0/10=")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_write_garbage_value(self):
        # test we can set MemoryFree (int) as a garbage value without crashing
        expectedStdout = ""
        expectedStderr = "AwaServerWriteOperation_Perform failed\nFailed to write to path /3/0/10: AwaLWM2MError_MethodNotAllowed\n"
        expectedCode = 1
        
        result = server_write(self.config, "/3/0/10=?!?!%%%%%%%")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_write_verbose(self):
        # test that set with the verbose flag prints correct output
        expectedStdout = "Session IPC configured for UDP: address 127.0.0.1, port %d\nSession connected\nSet String /3/0/15 <- abc\nWrite operation completed successfully.\nSession disconnected\n" % (self.config.serverIpcPort, )
        expectedStderr = ""
        expectedCode = 0
        
        result = server_write(self.config, "-v /3/0/15=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
       
    def test_write_no_slash(self):
        # test that we cannot set a resource without a preceding slash
        expectedStdout = ""
        expectedStderr = "Target 3/0/1=123 is not valid\n"#AwaError_OperationInvalid: No paths specified\nAwaServerWriteOperation_Perform failed\nAwaError_ResponseInvalid: response is NULL\n"
        expectedCode = 1
        
        result = server_write(self.config, "3/0/1=123")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_write_executable_only_resource(self):
        # test that we cannot write to an executable resource
        expectedStdout = ""
        expectedStderr = "Resource /3/0/4 is of type None and cannot be set\n"#AwaError_OperationInvalid: No paths specified\nAwaServerWriteOperation_Perform failed\nAwaError_ResponseInvalid: response is NULL\n"
        expectedCode = 1
        
        # attempt to get an executable resource, should print the resource name
        result = server_write(self.config, "/3/0/4=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
    def test_create_object_instance_with_id(self):
        # test that the set tool can create an object instance without specifying an ID
        #expectedStdout = "Create /1000/0"
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        
        customObjects = (
            tools_common.CustomObject("Object1001", 1001, False, "single", (
                    tools_common.CustomResource("Resource100", 100, "string",  "single", "mandatory", "rw"),
            )),
        )
        params = tools_common.create_define_command(customObjects)
        result = test_awa_client_define.client_define(self.config, *params)
        self.assertEqual(0, result.code)
        result = test_awa_server_define.server_define(self.config, *params)
        self.assertEqual(0, result.code)

        result = server_write(self.config, "--create /1001/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        # TODO: check that instance 0 is present
    
    def test_create_multiple_object_instances_for_single_instance_object(self):
        # test that the set tool cannot create an object instance with a non-zero ID on a single-instance
        
        customObjects = (
            tools_common.CustomObject("Object1001", 1001, False, "single", (
                    tools_common.CustomResource("Resource100", 100, "string",  "single", "mandatory", "rw"),
            )),
        )
        params = tools_common.create_define_command(customObjects)
        result = test_awa_client_define.client_define(self.config, *params)
        self.assertEqual(0, result.code)
        result = test_awa_server_define.server_define(self.config, *params)
        self.assertEqual(0, result.code)
        
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "--create /1001/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        expectedStdout = ""
        expectedStderr = "AwaServerWriteOperation_Perform failed\nFailed to write to path /1001/1: AwaLWM2MError_MethodNotAllowed\n"
        expectedCode = 1

        result = server_write(self.config, "--create /1001/1")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def test_create_object_instance_with_non_zero_id(self):
        # test that the set tool can create an object instance with a specified ID
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        
        customObjects = (
            tools_common.CustomObject("Object1001", 1001, False, "single", (
                    tools_common.CustomResource("Resource100", 100, "string",  "single", "mandatory", "rw"),
            )),
        )
        params = tools_common.create_define_command(customObjects)
        result = test_awa_client_define.client_define(self.config, *params)
        self.assertEqual(0, result.code)
        result = test_awa_server_define.server_define(self.config, *params)
        self.assertEqual(0, result.code)

        result = server_write(self.config, "--create /1001/1")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        # TODO: check that instance 1 is present

    def test_create_object_instance_without_id(self):
        # test that the set tool can create an object instance without specifying an ID
        #expectedStdout = "Create /1000/0"
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        
        customObjects = (
            tools_common.CustomObject("Object1001", 1001, False, "single", (
                    tools_common.CustomResource("Resource100", 100, "string",  "single", "mandatory", "rw"),
            )),
        )
        params = tools_common.create_define_command(customObjects)
        result = test_awa_client_define.client_define(self.config, *params)
        self.assertEqual(0, result.code)
        result = test_awa_server_define.server_define(self.config, *params)
        self.assertEqual(0, result.code)

        result = server_write(self.config, "--create /1001")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)


class TestWriteBasic(tools_common.BasicTestCase):

    def test_write_help(self):
        expectedStdout = """awa-server-write 1.0

LWM2M Server Resource Write Operation

Usage: awa-server-write [OPTIONS]... [PATHS]...

  -h, --help                Print help and exit
  -V, --version             Print version and exit
  -v, --verbose             Increase program verbosity  (default=off)
  -d, --debug               Increase program verbosity  (default=off)
  -a, --ipcAddress=ADDRESS  Connect to Server IPC Address
                              (default=`127.0.0.1')
  -p, --ipcPort=PORT        Connect to Server IPC port  (default=`54321')
  -c, --clientID=ID         Client ID
  -o, --create=STRING       Create a new Object Instance or Optional Resource

Specify one or more object, object instance and resource paths
and values in the format "/O/I/R/i=VALUE", separated by spaces.
For example:

    /3/0/0=Imagination
    /3/0/7/1=4200
    /4/0/3=3 /4/0/6=7

"""
        expectedStderr = ""
        expectedCode = 0
        helpOptions = ("--help","-h")

        for option in helpOptions:
            code, stdout, stderr = server_write_basic(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)


    def test_write_version(self):
        expectedStdout = "awa-server-write 1.0\n"
        expectedStderr = ""
        expectedCode = 0
        versionOptions = ("--version","-V")

        for option in versionOptions:
            code, stdout, stderr = server_write_basic(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)

    def test_write_no_args(self):
        expectedStdout = ""
        expectedStderr = SERVER_WRITE + ": '--clientID' ('-c') option required\n"
        expectedCode = 1

        code, stdout, stderr = server_write_basic(self.config)
        self.assertEqual(expectedStdout, stdout)
        self.assertEqual(expectedStderr, stderr)
        self.assertEqual(expectedCode, code)
