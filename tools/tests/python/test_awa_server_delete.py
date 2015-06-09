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
        expectedStderr = "AwaServerDeleteOperation_Perform failed\nFailed to delete target /123/456: AwaLWM2MError_MethodNotAllowed\n"
        expectedCode = 1
        
        result = server_delete(self.config, "/123/456")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
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
        expectedStdout = """awa-server-delete 1.0\n\nLWM2M Server Resource Delete Operation\n\nUsage: awa-server-delete [OPTIONS]... [PATHS]...\n\n  -h, --help                Print help and exit\n  -V, --version             Print version and exit\n  -v, --verbose             Increase program verbosity  (default=off)\n  -d, --debug               Increase program verbosity  (default=off)\n  -a, --ipcAddress=ADDRESS  Connect to Server IPC Address\n                              (default=`127.0.0.1\')\n  -p, --ipcPort=PORT        Connect to Server IPC port  (default=`54321\')\n  -c, --clientID=ID         Client ID\n\nSpecify one or more object, object instance and resource paths\nto delete in the format "/O/I/R/i", separated by spaces.\nFor example:\n\n    /3/0/0\n    /3/0/7/1\n    /4/0/3 /4/0/6\n\n"""
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
