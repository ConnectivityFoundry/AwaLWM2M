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

# Tests related to basic server observe operations

import unittest

import common
import tools_common
from tools_common import SERVER_OBSERVE
from test_awa_client_set import client_set
from test_awa_server_write import server_write

def server_observe(config, *args):
    return tools_common.run_server_with_client(config, SERVER_OBSERVE, *args)

def server_observe_basic(config, *args):
    return tools_common.run_server(config, SERVER_OBSERVE, *args)

class TestObserve(tools_common.AwaTest):

    def test_observe_timeout_verbose(self):
        # test that observe will timeout successfully after the specified time (will still receive current values)
        expectedStdout = "Session IPC configured for UDP: address 127.0.0.1, port %d\nSession connected\nObserve /3/0/0\nWaiting for notifications over 1 seconds:\nNotify 0 from clientID TestClient:\nChanged: /3/0/0 Resource Modified:\nDevice[/3/0]:\n    Manufacturer[/3/0/0]: Imagination Technologies\nSession disconnected\n" % (self.config.serverIpcPort,)
        expectedStderr = ""
        expectedCode = 0
        
        result = server_observe(self.config, "--verbose --waitTime 1 /3/0/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def test_observe_invalid_object(self):
        # test that we cannot observe an object that doesn't exist
        expectedStdout = "Session IPC configured for UDP: address 127.0.0.1, port %d\nSession connected\nSession disconnected\n" % (self.config.serverIpcPort,)
        expectedStderr = "Target /asd/zxc is not valid\nAwaError_OperationInvalid: No paths specified\nFailed to perform observe operation\n"
        expectedCode = 0
        
        result = server_observe(self.config, "--verbose --waitTime 1 /asd/zxc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def test_observe_empty_slash(self):
        # test that we cannot observe an object that doesn't exist
        expectedStdout = "Session IPC configured for UDP: address 127.0.0.1, port %d\nSession connected\nSession disconnected\n" % (self.config.serverIpcPort,)
        expectedStderr = "Target / is not valid\nAwaError_OperationInvalid: No paths specified\nFailed to perform observe operation\n"
        expectedCode = 0
        
        result = server_observe(self.config, "--verbose --waitTime 1 /")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_observe_executable_resource(self):
        # test that we should not be able to observe an execute command on the server 
        # since the server is the one which would be executing the command
        expectedStdout = "Session IPC configured for UDP: address 127.0.0.1, port %d\nSession connected\nObserve /3/0/4\nSession disconnected\n" % (self.config.serverIpcPort,)
        expectedStderr = "AwaError_LWM2MError: Failed to observe to path /3/0/4\nFailed to perform observe operation\n"
        expectedCode = 0
        
        result = server_observe(self.config, "--verbose /3/0/4")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def temp_create_resource(self, path, value):
        # because defaults aren't currently supported, do a write to create the resource.
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        result = server_write(self.config, "%s=%s" % (path, value))
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def observe_resource(self, path, currentValuesOutput, value, expectedResponse):
        # test that we can observe to a single change of value on a specified resource
        # open client observe subprocess. Only wait for a single change to the resource
        resourcePath = "/".join(path.split("/")[0:4])
        
        port = self.config.serverIpcPort
        observeProcess = tools_common.run_non_blocking(SERVER_OBSERVE, 
                                                    "--verbose --ipcPort %i --waitCount 1 --waitTime 555 %s --clientID %s" % (port, resourcePath, self.config.clientEndpointName))
        
        # wait for observe process to start up
        self.assertEqual(tools_common.strip_prefix(observeProcess.stdout.readline()), "Session IPC configured for UDP: address 127.0.0.1, port %d" % (port,))
        self.assertEqual(tools_common.strip_prefix(observeProcess.stdout.readline()), "Session connected")
        self.assertEqual(tools_common.strip_prefix(observeProcess.stdout.readline()), "Observe %s\n" % (resourcePath, ))
        self.assertEqual(tools_common.strip_prefix(observeProcess.stdout.readline()), "Waiting for 1 notifications over 555 seconds:\n")
        self.assertEqual(tools_common.strip_prefix(observeProcess.stdout.readline()), "Notify 0 from clientID TestClient:\n")
        self.assertEqual(tools_common.strip_prefix(observeProcess.stdout.readline()), "Changed: %s Resource Modified:\n" % (resourcePath, ))
        
        for line in currentValuesOutput:
            self.assertEqual(tools_common.strip_prefix(observeProcess.stdout.readline()), line)
        
        # do write command
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        
        result = server_write(self.config, "%s=%s" % (path, value))
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        #Notify 1:\n    Manufacturer[3/0/0]: abc\nUnobserve /3/0/0\n
        # read observe output
        expectedStdout = expectedResponse
        expectedStderr = ""
        expectedCode = 0
        
        result = tools_common.non_blocking_get_run_result(observeProcess)
        
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
    
    def test_observe_device_time_resource_single_change(self):
        self.observe_resource("/3/0/13", ["Device[/3/0]:\n", "    CurrentTime[/3/0/13]: 2718619435\n"], "123456789", "Notify 1 from clientID TestClient:\nChanged: /3/0/13 Resource Modified:\nDevice[/3/0]:\n    CurrentTime[/3/0/13]: 123456789\nSession disconnected\n")
    
    def test_observe_string_resource_single_change(self):
        self.temp_create_resource("/1000/0/100", "271861946")
        self.observe_resource("/1000/0/100", ["Object1000[/1000/0]:\n", "    Resource100[/1000/0/100]: 271861946\n"], "abc", "Notify 1 from clientID TestClient:\nChanged: /1000/0/100 Resource Modified:\nObject1000[/1000/0]:\n    Resource100[/1000/0/100]: abc\nSession disconnected\n")
    
    def test_observe_int_resource_single_change(self):
        self.temp_create_resource("/1000/0/101", "54321")
        self.observe_resource("/1000/0/101", ["Object1000[/1000/0]:\n", "    Resource101[/1000/0/101]: 54321\n"], "123", "Notify 1 from clientID TestClient:\nChanged: /1000/0/101 Resource Modified:\nObject1000[/1000/0]:\n    Resource101[/1000/0/101]: 123\nSession disconnected\n")
    
    def test_observe_float_resource_single_change(self):
        self.temp_create_resource("/1000/0/102", "543.12")
        self.observe_resource("/1000/0/102", ["Object1000[/1000/0]:\n", "    Resource102[/1000/0/102]: 543.12\n"], "123.45678901234", "Notify 1 from clientID TestClient:\nChanged: /1000/0/102 Resource Modified:\nObject1000[/1000/0]:\n    Resource102[/1000/0/102]: 123.45678901234\nSession disconnected\n")
    
    def test_observe_boolean_resource_single_change(self):
        self.temp_create_resource("/1000/0/103", "False")
        self.observe_resource("/1000/0/103", ["Object1000[/1000/0]:\n", "    Resource103[/1000/0/103]: False\n"], "True", "Notify 1 from clientID TestClient:\nChanged: /1000/0/103 Resource Modified:\nObject1000[/1000/0]:\n    Resource103[/1000/0/103]: True\nSession disconnected\n")
    
    def test_observe_time_resource_single_change(self):
        self.temp_create_resource("/1000/0/104", "271861946")
        self.observe_resource("/1000/0/104", ["Object1000[/1000/0]:\n", "    Resource104[/1000/0/104]: 271861946\n"], "123456789", "Notify 1 from clientID TestClient:\nChanged: /1000/0/104 Resource Modified:\nObject1000[/1000/0]:\n    Resource104[/1000/0/104]: 123456789\nSession disconnected\n")
    
    def test_observe_opaque_resource_single_change(self):
        self.temp_create_resource("/1000/0/105", "271861946")
        self.observe_resource("/1000/0/105", ["Object1000[/1000/0]:\n", "    Resource105[/1000/0/105]: Opaque (9):32 37 31 38 36 31 39 34 36 \n"], "some_data_here", "Notify 1 from clientID TestClient:\nChanged: /1000/0/105 Resource Modified:\nObject1000[/1000/0]:\n    Resource105[/1000/0/105]: Opaque (14):73 6F 6D 65 5F 64 61 74 61 5F 68 65 72 65 \nSession disconnected\n")
    #TODO object link and arrays"""

class TestObserveBasic(tools_common.BasicTestCase):

    def test_observe_help(self):
        expectedStdout = """awa-server-observe 1.0\n\nLWM2M Server Observe Operation\n\nUsage: awa-server-observe [OPTIONS]... [PATHS]...\n\n  -h, --help                Print help and exit\n  -V, --version             Print version and exit\n  -v, --verbose             Increase program verbosity  (default=off)\n  -d, --debug               Increase program verbosity  (default=off)\n  -q, --quiet               Decrease program verbosity  (default=off)\n  -a, --ipcAddress=ADDRESS  Connect to Server IPC Address\n                              (default=`127.0.0.1\')\n  -p, --ipcPort=PORT        Connect to IPC port  (default=`54321\')\n  -c, --clientID=ID         Client ID\n  -t, --waitTime=SECONDS    Time to wait for notification  (default=`0\')\n  -n, --waitCount=NUMBER    Number of notifications to wait for  (default=`0\')\n\nSpecify one or more object, object instance and resource paths\nto observe in the format "/O/I/R/i", separated by spaces.\nFor example:\n\n    /3/0/0\n    /3/0/7/1\n    /4/0/3 /4/0/6\n\n"""
        expectedStderr = ""
        expectedCode = 0
        helpOptions = ("--help","-h")

        for option in helpOptions:
            code, stdout, stderr = server_observe_basic(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)

    def test_observe_version(self):
        expectedStdout = "awa-server-observe 1.0\n"
        expectedStderr = ""
        expectedCode = 0
        versionOptions = ("--version","-V")

        for option in versionOptions:
            code, stdout, stderr = server_observe_basic(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)

    def test_observe_no_args(self):
        expectedStdout = ""
        expectedStderr = SERVER_OBSERVE + ": '--clientID' ('-c') option required\n"
        expectedCode = 1

        code, stdout, stderr = server_observe_basic(self.config)
        self.assertEqual(expectedStdout, stdout)
        self.assertEqual(expectedStderr, stderr)
        self.assertEqual(expectedCode, code)
