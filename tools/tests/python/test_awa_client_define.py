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

# Tests related to basic client define operations

import unittest

import common
import tools_common
from tools_common import CustomObject
from tools_common import CustomResource

def client_define(config, *args):
    return tools_common.run_client(config, tools_common.CLIENT_DEFINE, *args)

class TestDefine(tools_common.AwaTest):

    def test_define_object(self):
        # test that we can define a custom object
        customObjects = (
            CustomObject("Object1001", 1001, False, "single", (
                    CustomResource("Resource100", 100, "string",  "single", "optional", "rw"),
            )),
        )
        params = tools_common.create_define_command(customObjects)

        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        # define our custom object
        result = client_define(self.config, *params)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_define_object_verbose(self):
        # test that we can define a custom object
        customObjects = (
            CustomObject("Object1001", 1001, False, "single", (
                    CustomResource("Resource100", 100, "string",  "single", "optional", "rw"),
            )),
        )
        params = ["--verbose"] + tools_common.create_define_command(customObjects)

        expectedStdout = "Session IPC configured for UDP: address 127.0.0.1, port %d\nSession connected\nObjectID 1001 defined successfully\nSession disconnected\n" % (self.config.clientIpcPort, )
        expectedStderr = ""
        expectedCode = 0

        # define our custom object
        result = client_define(self.config, *params)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_redefine_existing_object(self):
        # test that we can't define an object if the ID is already used
        customObjects = (
            CustomObject("Object1001", 1001, False, "single", (
                    CustomResource("Resource100", 100, "string",  "single", "optional", "rw"),
            )),
        )
        params = tools_common.create_define_command(customObjects)

        # define our custom object
        result = client_define(self.config, *params)
        self.assertEqual(0, result.code)

        expectedStdout = ""
        expectedStderr = "AwaError_AlreadyDefined: Failed to add object definition\nDefine operation failed\n"
        expectedCode = 1

        #attempt to redefine custom objects (should fail)
        result = client_define(self.config, *params)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_define_multiple_resources_same_ID(self):
        # test that we can't define multiple resources with the same ID in a single object
        customObjects = (
            CustomObject("Object1001", 1001, False, "single", (
                    CustomResource("Resource100", 100, "string",  "single", "optional", "rw"),
                    CustomResource("Resource100", 100, "string",  "single", "optional", "rw"),
            )),
        )

        params = tools_common.create_define_command(customObjects)
        result = client_define(self.config, *params)

        expectedStdout = ""
        expectedStderr = "AwaError_AlreadyDefined: \nCould not add resource definition (resource [100] Resource100) to object definition\nFailed to create object definition\n"
        expectedCode = 1

        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_define_unsupported_resource_type(self):
        # test that we can't use 'byte' as a resource type without crashing
        customObjects = (
            CustomObject("Object1001", 1001, False, "single", (
                    CustomResource("Resource100", 100, "byte",  "single", "optional", "rw"),
            )),
        )
        params = tools_common.create_define_command(customObjects)

        expectedStdout = ""
        expectedStderr = tools_common.CLIENT_DEFINE + ": invalid argument, \"byte\", for option `--resourceType\' (`-t\')\n"
        expectedCode = 1

        # define our custom object
        result = client_define(self.config, *params)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_define_no_resources(self):
        # test that we can define an object without resources
        customObjects = (
            CustomObject("Object1001", 1001, False, "single", ()),
        )
        params = ["--verbose"] + tools_common.create_define_command(customObjects)

        expectedStdout = "Session IPC configured for UDP: address 127.0.0.1, port %d\nSession connected\nObjectID 1001 defined successfully\nSession disconnected\n"  % (self.config.clientIpcPort, )
        expectedStderr = ""
        expectedCode = 0

        # define our custom object
        result = client_define(self.config, *params)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_define_read_write_execute(self):
        # test that we can't create a resource that supports read, write and execute together (only r, w, rw or e)
        customObjects = (
            CustomObject("Object1001", 1001, False, "single", (
                    CustomResource("Resource100", 100, "string",  "single", "optional", "rwe"),
            )),
        )
        params = tools_common.create_define_command(customObjects)

        expectedStdout = ""
        expectedStderr = tools_common.CLIENT_DEFINE + ": invalid argument, \"rwe\", for option `--resourceOperations\' (`-k\')\n"
        expectedCode = 1

        # define our custom object
        result = client_define(self.config, *params)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_define_executable_non_none_resource(self):
        # test that only none resource types can be executable
        customObjects = (
            CustomObject("Object1001", 1001, False, "single", (
                    CustomResource("Resource100", 100, "string",  "single", "optional", "e"),
            )),
        )
        params = tools_common.create_define_command(customObjects)

        expectedStdout = ""
        expectedStderr = "AwaError_DefinitionInvalid: \nCould not add resource definition (resource [100] Resource100) to object definition\nFailed to create object definition\n"
        expectedCode = 1

        # define our custom object
        result = client_define(self.config, *params)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_define_multiple_instance_executable_resource(self):
        # test that executable resources can only be single instance
        customObjects = (
            CustomObject("Object1001", 1001, False, "single", (
                    CustomResource("Resource100", 100, "string",  "multiple", "optional", "e"),
            )),
        )
        params = tools_common.create_define_command(customObjects)

        expectedStdout = ""
        expectedStderr = "AwaError_DefinitionInvalid: \nCould not add resource definition (resource [100] Resource100) to object definition\nFailed to create object definition\n"
        expectedCode = 1

        # define our custom object
        result = client_define(self.config, *params)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)


class TestDefineBasic(tools_common.BasicTestCase):

    usage = """awa-client-define 1.0

LWM2M Client Object Definition Operation

Usage: awa-client-define [OPTIONS]... [PATHS]...

  -h, --help                    Print help and exit
  -V, --version                 Print version and exit
  -v, --verbose                 Increase program verbosity  (default=off)
  -d, --debug                   Increase program verbosity  (default=off)
  -a, --ipcAddress=ADDRESS      Connect to Client IPC Address
                                  (default=`127.0.0.1')
  -p, --ipcPort=PORT            Connect to Client IPC port  (default=`12345')

 Mode: XML Definition
  -x, --xmlFile=FILENAME        Load XML Definition file

 Mode: Cmdline Definition
  -o, --objectID=ID             Object ID
  -j, --objectName=NAME         Object name
  -m, --objectMandatory         Object is required or optional  (default=off)
  -y, --objectInstances=TYPE    Object supports single or multiple instances
                                  (possible values="single", "multiple"
                                  default=`single')
  -r, --resourceID=ID           Resource ID
  -n, --resourceName=NAME       Resource Name
  -t, --resourceType=TYPE       Resource Type  (possible values="opaque",
                                  "integer", "float", "boolean",
                                  "string", "time", "objlink", "none")
  -u, --resourceInstances=VALUE Resource supports single or multiple instances
                                  (possible values="single", "multiple")
  -q, --resourceRequired=VALUE  Resource is required or optional  (possible
                                  values="optional", "mandatory")
  -k, --resourceOperations=VALUE
                                Resource Operation  (possible values="r",
                                  "w", "e", "rw")
"""

    def test_define_help(self):
        expectedStdout = self.usage
        expectedStderr = ""
        expectedCode = 0
        helpOptions = ("--help","-h")

        for option in helpOptions:
            code, stdout, stderr = client_define(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)

    def test_define_version(self):
        expectedStdout = "awa-client-define 1.0\n"
        expectedStderr = ""
        expectedCode = 0
        versionOptions = ("--version","-V")

        for option in versionOptions:
            code, stdout, stderr = client_define(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)

    def test_define_no_args(self):
        expectedStdout = self.usage
        expectedStderr = ""
        expectedCode = 1

        code, stdout, stderr = client_define(self.config)
        self.assertEqual(expectedStdout, stdout)
        self.assertEqual(expectedStderr, stderr)
        self.assertEqual(expectedCode, code)
