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

# Tests related to basic client set operations

import unittest

import common
import tools_common
import test_awa_client_define

def client_set(config, *args):
    return tools_common.run_client(config, tools_common.CLIENT_SET, *args)

class TestSet(tools_common.AwaTest):

    def test_set(self):
        # test that on a successful set we receive no output and the return code is zero
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/100=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_opaque(self):
        # test that on a successful set we receive no output and the return code is zero
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/105=xyz")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_resource_instance_on_single_valued_resource(self):
        # test that we cannot set set on single valued resource
        expectedStdout = ""
        expectedStderr = "Error: resource /1000/0/100 is not an array; do not specify a resource instance ID\n"
        expectedCode = 0

        result = client_set(self.config, "/1000/0/100/5=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_resource_instance_on_multi_valued_resource(self):
        # test that on a successful set we receive no output and the return code is zero
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/201/5=1234")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_resource_as_single_valued_on_multi_valued_resource(self):
        # test that cannot set on multivalued resource
        expectedStdout = ""
        expectedStderr = "Error: resource is an array; specify a resource instance ID: /3/0/6/x\n"
        expectedCode = 0

        result = client_set(self.config, "/3/0/6=4321")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_value_on_execute_resource(self):
        # test that on a successful set we receive no output and the return code is zero
        expectedStdout = ""
        expectedStderr = "Resource /1000/0/107 is of type None and cannot be set\n"
        expectedCode = 0

        result = client_set(self.config, "/1000/0/107=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_value_on_non_existing_resource(self):
        # test that on a successful set we receive no output and the return code is zero
        expectedStdout = ""
        expectedStderr = "/3/0/144 is not defined\n"
        expectedCode = 0

        result = client_set(self.config, "/3/0/144=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_value_on_non_existing_instance(self):
        # test that cannot set on existing instance
        expectedStdout = ""
        expectedStderr = "AwaClientSetOperation_Perform failed\nFailed to set on path /3/1000/0: AwaError_PathInvalid\n"
        expectedCode = 1

        result = client_set(self.config, "/3/1000/0=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_no_instance(self):
        # test that we cannot set a value to an object type
        expectedStdout = ""
        expectedStderr = "Resource or Resource Instance must be specified: /9001\n"
        expectedCode = 0

        result = client_set(self.config, "/9001")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_no_resource(self):
        # test that we cannot set a value to an instance
        expectedStdout = ""
        expectedStderr = "Resource or Resource Instance must be specified: /9001/2\n"
        expectedCode = 0

        result = client_set(self.config, "/9001/2")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_no_value(self):
        # test that we cannot set a resource without specifying a value
        expectedStdout = ""
        expectedStderr = "A value must be specified: /9001/2/3\n"
        expectedCode = 0

        result = client_set(self.config, "/9001/2/3")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_no_value_2(self):
        # test that we cannot set a resource without specifying a value
        expectedStdout = ""
        expectedStderr = "A value must be specified: /9001/2/3/4\n"
        expectedCode = 0

        result = client_set(self.config, "/9001/2/3/4")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_non_existent_resource(self):
        # test that we cannot set a value of a resource from a non-existent resource or object type
        expectedStdout = ""
        expectedStderr = "/9001/2/3 is not defined\n"
        expectedCode = 0

        result = client_set(self.config, "/9001/2/3=5")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_empty_value(self):
        # test that we must specify a value when doing a set
        expectedStdout = ""
        expectedStderr = "A value must be specified: /3/0/10=\n"
        expectedCode = 0

        result = client_set(self.config, "/3/0/10=")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_garbage_value(self):
        # test we can set MemoryFree (int) as a garbage value without crashing
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/3/0/10=?!?!%%%%%%%")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_verbose(self):
        # test that set returns correct output with the verbose flag
        expectedStdout="Session IPC configured for UDP: address 127.0.0.1, port %d\nSession connected\nSet Integer /3/0/10 <- abc\nSet operation completed successfully.\nSession disconnected\n" % self.config.clientIpcPort
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "-v /3/0/10=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_no_preceding_slash(self):
        # test that we cannot set a resource without a preceding slash
        expectedStdout = ""
        expectedStderr = "Target 3/0/1=123 is not valid\n"
        expectedCode = 0

        result = client_set(self.config, "3/0/1=123")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_executable_only_resource(self):
        # test that we can set the name of an executable resource
        expectedStdout = ""
        expectedStderr = "Resource /3/0/4 is of type None and cannot be set\n"
        expectedCode = 0

        result = client_set(self.config, "/3/0/4=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_set_read_only_resource(self):
        # test that we can set the name of an read-only resource
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/100=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_set_string_multi_resource(self):
        # test that we can set the name of an multi valued resource
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/4/0/4/2=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)


    def test_set_no_invalid_path1(self):
        # test that we cannot set an invalid path
        expectedStdout = ""
        expectedStderr = "Target INVALID is not valid\n"
        expectedCode = 0

        result = client_set(self.config, "INVALID")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_no_invalid_path2(self):
        # test that we cannot set an invalid path
        expectedStdout = ""
        expectedStderr = "Target /a/b/c is not valid\n"
        expectedCode = 0

        result = client_set(self.config, "/a/b/c")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_no_invalid_path3(self):
        # test that we cannot set an invalid path
        expectedStdout = ""
        expectedStderr = "Target / is not valid\n"
        expectedCode = 0

        result = client_set(self.config, "/")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_no_invalid_path4(self):
        # test that we cannot set an invalid path
        expectedStdout = ""
        expectedStderr = "Target /3/0/6/5/2=234 is not valid\n"
        expectedCode = 0

        result = client_set(self.config, "/3/0/6/5/2=234")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_integer(self):
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/100=100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_string(self):
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/100=new_value")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_float(self):
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/102=91.99")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_bool(self):
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/103=false")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_bool_numeric(self):
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/103=1")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_time(self):
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/104=124541")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_objectlink(self):
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/106=/1000/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_opaque(self):
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/105=helloworld")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_integer_array(self):
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/201/1=100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_string_array(self):
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/200/4=new_value")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_float_array(self):
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/202/3=91.99")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_bool_array(self):
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/203/2=false")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_time_array(self):
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/204/8=881231")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_objectlink_array(self):
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/206/1=/1000/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_objectlink_array_invalid_input(self):
        expectedStdout = ""
        expectedStderr = "AwaClientSetOperation_AddArrayValueAsObjectLink failed\n"
        expectedCode = 0

        result = client_set(self.config, "/1000/0/206/1=1000:0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_set_opaque_array(self):
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "/1000/0/205/5=helloworld")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_create_object_instance_with_id(self):
        # test that the set tool can create an object instance without specifying an ID
        #expectedStdout = "Create /1000/0"
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        customObjects = (
            tools_common.CustomObject("Object1001", 1001, False, "single", (
                    tools_common.CustomResource("Resource100", 100, "string",  "single", "optional", "rw"),
            )),
        )
        params = tools_common.create_define_command(customObjects)
        result = test_awa_client_define.client_define(self.config, *params)
        self.assertEqual(0, result.code)

        result = client_set(self.config, "--create /1001/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        # TODO: check that instance 0 is present

    def test_create_multiple_object_instances_for_single_instance_object(self):
        # test that the set tool cannot create an object instance with a non-zero ID on a single-instance

        customObjects = (
            tools_common.CustomObject("Object1001", 1001, False, "single", (
                    tools_common.CustomResource("Resource100", 100, "string",  "single", "optional", "rw"),
            )),
        )
        params = tools_common.create_define_command(customObjects)
        result = test_awa_client_define.client_define(self.config, *params)
        self.assertEqual(0, result.code)

        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "--create /1001/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        expectedStdout = ""
        expectedStderr = "AwaClientSetOperation_Perform failed\nFailed to set on path /1001/1: AwaError_CannotCreate\n"
        expectedCode = 1

        result = client_set(self.config, "--create /1001/1")
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
                    tools_common.CustomResource("Resource100", 100, "string",  "single", "optional", "rw"),
            )),
        )
        params = tools_common.create_define_command(customObjects)
        result = test_awa_client_define.client_define(self.config, *params)
        self.assertEqual(0, result.code)

        result = client_set(self.config, "--create /1001/1")
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
                    tools_common.CustomResource("Resource100", 100, "string",  "single", "optional", "rw"),
            )),
        )
        params = tools_common.create_define_command(customObjects)
        result = test_awa_client_define.client_define(self.config, *params)
        self.assertEqual(0, result.code)

        result = client_set(self.config, "--create /1001")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_create_optional_resource(self):
        # test that the set tool can create an object instance without specifying an ID
        #expectedStdout = "Create /1000/0"
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        customObjects = (
            tools_common.CustomObject("Object1001", 1001, False, "single", (
                    tools_common.CustomResource("Resource100", 100, "string",  "single", "optional", "rw"),
            )),
        )
        params = tools_common.create_define_command(customObjects)
        result = test_awa_client_define.client_define(self.config, *params)
        self.assertEqual(0, result.code)

        result = client_set(self.config, "--create /1001/0/")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        result = client_set(self.config, "--create /1001/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        # TODO: check that instance 0 is present

    def test_create_object_instance_twice(self):
        # test that the set tool can create an object instance without specifying an ID,
        # and subsequent calls create increasing IDs.

        # TODO: check that instance 0 is *not* present

        result = client_set(self.config, "--create /2000")
        #self.assertEqual("Created /1000/0", result.stdout)
        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertEqual(0, result.code)
        # TODO: check that instance 0 is present

        result = client_set(self.config, "--create /2000")
        #self.assertEqual("Created /1000/1", result.stdout)
        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertEqual(0, result.code)
        # TODO: check that instance 1 is present

        result = client_set(self.config, "--create /2000")
        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertEqual(0, result.code)
        # TODO: check that instance 2 is present

    def test_create_object_instance_by_ID(self):
        # TODO: test that the set tool can create an object instance with the specified ID
        result = client_set(self.config, "--create /2000/5")
        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertEqual(0, result.code)
        # TODO: check that instance 5 is present

    def test_create_object_instance_by_existing_ID(self):
        # test that the set tool returns an error when creating an object instance with an existing ID
        result = client_set(self.config, "--create /2000/19")
        self.assertEqual("", result.stdout)
        self.assertEqual("", result.stderr)
        self.assertEqual(0, result.code)
        # TODO: check that instance 19 is present

        # attempt should fail:
        result = client_set(self.config, "--create /2000/19")
        self.assertEqual("", result.stdout)
        self.assertEqual("AwaClientSetOperation_Perform failed\nFailed to set on path /2000/19: AwaError_CannotCreate\n", result.stderr)
        self.assertEqual(1, result.code)

    def test_create_optional_resource(self):
        # test that the set tool can create an optional resource
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = client_set(self.config, "--create /2000/2")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        result = client_set(self.config, "--create /2000/2/102")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)


class TestSetBasic(tools_common.BasicTestCase):

    def test_set_help(self):
        expectedStdout = """awa-client-set 1.0

LWM2M Client Resource Set Operation

Usage: awa-client-set [OPTIONS]... [PATHS]...

  -h, --help                Print help and exit
  -V, --version             Print version and exit
  -v, --verbose             Increase program verbosity  (default=off)
  -d, --debug               Increase program verbosity  (default=off)
  -a, --ipcAddress=ADDRESS  Connect to Client IPC Address
                              (default=`127.0.0.1')
  -p, --ipcPort=PORT        Connect to Client IPC port  (default=`12345')
  -c, --create=STRING       Create a new Object Instance or Optional Resource

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
            code, stdout, stderr = client_set(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)


    def test_set_version(self):
        expectedStdout = "awa-client-set 1.0\n"
        expectedStderr = ""
        expectedCode = 0
        versionOptions = ("--version","-V")

        for option in versionOptions:
            code, stdout, stderr = client_set(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)

    def test_set_no_args(self):
        expectedStdout = ""
        expectedStderr = "Specify one or more resource paths.\n"
        expectedCode = 1

        code, stdout, stderr = client_set(self.config)
        self.assertEqual(expectedStdout, stdout)
        self.assertEqual(expectedStderr, stderr)
        self.assertEqual(expectedCode, code)
