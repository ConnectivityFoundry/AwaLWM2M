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

# Tests related to basic server delete operations

import unittest

import common
import tools_common
from tools_common import SERVER_DELETE

def server_delete(config, *args):
    return tools_common.run_server_with_client(config, SERVER_DELETE, *args)

def server_delete_basic(config, *args):
    return tools_common.run_server(config, SERVER_DELETE, *args)

class TestDelete(tools_common.AwaTest):

    def test_invalid_delete_resource(self):
        # test that we can't delete resources - only object instances
        expectedStdout = ""
        expectedStderr = "AwaServerDeleteOperation_Perform failed\nFailed to delete target /3/0/1: AwaLWM2MError_MethodNotAllowed\n"
        expectedCode = 1

        result = server_delete(self.config, "/3/0/1")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_invalid_delete_object(self):
        # test that we can't delete an object without specifying an instance ID
        expectedStdout = ""
        expectedStderr = "AwaServerDeleteOperation_Perform failed\nFailed to delete target /3: AwaLWM2MError_MethodNotAllowed\n"
        expectedCode = 1

        result = server_delete(self.config, "/3")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_delete_non_existing_object_non_existing_instance(self):
         # test that a single resource can be set and retrieved
        expectedStdout = ""
        expectedStderr = "AwaServerDeleteOperation_Perform failed\nFailed to delete target /123/456: AwaLWM2MError_"
        expectedCode = 1

        result = server_delete(self.config, "/123/456")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr[:len(expectedStderr)])
        self.assertEqual(expectedCode,   result.code)

    def test_delete_no_slash(self):
        # test that we cannot delete a non-existing value
        expectedStdout = ""
        expectedStderr = "Target 3 is not valid\nAwaError_OperationInvalid: No paths specified\nAwaServerDeleteOperation_Perform failed\nAwaError_ResponseInvalid: response is NULL\nAwaServerDeleteOperation_GetResponse returned NULL\n"
        expectedCode = 1

        result = server_delete(self.config, "3")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_delete_garbage(self):
        # test that we run delete on a non /O/I/R input without crashing
        expectedStdout = ""
        expectedStderr = "Target hello is not valid\nAwaError_OperationInvalid: No paths specified\nAwaServerDeleteOperation_Perform failed\nAwaError_ResponseInvalid: response is NULL\nAwaServerDeleteOperation_GetResponse returned NULL\n"
        expectedCode = 1

        result = server_delete(self.config, "hello")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_delete_mandatory_object_instance(self):
         # test that we shouldn't be able to delete a mandatory object
        expectedStdout = ""
        expectedStderr = "AwaServerDeleteOperation_Perform failed\nFailed to delete target /3/0: AwaLWM2MError_Unauthorized\n"
        expectedCode = 1

        result = server_delete(self.config, "/3/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)


class TestDeleteBasic(tools_common.BasicTestCase):

    def test_delete_help(self):
        expectedStdout = """awa-server-delete 1.0\n\nLWM2M Server Resource Delete Operation\n\nUsage: awa-server-delete [OPTIONS]... [PATHS]...\n\n  -h, --help                Print help and exit\n  -V, --version             Print version and exit\n  -v, --verbose             Increase program verbosity  (default=off)\n  -d, --debug               Increase program verbosity  (default=off)\n  -a, --ipcAddress=ADDRESS  Connect to Server IPC Address\n                              (default=`127.0.0.1\')\n  -p, --ipcPort=PORT        Connect to Server IPC port  (default=`54321\')\n  -c, --clientID=ID         Client ID\n\nSpecify one or more object instance paths\nto delete in the format "/O/I", separated by spaces.\nFor example:\n\n    /3/0\n    /4/0\n\n"""
        expectedStderr = ""
        expectedCode = 0
        helpOptions = ("--help","-h")

        for option in helpOptions:
            code, stdout, stderr = server_delete_basic(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)


    def test_delete_version(self):
        expectedStdout = "awa-server-delete 1.0\n"
        expectedStderr = ""
        expectedCode = 0
        versionOptions = ("--version","-V")

        for option in versionOptions:
            code, stdout, stderr = server_delete_basic(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)

    def test_delete_no_args(self):
        expectedStdout = ""
        expectedStderr = SERVER_DELETE + ": '--clientID' ('-c') option required\n"
        expectedCode = 1

        code, stdout, stderr = server_delete_basic(self.config)
        self.assertEqual(expectedStdout, stdout)
        self.assertEqual(expectedStderr, stderr)
        self.assertEqual(expectedCode, code)
