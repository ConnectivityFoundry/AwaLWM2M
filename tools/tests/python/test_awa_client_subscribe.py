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

# Tests related to basic client subscribe operations

import unittest

import common
import tools_common
from test_awa_client_set import client_set
from test_awa_server_execute import server_execute
from test_awa_server_execute import server_execute_stdin

def client_subscribe(config, *args):
    return tools_common.run_client(config, tools_common.CLIENT_SUBSCRIBE, *args)


class TestSubscribe(tools_common.AwaTest):
    
    def subscribe_resource(self, path, value, expectedResponse):
        resourcePath = "/".join(path.split("/")[0:4])
        # test that we can subscribe to a single change of value on a specified resource
        # open client subscribe subprocess. Only wait for a single change to the resource
        port = self.config.clientIpcPort
        subscribeProcess = tools_common.run_non_blocking(tools_common.CLIENT_SUBSCRIBE, 
                                                    "--verbose --ipcPort %i --waitCount 1 --waitTime 5 %s" % (port, resourcePath,))
        
        # wait for subscribe process to start up
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session IPC configured for UDP: address 127.0.0.1, port %d" % (port,))
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session connected")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Subscribe %s Change\n" % (resourcePath, ))
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Waiting for 1 notifications over 5 seconds:\n")
        
        # do set command
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        
        result = client_set(self.config, "%s=%s" % (path, value))
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        #Notify 1:\n    Manufacturer[3/0/0]: abc\nUnsubscribe /3/0/0\n
        # read subscribe output
        expectedStdout = expectedResponse
        expectedStderr = ""
        expectedCode = 0
        
        result = tools_common.non_blocking_get_run_result(subscribeProcess)
        
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
    
    def test_subscribe_string_resource_single_change(self):
        self.subscribe_resource("/1000/0/100", "abc", "Notify 1:\nChanged: /1000/0/100 Resource Modified:\nObject1000[/1000/0]:\n    Resource100[/1000/0/100]: abc\nSession disconnected\n")
    def test_subscribe_int_resource_single_change(self):
        self.subscribe_resource("/1000/0/101", "123", "Notify 1:\nChanged: /1000/0/101 Resource Modified:\nObject1000[/1000/0]:\n    Resource101[/1000/0/101]: 123\nSession disconnected\n")
    def test_subscribe_float_resource_single_change(self):
        self.subscribe_resource("/1000/0/102", "123.456", "Notify 1:\nChanged: /1000/0/102 Resource Modified:\nObject1000[/1000/0]:\n    Resource102[/1000/0/102]: 123.456\nSession disconnected\n")
    def test_subscribe_boolean_resource_single_change(self):
        self.subscribe_resource("/1000/0/103", "True", "Notify 1:\nChanged: /1000/0/103 Resource Modified:\nObject1000[/1000/0]:\n    Resource103[/1000/0/103]: True\nSession disconnected\n")
    def test_subscribe_time_resource_single_change(self):
        self.subscribe_resource("/1000/0/104", "123456789", "Notify 1:\nChanged: /1000/0/104 Resource Modified:\nObject1000[/1000/0]:\n    Resource104[/1000/0/104]: 123456789\nSession disconnected\n")
    def test_subscribe_opaque_resource_single_change(self):
        self.subscribe_resource("/1000/0/105", "some_data_here", "Notify 1:\nChanged: /1000/0/105 Resource Modified:\nObject1000[/1000/0]:\n    Resource105[/1000/0/105]: Opaque (14):73 6F 6D 65 5F 64 61 74 61 5F 68 65 72 65 \nSession disconnected\n")
    
    @unittest.skip("OBJLINK UNSUPPORTED")
    def test_subscribe_objectlink_resource_single_change(self):
        self.subscribe_resource("/1000/0/106", "/3/7", "Notify 1:\nChanged: /1000/0/106 Resource Modified:\nObject1000[/1000/0]:\n    Resource106[/1000/0/106]: ObjectLink[3:7]\nSession disconnected\n")

    @unittest.skip("ARRAYS UNSUPPORTED")
    def test_subscribe_string_array_resource_single_change(self):
        self.subscribe_resource("/1000/0/200/3", "abc", "Notify 1:\nChanged: /1000/0/200 Resource Modified:\nObject1000[/1000/0]:\n    Resource200[/1000/0/200/3]: abc\nSession disconnected\n")
    @unittest.skip("ARRAYS UNSUPPORTED")
    def test_subscribe_int_array_resource_single_change(self):
        self.subscribe_resource("/1000/0/201/2", "123", "Notify 1:\nChanged: /1000/0/201 Resource Modified:\nObject1000[/1000/0]:\n    Resource201[/1000/0/201/2]: 123\nSession disconnected\n")
    @unittest.skip("ARRAYS UNSUPPORTED")
    def test_subscribe_float_array_resource_single_change(self):
        self.subscribe_resource("/1000/0/202/4", "1.234", "Notify 1:\nChanged: /1000/0/202 Resource Modified:\nObject1000[/1000/0]:\n    Resource202[/1000/0/202/4]: 1.234\nSession disconnected\n")
    @unittest.skip("ARRAYS UNSUPPORTED")
    def test_subscribe_boolean_array_resource_single_change(self):
        self.subscribe_resource("/1000/0/203/5", "True", "Notify 1:\nChanged: /1000/0/203 Resource Modified:\nObject1000[/1000/0]:\n    Resource203[/1000/0/203/5]: True\nSession disconnected\n")
    @unittest.skip("ARRAYS UNSUPPORTED")
    def test_subscribe_time_array_resource_single_change(self):
        self.subscribe_resource("/1000/0/204/6", "987654321", "Notify 1:\nChanged: /1000/0/204 Resource Modified:\nObject1000[/1000/0]:\n    Resource204[/1000/0/204/6]: 987654321\nSession disconnected\n")
    @unittest.skip("ARRAYS UNSUPPORTED")
    def test_subscribe_opaque_array_resource_single_change(self):
        self.subscribe_resource("/1000/0/205/7", "some_array_opaque_data", "Notify 1:\nChanged: /1000/0/205 Resource Modified:\nObject1000[/1000/0]:\n    Resource205[/1000/0/205/7]: some_array_opaque_data\nSession disconnected\n")
    @unittest.skip("ARRAYS UNSUPPORTED")
    def test_subscribe_objlink_array_resource_single_change(self):
        self.subscribe_resource("/1000/0/206/8", "/5/0", "Notify 1:\nChanged: /1000/0/206 Resource Modified:\nObject1000[/1000/0]:\n    Resource206[/1000/0/206/8]: ObjectLink[5:0]\nSession disconnected\n")
 
    def test_subscribe_to_execute(self):
        port = self.config.clientIpcPort
        subscribeProcess = tools_common.run_non_blocking(tools_common.CLIENT_SUBSCRIBE, 
                                                    "--verbose --ipcPort %i --waitCount 1 --waitTime 5 /3/0/4" % (port,))
        
        # wait for subscribe process to start up
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session IPC configured for UDP: address 127.0.0.1, port %d" % (port,))
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session connected")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Subscribe /3/0/4 Execute\n")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Waiting for 1 notifications over 5 seconds:\n")
        
        # do execute command
        expectedStdout = "Target /3/0/4 executed successfully\n"
        expectedStderr = ""
        expectedCode = 0
        
        result = server_execute(self.config, "/3/0/4")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        # read subscribe output
        expectedStdout = "Execute 1:\nNO DATA\nSession disconnected\n"
        expectedStderr = ""
        expectedCode = 0
        
        result = tools_common.non_blocking_get_run_result(subscribeProcess)
        
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
        
    def test_subscribe_to_execute_with_payload(self):
        port = self.config.clientIpcPort
        subscribeProcess = tools_common.run_non_blocking(tools_common.CLIENT_SUBSCRIBE, 
                                                    "--verbose --ipcPort %i --waitCount 1 --waitTime 5 /3/0/4" % (port,))
        
        # wait for subscribe process to start up
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session IPC configured for UDP: address 127.0.0.1, port %d" % (port,))
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Session connected")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Subscribe /3/0/4 Execute\n")
        self.assertEqual(tools_common.strip_prefix(subscribeProcess.stdout.readline()), "Waiting for 1 notifications over 5 seconds:\n")
        
        # do execute command
        expectedStdout = "Target /3/0/4 executed successfully\n"
        expectedStderr = ""
        expectedCode = 0
        
        inputText = "QmFzZTY0IGlzIGEgZ2VuZXJpYyB0ZXJtIGZvciB"
        result = server_execute_stdin(self.config, inputText, "/3/0/4")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        # read subscribe output
        expectedStdout = "Execute 1:\nDATA: length 39, payload: [51 6d 46 7a 5a 54 59 30 49 47 6c 7a 49 47 45 67 5a 32 56 75 5a 58 4a 70 59 79 42 30 5a 58 4a 74 49 47 5a 76 63 69 42 ]\nSession disconnected\n"
        expectedStderr = ""
        expectedCode = 0
        
        result = tools_common.non_blocking_get_run_result(subscribeProcess)
        
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
                

    def test_subscribe_timeout_verbose(self):
        # test that subscribe will timeout successfully after the specified time
        expectedStdout = "Session IPC configured for UDP: address 127.0.0.1, port %d\nSession connected\nSubscribe /3/0/0 Change\nWaiting for notifications over 1 seconds:\nSession disconnected\n" % self.config.clientIpcPort
        expectedStderr = ""
        expectedCode = 0
        
        result = client_subscribe(self.config, "--verbose --waitTime 1 /3/0/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def test_subscribe_invalid_object(self):
        # test that we cannot subscribe to an object that doesn't exist
        expectedStdout = "Session IPC configured for UDP: address 127.0.0.1, port %d\nSession connected\nSession disconnected\n" % self.config.clientIpcPort
        expectedStderr = "Target /asd/zxc is not valid\nAwaError_OperationInvalid: No paths specified\nFailed to perform subscribe operation\n"
        expectedCode = 0
        
        result = client_subscribe(self.config, "--verbose --waitTime 1 /asd/zxc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def test_subscribe_empty_slash(self):
        # test that we cannot subscribe to an object that doesn't exist
        expectedStdout = "Session IPC configured for UDP: address 127.0.0.1, port %d\nSession connected\nSession disconnected\n" % self.config.clientIpcPort
        expectedStderr = "Target / is not valid\nAwaError_OperationInvalid: No paths specified\nFailed to perform subscribe operation\n"
        expectedCode = 0
        
        result = client_subscribe(self.config, "--verbose --waitTime 1 /")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)


class TestSubscribeBasic(tools_common.BasicTestCase):

    def test_subscribe_help(self):
        expectedStdout = """awa-client-subscribe 1.0\n\nLWM2M Client Resource Subscribe Operation\n\nUsage: awa-client-subscribe [OPTIONS]... [PATHS]...\n\n  -h, --help                Print help and exit\n  -V, --version             Print version and exit\n  -v, --verbose             Increase program verbosity  (default=off)\n  -d, --debug               Increase program verbosity  (default=off)\n  -q, --quiet               Decrease program verbosity  (default=off)\n  -a, --ipcAddress=ADDRESS  Connect to Client IPC Address\n                              (default=`127.0.0.1\')\n  -p, --ipcPort=PORT        Connect to IPC port  (default=`12345\')\n  -t, --waitTime=SECONDS    Time to wait for notification  (default=`0\')\n  -c, --waitCount=NUMBER    Number of notifications to wait for  (default=`0\')\n\nSpecify one or more object, object instance and resource paths\nto subscribe to in the format "/O/I/R/i", separated by spaces.\nFor example:\n\n    /3/0/0\n    /3/0/7/1\n    /4/0/3 /4/0/6\n\n"""
        expectedStderr = ""
        expectedCode = 0
        helpOptions = ("--help","-h")

        for option in helpOptions:
            code, stdout, stderr = client_subscribe(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)


    def test_subscribe_version(self):
        expectedStdout = "awa-client-subscribe 1.0\n"
        expectedStderr = ""
        expectedCode = 0
        versionOptions = ("--version","-V")

        for option in versionOptions:
            code, stdout, stderr = client_subscribe(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)

    def test_subscribe_no_args(self):
        expectedStdout = ""
        expectedStderr = "Specify one or more resource paths.\n"
        #expectedStderr = ""
        expectedCode = 1

        code, stdout, stderr = client_subscribe(self.config)
        self.assertEqual(expectedStdout, stdout)
        self.assertEqual(expectedStderr, stderr)
        self.assertEqual(expectedCode, code)
