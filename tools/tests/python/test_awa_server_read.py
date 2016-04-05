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

# Tests related to basic server read operations

import unittest

import common
import tools_common
from tools_common import SERVER_READ

def server_read(config, *args):
    return tools_common.run_server_with_client(config, SERVER_READ, *args)

def server_read_basic(config, *args):
    return tools_common.run_server(config, SERVER_READ, *args)

class TestRead(tools_common.AwaTest):
    
    def test_read(self):
        # test that a single resource can be retrieved
        expectedStdout = "Device[/3/0]:\n    ErrorCode[/3/0/11/0]: 0\n    SupportedBindingandModes[/3/0/16]: U\n    Manufacturer[/3/0/0]: Imagination Technologies\n    ModelNumber[/3/0/1]: Awa Client\n    SerialNumber[/3/0/2]: SN12345678\n    FirmwareVersion[/3/0/3]: 0.1a\n    AvailablePowerSources[/3/0/6/0]: 1\n    AvailablePowerSources[/3/0/6/1]: 5\n    PowerSourceVoltage[/3/0/7/0]: 3800\n    PowerSourceVoltage[/3/0/7/1]: 5000\n    PowerSourceCurrent[/3/0/8/0]: 125\n    PowerSourceCurrent[/3/0/8/1]: 900\n    BatteryLevel[/3/0/9]: 100\n    MemoryFree[/3/0/10]: 15\n    CurrentTime[/3/0/13]: 2718619435\n    UTCOffset[/3/0/14]: +12:00\n    Timezone[/3/0/15]: Pacific/Wellington\n    DeviceType[/3/0/17]: Awa Client\n    HardwareVersion[/3/0/18]: 0.0.0.1\n    SoftwareVersion[/3/0/19]: 0.0.0.11\n    BatteryStatus[/3/0/20]: 2\n    MemoryTotal[/3/0/21]: 42\n"
        expectedStderr = ""
        expectedCode = 0

        result = server_read(self.config, "/3")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_read_non_existent_object(self):
        # test that we shouldn't be able to retrieve a non-existent object
        expectedStdout = ""
        expectedStderr = "AwaServerReadOperation_Perform failed\nFailed to read from path /9001: AwaLWM2MError_NotFound\n"
        expectedCode = 1
        
        result = server_read(self.config, "/9001")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def test_read_non_existent_instance(self):
        # test that we shouldn't be able to retrieve a non-existent object instance
        expectedStdout = ""
        expectedStderr = "AwaServerReadOperation_Perform failed\nFailed to read from path /3/1: AwaLWM2MError_NotFound\n"
        expectedCode = 1
        
        result = server_read(self.config, "/3/1")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def test_read_non_existent_resource(self):
        # test that we shouldn't be able to retrieve a non-existent resource
        expectedStdout = ""
        expectedStderr = "AwaError_DefinitionInvalid: resourceDefinition for /3/0/999 is NULL\nAwaServerReadOperation_Perform failed\nFailed to read from path /3/0/999: AwaLWM2MError_NotFound\n"
        expectedCode = 1
        
        result = server_read(self.config, "/3/0/999")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    
    @unittest.skip("Low priority - Applies to tool only, not an issue with API")
    def test_read_non_existent_resource_instance(self):
        # test that we shouldn't be able to retrieve a non-existent resource instance
        expectedStdout = ""
        expectedStderr = "The requested resource instance does not exist: /3/0/7/5\n"
        expectedCode = 1

        result = server_read(self.config, "/3/0/7/5")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def test_read_no_preceding_slash(self):
        # test that we shouldn't be able to retrieve an object if the request doesn't start with a slash
        expectedStdout = ""
        expectedStderr = "Target 3 is not valid\nAwaError_OperationInvalid: No paths specified\nAwaServerReadOperation_Perform failed\nAwaError_ResponseInvalid: response is NULL\nresponse is NULL\nAwaError_IteratorInvalid: iterator is NULL\n"
        expectedCode = 1
        
        result = server_read(self.config, "3")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_read_garbage(self):
        # test read in non /O/I/R/ format doesn't crash
        expectedStdout = ""
        expectedStderr = "Specify one or more resource paths.\n"
        expectedCode = 1
        
        result = server_read(self.config, "####")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def test_read_garbage_2(self):
        # test read in non /O/I/R/ format doesn't crash
        expectedStdout = ""
        expectedStderr = "Target /?!?!%%%%%%% is not valid\nAwaError_OperationInvalid: No paths specified\nAwaServerReadOperation_Perform failed\nAwaError_ResponseInvalid: response is NULL\nresponse is NULL\nAwaError_IteratorInvalid: iterator is NULL\n"
        expectedCode = 1
        
        result = server_read(self.config, "/?!?!%%%%%%%")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def test_read_loud(self):
        # test standard read prints out the correct output
        expectedStdout = "Device[/3/0]:\n    ModelNumber[/3/0/1]: Awa Client\n"
        expectedStderr = ""
        expectedCode = 0
        
        result = server_read(self.config, "/3/0/1")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def test_read_quiet(self):
        # test read with the quiet flag prints out the correct output
        expectedStdout = "Awa Client\n"
        expectedStderr = ""
        expectedCode = 0
        
        result = server_read(self.config, "-q /3/0/1")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_read_verbose(self):
        # test read with the verbose flag prints out the correct output
        expectedStdout = "Session IPC configured for UDP: address 127.0.0.1, port %d\nSession connected\nRead TestClient /3/0/1\nDevice[/3/0]:\n    ModelNumber[/3/0/1]: Awa Client\nSession disconnected\n" % (self.config.serverIpcPort,)
        expectedStderr = ""
        expectedCode = 0
        
        result = server_read(self.config, "-v /3/0/1")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    @unittest.skip("test_read_debug Unmaintainable")
    def test_read_debug(self):
        # test read with the debug flag prints out the correct output
        expectedStdout = "Read /3/0/1\n    ModelNumber[3/0/1]: Lightweight M2M Client\n"
        expectedStderr = ""
        expectedCode = 0
        
        result = server_read(self.config, "-d /3/0/1")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_read_executable_resource(self):
        # test that we cannot read from an executable resource
        expectedStdout = "Session IPC configured for UDP: address 127.0.0.1, port %d\nSession connected\nRead TestClient /3/0/4\nSession disconnected\n" % (self.config.serverIpcPort)
        expectedStderr = "AwaServerReadOperation_Perform failed\nFailed to read from path /3/0/4: AwaLWM2MError_BadRequest\n"
        expectedCode = 1
        
        # attempt to Read an executable resource, should print the resource name
        result = server_read(self.config, "/3/0/4 -v")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)


class TestReadBasic(tools_common.BasicTestCase):

    def test_read_help(self):
        expectedStdout = """awa-server-read 1.0\n\nLWM2M Server Resource Read Operation\n\nUsage: awa-server-read [OPTIONS]... [PATHS]...\n\n  -h, --help                Print help and exit\n  -V, --version             Print version and exit\n  -v, --verbose             Increase program verbosity  (default=off)\n  -d, --debug               Increase program verbosity  (default=off)\n  -q, --quiet               Decrease program verbosity  (default=off)\n  -a, --ipcAddress=ADDRESS  Connect to Server IPC Address\n                              (default=`127.0.0.1\')\n  -p, --ipcPort=PORT        Connect to Server IPC port  (default=`54321\')\n  -c, --clientID=ID         Client ID\n\nSpecify one or more object, object instance and resource paths\nin the format "/O/I/R/i", separated by spaces. For example:\n\n    /3/0/7/1 /3/0/0 /4/1 /5\n\n"""
        expectedStderr = ""
        expectedCode = 0
        helpOptions = ("--help","-h")

        for option in helpOptions:
            code, stdout, stderr = server_read_basic(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)


    def test_read_version(self):
        expectedStdout = "awa-server-read 1.0\n"
        expectedStderr = ""
        expectedCode = 0
        versionOptions = ("--version","-V")

        for option in versionOptions:
            code, stdout, stderr = server_read_basic(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)

    def test_read_no_args(self):
        expectedStdout = ""
        expectedStderr = SERVER_READ + ": '--clientID' ('-c') option required\n"
        expectedCode = 1

        code, stdout, stderr = server_read_basic(self.config)
        self.assertEqual(expectedStdout, stdout)
        self.assertEqual(expectedStderr, stderr)
        self.assertEqual(expectedCode, code)
