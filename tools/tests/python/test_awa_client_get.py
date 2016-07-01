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

# Tests related to basic client get operations

import unittest
import re

import common
import tools_common

from test_awa_client_define import client_define
from test_awa_client_set import client_set

from tools_common import CustomObject
from tools_common import CustomResource

def client_get(config, *args):
    return tools_common.run_client(config, tools_common.CLIENT_GET, *args)

class TestGet(tools_common.AwaTest):

    def test_get_device_object(self):
        expectedStdout = """Device[/3/0]:\n    Reboot[/3/0/4]: [Executable]\n    ErrorCode[/3/0/11/0]: 0\n    SupportedBindingandModes[/3/0/16]: U\n    Manufacturer[/3/0/0]: Imagination Technologies\n    ModelNumber[/3/0/1]: Awa Client\n    SerialNumber[/3/0/2]: SN12345678\n    FirmwareVersion[/3/0/3]: 0.1a\n    FactoryReset[/3/0/5]: [Executable]\n    AvailablePowerSources[/3/0/6/0]: 1\n    AvailablePowerSources[/3/0/6/1]: 5\n    PowerSourceVoltage[/3/0/7/0]: 3800\n    PowerSourceVoltage[/3/0/7/1]: 5000\n    PowerSourceCurrent[/3/0/8/0]: 125\n    PowerSourceCurrent[/3/0/8/1]: 900\n    BatteryLevel[/3/0/9]: 100\n    MemoryFree[/3/0/10]: 15\n    CurrentTime[/3/0/13]: 2718619435\n    UTCOffset[/3/0/14]: +12:00\n    Timezone[/3/0/15]: Pacific/Wellington\n    DeviceType[/3/0/17]: Awa Client\n    HardwareVersion[/3/0/18]: 0.0.0.1\n    SoftwareVersion[/3/0/19]: 0.0.0.11\n    BatteryStatus[/3/0/20]: 2\n    MemoryTotal[/3/0/21]: 42\n"""
        expectedStderr = ""
        expectedCode = 0

        result = client_get(self.config, "/3")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_get_device_object_instance(self):
        # test that a whole object instance can be retrieved
        expectedStdout = """Device[/3/0]:\n    Reboot[/3/0/4]: [Executable]\n    ErrorCode[/3/0/11/0]: 0\n    SupportedBindingandModes[/3/0/16]: U\n    Manufacturer[/3/0/0]: Imagination Technologies\n    ModelNumber[/3/0/1]: Awa Client\n    SerialNumber[/3/0/2]: SN12345678\n    FirmwareVersion[/3/0/3]: 0.1a\n    FactoryReset[/3/0/5]: [Executable]\n    AvailablePowerSources[/3/0/6/0]: 1\n    AvailablePowerSources[/3/0/6/1]: 5\n    PowerSourceVoltage[/3/0/7/0]: 3800\n    PowerSourceVoltage[/3/0/7/1]: 5000\n    PowerSourceCurrent[/3/0/8/0]: 125\n    PowerSourceCurrent[/3/0/8/1]: 900\n    BatteryLevel[/3/0/9]: 100\n    MemoryFree[/3/0/10]: 15\n    CurrentTime[/3/0/13]: 2718619435\n    UTCOffset[/3/0/14]: +12:00\n    Timezone[/3/0/15]: Pacific/Wellington\n    DeviceType[/3/0/17]: Awa Client\n    HardwareVersion[/3/0/18]: 0.0.0.1\n    SoftwareVersion[/3/0/19]: 0.0.0.11\n    BatteryStatus[/3/0/20]: 2\n    MemoryTotal[/3/0/21]: 42\n"""
        expectedStderr = ""
        expectedCode = 0

        result = client_get(self.config, "/3/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_get_resource(self):
        # test standard get prints out the correct output
        expectedStdout = """Device[/3/0]:\n    ModelNumber[/3/0/1]: Awa Client\n"""
        expectedStderr = ""
        expectedCode = 0

        result = client_get(self.config, "/3/0/1")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_get_non_existent_object(self):
        # test that we should return AwaError_PathNotFound on a non-existent object
        expectedStdout = ""
        expectedStderr = "AwaClientGetOperation_Perform failed\nFailed to retrieve /9001: AwaError_PathNotFound\n"
        expectedCode = 1

        result = client_get(self.config, "/9001")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_get_non_existent_object_instance(self):
        # test that we should return AwaError_PathNotFound on a non existent object instance
        expectedStdout = ""
        expectedStderr = "AwaClientGetOperation_Perform failed\nFailed to retrieve /3/1/0: AwaError_PathNotFound\n"
        expectedCode = 1

        result = client_get(self.config, "/3/1/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_get_non_existent_resource(self):
        # test that we should just return an empty response for a non-existent resource
        expectedStdout = ""
        expectedStderr = "AwaError_DefinitionInvalid: resourceDefinition for /1000/0/999 is NULL\nAwaClientGetOperation_Perform failed\nFailed to retrieve /1000/0/999: AwaError_PathNotFound\n"
        expectedCode = 1

        result = client_get(self.config, "/1000/0/999")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    @unittest.skip("TODO fix")
    def test_get_non_existent_resource_instance(self):
        # test that we should just return an empty response for a non-existent resource instance
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_get(self.config, "/1000/0/204/50")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_get_device_multi_instance_resource(self):
        # test that we should be able to retrieve a multi instance resource types
        expectedStdout = """Device[/3/0]:\n    AvailablePowerSources[/3/0/6/0]: 1\n    AvailablePowerSources[/3/0/6/1]: 5\n"""
        expectedStderr = ""
        expectedCode = 0

        result = client_get(self.config, "/3/0/6")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_get_garbage_multi_resource_instance(self):
        # test that we shouldn't be able to retrieve a non-existent multi resource instance resource
        expectedStdout = ""
        expectedStderr = """Target /1000/0/6/#### is not valid\nAwaError_OperationInvalid: No paths specified\nAwaClientGetOperation_Perform failed\n"""
        expectedCode = 1

        result = client_get(self.config, "/1000/0/6/####")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_get_no_preceding_slash(self):
        # test that we shouldn't be able to retrieve an object if the request doesn't start with a slash
        expectedStdout = ""
        expectedStderr = """Target 3 is not valid\nAwaError_OperationInvalid: No paths specified\nAwaClientGetOperation_Perform failed\n"""
        expectedCode = 1

        result = client_get(self.config, "3")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_get_noinput(self):
        # test get in empty format doesn't crash/hang
        expectedStdout = ""
        expectedStderr = "Specify one or more resource paths.\n"
        expectedCode = 1

        result = client_get(self.config, "")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_get_garbage_instance_path(self):
        # test that we cannot
        expectedStdout = ""
        expectedStderr = "Target /3/#####/9 is not valid\nAwaError_OperationInvalid: No paths specified\nAwaClientGetOperation_Perform failed\n"
        expectedCode = 1

        result = client_get(self.config, "/3/#####/9")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_get_garbage_object_path(self):
        # test that we should not able to retrieve an object if the request not contains valid path
        expectedStdout = ""
        expectedStderr = """Target /#####/0/9 is not valid\nAwaError_OperationInvalid: No paths specified\nAwaClientGetOperation_Perform failed\n"""
        expectedCode = 1

        result = client_get(self.config, "/#####/0/9")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_get_invalid_option(self):
        # test get in wrong option format doesn't crash/hang
        expectedStdout = ""
        expectedStderr = "invalid option -- 1\n"
        expectedCode = 1

        result = client_get(self.config, "-1")

        # strip the filename from stderr
        stderr_mod = re.split(r": ", result.stderr, maxsplit=1)[1]

        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, stderr_mod)
        self.assertEqual(expectedCode,   result.code)

    def test_get_garbage(self):
        # test get in non /O/I/R/ format doesn't crash
        expectedStdout = ""
        expectedStderr = "Specify one or more resource paths.\n"
        expectedCode = 1

        result = client_get(self.config, "####")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_get_garbage_2(self):
        # test get in non /O/I/R/ format doesn't crash
        expectedStdout = ""
        expectedStderr = """Target /?!?!%%%%%%% is not valid\nAwaError_OperationInvalid: No paths specified\nAwaClientGetOperation_Perform failed\n"""
        expectedCode = 1

        result = client_get(self.config, "/?!?!%%%%%%%")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_get_quiet(self):
        # test get with the quiet flag prints out the correct output
        expectedStdout = "Awa Client\n"
        expectedStderr = ""
        expectedCode = 0

        result = client_get(self.config, "--quiet /3/0/1")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_get_verbose(self):
        # test get with the verbose flag prints out the correct output
        expectedStdout = """Session IPC configured for UDP: address 127.0.0.1, port %d\nSession connected\nGet /3/0/1\nGet operation completed successfully.\nDevice[/3/0]:\n    ModelNumber[/3/0/1]: Awa Client\nSession disconnected\n""" % (self.config.clientIpcPort,)
        expectedStderr = ""
        expectedCode = 0

        result = client_get(self.config, "-v /3/0/1")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    # testing debug mode is not feasible

    def test_get_executable_resource(self):
        # test that we cannot read from an executable resource
        expectedStdout = "Device[/3/0]:\n    Reboot[/3/0/4]: [Executable]\n"
        expectedStderr = ""
        expectedCode = 0

        # attempt to get an executable resource, should print the resource name
        result = client_get(self.config, "/3/0/4")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_new_object_no_instance(self):
        # test a get on a new object with no instances created

        customObjects = (
            CustomObject("Object1001", 1001, False, "single", (
                CustomResource("Resource100", 100, "string",  "single", "optional", "rw"),
            )),
        )
        params = tools_common.create_define_command(customObjects)
        client_define(self.config, *params)

        expectedStdout = "Object1001[/1001]: No instances\n"
        expectedStderr = ""
        expectedCode = 0
        result = client_get(self.config, "/1001")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_new_object_no_resource(self):
        # test a get on a new object with one instance created, but no optional resource created

        customObjects = (
            CustomObject("Object1001", 1001, False, "single", (
                CustomResource("Resource100", 100, "string",  "single", "optional", "rw"),
            )),
        )
        params = tools_common.create_define_command(customObjects)
        client_define(self.config, *params)

        client_set(self.config, "--create /1001/0")

        expectedStdout = "Object1001[/1001/0]:\n    No resources\n"
        expectedStderr = ""
        expectedCode = 0
        result = client_get(self.config, "/1001/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

class TestGetNoClientDaemon(tools_common.BasicTestCase):

    @unittest.skip("test_get_no_client skipped - variable output")
    def test_get_no_client(self):
        # test that the get tool provides an error if the client isn't running
        expectedStdout = ""
        expectedStderr = """Timed out receiving response on IPC UDP\nAwaClientSession_Connect() failed\nFailed to establish Awa Session\n"""
        expectedCode = 1

        result = client_get(self.config, "/3/0/1")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)


class TestGetBasic(tools_common.BasicTestCase):

    def test_get_help(self):
	expectedStdout = """awa-client-get 1.0

LWM2M Client Resource Get Operation

Usage: awa-client-get [OPTIONS]... [PATHS]...

  -h, --help                Print help and exit
  -V, --version             Print version and exit
  -v, --verbose             Increase program verbosity  (default=off)
  -d, --debug               Increase program verbosity  (default=off)
  -q, --quiet               Decrease program verbosity  (default=off)
  -a, --ipcAddress=ADDRESS  Connect to Client IPC Address
                              (default=`127.0.0.1\')
  -p, --ipcPort=PORT        Connect to Client IPC port  (default=`12345\')

Specify one or more object, object instance and resource paths
in the format "/O/I/R/i", separated by spaces. For example:

    /3/0/7/1 /3/0/0 /4/1 /5

"""
        expectedStderr = ""
        expectedCode = 0
        helpOptions = ("--help","-h")

        for option in helpOptions:
            code, stdout, stderr = client_get(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)

    def test_get_version(self):
        expectedStdout = "awa-client-get 1.0\n"
        expectedStderr = ""
        expectedCode = 0
        versionOptions = ("--version","-V")

        for option in versionOptions:
            code, stdout, stderr = client_get(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)

    def test_get_no_args(self):
        expectedStdout = ""
        expectedStderr = "Specify one or more resource paths.\n"
        expectedCode = 1

        code, stdout, stderr = client_get(self.config)
        self.assertEqual(expectedStdout, stdout)
        self.assertEqual(expectedStderr, stderr)
        self.assertEqual(expectedCode, code)
