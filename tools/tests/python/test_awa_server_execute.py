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

# Tests related to basic server execute operations

import unittest

import common
import tools_common
from tools_common import SERVER_EXECUTE

def server_execute_stdin(config, inputText, *args):
    return tools_common.run_server_with_client_stdin(config, SERVER_EXECUTE, inputText, *args)

def server_execute_basic(config, *args):
    return tools_common.run_server(config, SERVER_EXECUTE, *args)

def server_execute(config, *args):
    return tools_common.run_server_with_client(config, SERVER_EXECUTE, *args)

class TestExecute(tools_common.AwaTest):

    def test_execute_resource(self):
        # test we can execute an executable resource
        expectedStdout = "Target /3/0/4 executed successfully\n"
        expectedStderr = ""
        expectedCode = 0

        result = server_execute(self.config, "/3/0/4")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)   
    
    def test_execute_resource_verbose(self):
        # test we can execute an executable resource with verbose output
        expectedStdout = "Session IPC configured for UDP: address 127.0.0.1, port %d\nSession connected\nNo payload specified.\nExecute /3/0/4\nTarget /3/0/4 executed successfully\nSession disconnected\n" % (self.config.serverIpcPort)
        expectedStderr = ""
        expectedCode = 0

        result = server_execute(self.config, "--verbose /3/0/4")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
      
    def test_execute_non_executable_resource(self):
        # test we cannot execute an non-executable resource
        expectedStdout = ""
        expectedStderr = "AwaServerExecuteOperation_Perform failed\nFailed to execute target /3/0/1: AwaLWM2MError_BadRequest\n"
        expectedCode = 1

        result = server_execute(self.config, "/3/0/1")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_execute_object(self):
        # test we cannot execute a whole object
        expectedStdout = ""
        expectedStderr = "AwaError_PathInvalid: /3 is not a valid path to an executable resource\nAwaServerGetOperation_AddPath failed\nAwaError_OperationInvalid: No paths specified\nAwaServerExecuteOperation_Perform failed\nAwaError_ResponseInvalid: response is NULL\nAwaServerExecuteOperation_GetResponse returned NULL\n"
        expectedCode = 1

        result = server_execute(self.config, "/3")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_execute_object_instance(self):
        # test we cannot execute a whole object instance
        expectedStdout = ""
        expectedStderr = "AwaError_PathInvalid: /3/0 is not a valid path to an executable resource\nAwaServerGetOperation_AddPath failed\nAwaError_OperationInvalid: No paths specified\nAwaServerExecuteOperation_Perform failed\nAwaError_ResponseInvalid: response is NULL\nAwaServerExecuteOperation_GetResponse returned NULL\n"
        expectedCode = 1

        result = server_execute(self.config, "/3/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_execute_base64_stdin(self):
        # test we can execute a resource, specifying a payload of data
        # example:
        # echo "Hello, World!" | ./awa-server-execute -c client1 /3/0/4 --stdin
        inputText = "QmFzZTY0IGlzIGEgZ2VuZXJpYyB0ZXJtIGZvciB"
        expectedStdout = "Session IPC configured for UDP: address 127.0.0.1, port %d\nSession connected\nRead payload from stdin: QmFzZTY0IGlzIGEgZ2VuZXJpYyB0ZXJtIGZvciB [39 bytes]\nExecute /3/0/4\nTarget /3/0/4 executed successfully\nSession disconnected\n" % (self.config.serverIpcPort)
        expectedStderr = ""
        expectedCode = 0

        result = server_execute_stdin(self.config, inputText, "--verbose /3/0/4")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    
class TestExecuteBasic(tools_common.BasicTestCase):

    def test_execute_help(self):
        expectedStdout = """awa-server-execute 1.0\n\nLWM2M Server Resource Execute Operation\n\nUsage: awa-server-execute [OPTIONS]... [PATHS]...\n\n  -h, --help                Print help and exit\n  -V, --version             Print version and exit\n  -v, --verbose             Increase program verbosity  (default=off)\n  -d, --debug               Increase program verbosity  (default=off)\n  -a, --ipcAddress=ADDRESS  Connect to Server IPC Address\n                              (default=`127.0.0.1\')\n  -p, --ipcPort=PORT        Connect to Server IPC port  (default=`54321\')\n  -c, --clientID=ID         Client ID\n  -s, --stdin               Accept argument data from stdin  (default=off)\n\nSpecify one or more object, object instance and resource paths\nto execute in the format "/O/I/R", separated by spaces.\nFor example:\n\n    /3/0/4\n    /3/0/4 /3/0/5\n\n"""
        expectedStderr = ""
        expectedCode = 0
        helpOptions = ("--help","-h")

        for option in helpOptions:
            code, stdout, stderr = server_execute_basic(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)

    def test_execute_version(self):
        expectedStdout = "awa-server-execute 1.0\n"
        expectedStderr = ""
        expectedCode = 0
        versionOptions = ("--version","-V")

        for option in versionOptions:
            code, stdout, stderr = server_execute_basic(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)

    def test_execute_no_args(self):
        expectedStdout = ""
        expectedStderr = SERVER_EXECUTE + ": '--clientID' ('-c') option required\n"
        expectedCode = 1

        code, stdout, stderr = server_execute_basic(self.config)
        self.assertEqual(expectedStdout, stdout)
        self.assertEqual(expectedStderr, stderr)
        self.assertEqual(expectedCode, code)
