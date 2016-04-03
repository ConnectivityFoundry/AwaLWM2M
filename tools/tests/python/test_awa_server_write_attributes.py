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

import common
import tools_common
from tools_common import SERVER_WRITE_ATTRIBUTES

def server_write(config, *args):
    return tools_common.run_server_with_client(config, SERVER_WRITE_ATTRIBUTES, *args)

def server_write_basic(config, *args):
    return tools_common.run_server(config, SERVER_WRITE_ATTRIBUTES, *args)
 

class TestWriteAttributes(tools_common.AwaTest):

    def test_write_attributes_single(self):
        # test that on a successful write we receive no output and the return code is zero
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/3/0/15?pmin=10")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def test_write_attributes_multiple(self):
        # test that on a successful write we receive no output and the return code is zero
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/3/0/15?pmin=10\&pmax=20")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    @unittest.skip("Unsupported")
    def test_write_attributes_multiple_instances(self):
        # test that on a successful write we receive no output and the return code is zero
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/3/0/15=abc?pmin=10\&pmax=20 /3/0/14?pmin=5\&pmax=10")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_write_attributes_no_instance(self):
        # test that we cannot write a value to an object type
        expectedStdout = ""
        expectedStderr = "AwaError_NotDefined: /9001 is not defined\nAwaError_OperationInvalid: No paths specified\nAwaServerWriteAttributesOperation_Perform failed\nAwaError_ResponseInvalid: response is NULL\n"
        expectedCode = 1
        
        result = server_write(self.config, "/9001?pmin=10")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_write_attributes_no_resource(self):
        # test that we cannot write a value to an object type
        expectedStdout = ""
        expectedStderr = "AwaError_NotDefined: /9001/2 is not defined\nAwaError_OperationInvalid: No paths specified\nAwaServerWriteAttributesOperation_Perform failed\nAwaError_ResponseInvalid: response is NULL\n"
        expectedCode = 1
        
        result = server_write(self.config, "/9001/2?pmin=10")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def test_write_attributes_resource_instance(self):
        # test that we cannot set a resource without specifying a value
        expectedStdout = ""
        expectedStderr = "Target /3/0/7/1?pmin=10 is not valid\nAwaError_OperationInvalid: No paths specified\nAwaServerWriteAttributesOperation_Perform failed\nAwaError_ResponseInvalid: response is NULL\n"
        expectedCode = 1
        
        result = server_write(self.config, "/3/0/7/1?pmin=10")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def test_write_attributes_non_existent_resource(self):
        # test that we cannot set a value of a resource from a non-existent resource or object type
        expectedStdout = ""
        expectedStderr = "AwaError_NotDefined: /9001/2/3 is not defined\nAwaError_OperationInvalid: No paths specified\nAwaServerWriteAttributesOperation_Perform failed\nAwaError_ResponseInvalid: response is NULL\n"
        expectedCode = 1
        
        result = server_write(self.config, "/9001/2/3?pmin=5")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_write_attributes_empty_value(self):
        # test that we must specify a value when doing a set
        expectedStdout = ""
        expectedStderr = "Target /3/0/10 is not valid\nAwaError_OperationInvalid: No paths specified\nAwaServerWriteAttributesOperation_Perform failed\nAwaError_ResponseInvalid: response is NULL\n"
        expectedCode = 1
        
        result = server_write(self.config, "/3/0/10")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_write_attributes_garbage_value(self):
        # test we can set MemoryFree (int) as a garbage value without crashing
        expectedStdout = ""
        expectedStderr = "Failed to parse value of query link: /3/0/10/x\nFailed to parse value of query link: /3/0/10/x\nAwaError_OperationInvalid: No paths specified\nAwaServerWriteAttributesOperation_Perform failed\nAwaError_ResponseInvalid: response is NULL\n"
        expectedCode = 1
        
        result = server_write(self.config, "/3/0/10=?!?!%%%%%%%")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_write_attributes_verbose(self):
        # test that set with the verbose flag prints correct output
        expectedStdout = "Session IPC configured for UDP: address 127.0.0.1, port %d\nSession connected\nWrite Attribute to /3/0/15 as Integer: pmin = 10\nWrite Attributes operation completed successfully.\nSession disconnected\n" % (self.config.serverIpcPort, )
        expectedStderr = ""
        expectedCode = 0
        
        result = server_write(self.config, "-v /3/0/15?pmin=10")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
       
    def test_write_attributes_no_slash(self):
        # test that we cannot set a resource without a preceding slash
        expectedStdout = ""
        expectedStderr = "Target 3/0/1?pmin=123 is not valid\nAwaError_OperationInvalid: No paths specified\nAwaServerWriteAttributesOperation_Perform failed\nAwaError_ResponseInvalid: response is NULL\n"
        expectedCode = 1
        
        result = server_write(self.config, "3/0/1?pmin=123")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_write_attributes_executable_only_resource(self):
        # test that we cannot write to an executable resource
        expectedStdout = ""
        expectedStderr = "AwaServerWriteAttributesOperation_Perform failed\nFailed to write to path /3/0/4: AwaLWM2MError_BadRequest\n"
        expectedCode = 1
        
        # attempt to get an executable resource, should print the resource name
        result = server_write(self.config, "/3/0/4?pmin=123")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)


class TestWriteAttributesBasic(tools_common.BasicTestCase):

    def test_write_attributes_help(self):
        expectedStdout = """awa-server-write-attributes 1.0\n\nLWM2M Server Resource Write Attributes Operation\n\nUsage: awa-server-write-attributes [OPTIONS]... [PATHS]...\n\n  -h, --help                Print help and exit\n  -V, --version             Print version and exit\n  -v, --verbose             Increase program verbosity  (default=off)\n  -d, --debug               Increase program verbosity  (default=off)\n  -a, --ipcAddress=ADDRESS  Connect to Server IPC Address\n                              (default=`127.0.0.1\')\n  -p, --ipcPort=PORT        Connect to Server IPC port  (default=`54321\')\n  -c, --clientID=ID         Client ID\n\nSpecify one or more object, object instance and resource paths\nwith query parameters in the format "/O/I/R?LINK=VALUE\\&LINK2=VALUE2",\nseparated by spaces.\nFor example:\n\n    /3/0/13?gt=5\n    /3/0/13?pmin=2\\&pmax=10\n\n"""
        expectedStderr = ""
        expectedCode = 0
        helpOptions = ("--help","-h")

        for option in helpOptions:
            code, stdout, stderr = server_write_basic(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)


    def test_write_attributes_version(self):
        expectedStdout = "awa-server-write-attributes 1.0\n"
        expectedStderr = ""
        expectedCode = 0
        versionOptions = ("--version","-V")

        for option in versionOptions:
            code, stdout, stderr = server_write_basic(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)

    def test_write_attributes_no_args(self):
        expectedStdout = ""
        expectedStderr = SERVER_WRITE_ATTRIBUTES + ": '--clientID' ('-c') option required\n"
        expectedCode = 1

        code, stdout, stderr = server_write_basic(self.config)
        self.assertEqual(expectedStdout, stdout)
        self.assertEqual(expectedStderr, stderr)
        self.assertEqual(expectedCode, code)
