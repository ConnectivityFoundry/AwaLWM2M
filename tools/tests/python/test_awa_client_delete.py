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

# Tests related to basic client delete operations

import unittest

import common
import tools_common

def client_delete(config, *args):
    return tools_common.run_client(config, tools_common.CLIENT_DELETE, *args)

class TestDelete(tools_common.AwaTest):

    def test_delete_existing_object(self):
        # test we can delete an existing object successfully
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        
        result = client_delete(self.config, "/3/0/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_delete_non_existing_object_non_existing_instance(self):
        # test we can't delete a non existing object instance
        expectedStdout = ""
        expectedStderr = "AwaError_NotDefined: /123/456/789 is not defined\nAwaError_DefinitionInvalid: resourceDefinition for /123/456/789 is NULL\nAwaClientDeleteOperation_Perform failed\nFailed to delete target /123/456/789: AwaError_PathNotFound\n"
        expectedCode = 1
        
        result = client_delete(self.config, "/123/456/789")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
   
    def test_delete_already_deleted_object(self):
        # test that 
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        
        result = client_delete(self.config, "/3/0/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
        expectedStdout = ""
        expectedStderr = "AwaClientDeleteOperation_Perform failed\nFailed to delete target /3/0/0: AwaError_"
        expectedCode = 1
        
        result = client_delete(self.config, "/3/0/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr[:len(expectedStderr)])
        self.assertEqual(expectedCode,   result.code)
        
    def test_delete_no_slash(self):
        # test deleting in non /O/I/R/ format doesn't crash
        expectedStdout = ""
        expectedStderr = "Target 3 is not valid\nAwaError_OperationInvalid: No paths specified\nAwaClientDeleteOperation_Perform failed\nAwaClientDeleteOperation_GetResponse returned NULL\n"
        expectedCode = 1
        
        result = client_delete(self.config, "3")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_delete_garbage(self):
        # test deleting in non /O/I/R/ format doesn't crash
        expectedStdout = ""
        expectedStderr = "Target /hello is not valid\nAwaError_OperationInvalid: No paths specified\nAwaClientDeleteOperation_Perform failed\nAwaClientDeleteOperation_GetResponse returned NULL\n"
        expectedCode = 1
        
        result = client_delete(self.config, "/hello")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_delete_garbage1(self):
        # test deleting in non /O/I/R/ format doesn't crash
        expectedStdout = ""
        expectedStderr = "Target /@@@@/####/%%% is not valid\nAwaError_OperationInvalid: No paths specified\nAwaClientDeleteOperation_Perform failed\nAwaClientDeleteOperation_GetResponse returned NULL\n"
        expectedCode = 1

        result = client_delete(self.config, "/@@@@/####/%%%")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_delete_garbage2(self):
        # test deleting in non /O/I/R/ format doesn't crash
        expectedStdout = ""
        expectedStderr = "Target /3/@@@/1 is not valid\nAwaError_OperationInvalid: No paths specified\nAwaClientDeleteOperation_Perform failed\nAwaClientDeleteOperation_GetResponse returned NULL\n"
        expectedCode = 1 

        result = client_delete(self.config, "/3/@@@/1")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_delete_garbage3(self):
        # test deleting in non /O/I/R/ format doesn't crash
        expectedStdout = ""
        expectedStderr = "Target /-3/0/1 is not valid\nAwaError_OperationInvalid: No paths specified\nAwaClientDeleteOperation_Perform failed\nAwaClientDeleteOperation_GetResponse returned NULL\n"
        expectedCode = 1

        result = client_delete(self.config, "/-3/0/1")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_delete_root(self):
        expectedStdout = ""
        expectedStderr = "Target / is not valid\nAwaError_OperationInvalid: No paths specified\nAwaClientDeleteOperation_Perform failed\nAwaClientDeleteOperation_GetResponse returned NULL\n"
        expectedCode = 1

        result = client_delete(self.config, "/")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)  
        
    def test_delete_resource(self):
        # test that we can delete a resource - client can do anything
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_delete(self.config, "/3/0/1")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)  
        
    def test_delete_optional_object(self):
        # test that we can delete an object - client can do anything
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_delete(self.config, "/2")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def test_delete_mandatory_object(self):
         # test that we delete a mandatory object - clients can do anything
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        
        result = client_delete(self.config, "/3")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_delete_execute_resource(self):
        # test that we can delete a resource which have only execute permissions
        expectedStdout = ""
        expectedStderr = "AwaClientDeleteOperation_Perform failed\nFailed to delete target /3/0/12: AwaError_PathNotFound\n"
        expectedCode = 1

        result = client_delete(self.config, "/3/0/12")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_delete_rw_permission_resource(self):
        # test that we can delete a resource which have RW permissions
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_delete(self.config, "/3/0/13")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)


class TestDeleteBasic(tools_common.BasicTestCase):

    def test_delete_help(self):
        expectedStdout = """awa-client-delete 1.0\n\nLWM2M Client Resource Delete Operation\n\nUsage: awa-client-delete [OPTIONS]... [PATHS]...\n\n  -h, --help                Print help and exit\n  -V, --version             Print version and exit\n  -v, --verbose             Increase program verbosity  (default=off)\n  -d, --debug               Increase program verbosity  (default=off)\n  -p, --ipcPort=PORT        Connect to Client IPC port  (default=`12345\')\n  -a, --ipcAddress=ADDRESS  Connect to Client IPC Address\n                              (default=`127.0.0.1\')\n\nSpecify one or more object, object instance and resource paths\nto delete in the format "/O/I/R/i", separated by spaces.\nFor example:\n\n    /3/0/0\n    /3/0/7/1\n    /4/0/3 /4/0/6\n\n"""
        expectedStderr = ""
        expectedCode = 0
        helpOptions = ("--help","-h")

        for option in helpOptions:
            code, stdout, stderr = client_delete(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)


    def test_delete_version(self):
        expectedStdout = "awa-client-delete 1.0\n"
        expectedStderr = ""
        expectedCode = 0
        versionOptions = ("--version","-V")

        for option in versionOptions:
            code, stdout, stderr = client_delete(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)

    def test_delete_no_args(self):
        expectedStdout = "Specify one or more resource paths.\n"
        expectedStderr = ""
        expectedCode = 1

        code, stdout, stderr = client_delete(self.config)
        self.assertEqual(expectedStdout, stdout)
        self.assertEqual(expectedStderr, stderr)
        self.assertEqual(expectedCode, code)
        
