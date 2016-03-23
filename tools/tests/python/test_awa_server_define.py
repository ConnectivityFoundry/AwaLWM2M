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

# Tests related to basic server define operations

import unittest

import common
import tools_common
from tools_common import CustomObject
from tools_common import CustomResource

def server_define(config, *args):
    return tools_common.run_server(config, tools_common.SERVER_DEFINE, *args)

class TestDefine(tools_common.AwaTest):

    def test_define_object(self):
        # test that we can define a custom object 
        customObjects = (
            CustomObject("Object1234", 1234, False, "single", (
                    CustomResource("Resource100", 100, "string",  "single", "optional", "rw"),
            )),
        )
        params = tools_common.create_define_command(customObjects)
        
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0
        
        # define our custom object
        result = server_define(self.config, *params)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_define_object_verbose(self):
        # test that we can define a custom object 
        customObjects = (
            CustomObject("Object1234", 1234, False, "single", (
                    CustomResource("Resource100", 100, "string",  "single", "optional", "rw"),
            )),
        )
        params = ["--verbose"] + tools_common.create_define_command(customObjects)
        
        expectedStdout = "Session IPC configured for UDP: address 127.0.0.1, port %d\nSession connected\nObjectID 1234 defined successfully\nSession disconnected\n" % (self.config.serverIpcPort, )
        expectedStderr = ""
        expectedCode = 0
        
        # define our custom object
        result = server_define(self.config, *params)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_redefine_existing_object(self):
        # test that we can't define an object if the ID is already used
        customObjects = (
            CustomObject("Object1234", 1234, False, "single", (
                    CustomResource("Resource100", 100, "string",  "single", "optional", "rw"),
            )),
        )
        params = tools_common.create_define_command(customObjects)
        
        # define our custom object
        result = server_define(self.config, *params)
        self.assertEqual(0, result.code)
        
        expectedStdout = ""
        expectedStderr = "AwaError_AlreadyDefined: Failed to add object definition\nDefine operation failed\n"
        expectedCode = 1
        
        #attempt to redefine custom objects (should fail)
        result = server_define(self.config, *params)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_define_multiple_resources_same_ID(self):
        # test that we can't define multiple resources with the same ID in a single object
        customObjects = (
            CustomObject("Object1234", 1234, False, "single", (
                    CustomResource("Resource100", 100, "string",  "single", "optional", "rw"),
                    CustomResource("Resource100", 100, "string",  "single", "optional", "rw"),
            )),
        )
        
        params = tools_common.create_define_command(customObjects)
        result = server_define(self.config, *params)
        
        expectedStdout = ""
        expectedStderr = "AwaError_AlreadyDefined: \nCould not add resource definition (resource [100] Resource100) to object definition\nFailed to create object definition\n"
        expectedCode = 1

        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def test_define_unsupported_resource_type(self):
        # test that we can't use 'byte' as a resource type without crashing
        customObjects = (
            CustomObject("Object1000", 1000, False, "single", (
                    CustomResource("Resource100", 100, "byte",  "single", "optional", "rw"),
            )),
        )
        params = tools_common.create_define_command(customObjects)
        
        expectedStdout = ""
        expectedStderr = tools_common.SERVER_DEFINE + ": invalid argument, \"byte\", for option `--resourceType\' (`-t\')\n"
        expectedCode = 1
        
        # define our custom object
        result = server_define(self.config, *params)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_define_no_resources(self):
        # test that we can define an object without resources
        customObjects = (
            CustomObject("Object1234", 1234, False, "single", ()),
        )
        params = ["--verbose"] + tools_common.create_define_command(customObjects)
        
        expectedStdout = "Session IPC configured for UDP: address 127.0.0.1, port %d\nSession connected\nObjectID 1234 defined successfully\nSession disconnected\n" % (self.config.serverIpcPort, )
        expectedStderr = ""
        expectedCode = 0
        
        # define our custom object
        result = server_define(self.config, *params)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def test_define_read_write_execute(self):
        # test that we can't create a resource that supports read, write and execute together (only r, w, rw or e)
        customObjects = (
            CustomObject("Object1000", 1000, False, "single", (
                    CustomResource("Resource100", 100, "string",  "single", "optional", "rwe"),
            )),
        )
        params = tools_common.create_define_command(customObjects)
        
        expectedStdout = ""
        expectedStderr = tools_common.SERVER_DEFINE + ": invalid argument, \"rwe\", for option `--resourceOperations\' (`-k\')\n"
        expectedCode = 1
        
        # define our custom object
        result = server_define(self.config, *params)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
    
    def test_define_executable_non_none_resource(self):
        # test that only none resource types can be executable
        customObjects = (
            CustomObject("Object1000", 1000, False, "single", (
                    CustomResource("Resource100", 100, "string",  "single", "optional", "e"),
            )),
        )
        params = tools_common.create_define_command(customObjects)
        
        expectedStdout = ""
        expectedStderr = "AwaError_DefinitionInvalid: \nCould not add resource definition (resource [100] Resource100) to object definition\nFailed to create object definition\n"
        expectedCode = 1
        
        # define our custom object
        result = server_define(self.config, *params)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        
    def test_define_multiple_instance_executable_resource(self):
        # test that executable resources can only be single instance
        customObjects = (
            CustomObject("Object1000", 1000, False, "single", (
                    CustomResource("Resource100", 100, "string",  "multiple", "optional", "e"),
            )),
        )
        params = tools_common.create_define_command(customObjects)
        
        expectedStdout = ""
        expectedStderr = "AwaError_DefinitionInvalid: \nCould not add resource definition (resource [100] Resource100) to object definition\nFailed to create object definition\n"
        expectedCode = 1
        
        # define our custom object
        result = server_define(self.config, *params)
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)
        

class TestDefineBasic(tools_common.BasicTestCase):
    
    usage = """awa-server-define 1.0\n\nLWM2M Server Object Definition Operation\n\nUsage: awa-server-define [OPTIONS]... [PATHS]...\n\n  -h, --help                    Print help and exit\n  -V, --version                 Print version and exit\n  -v, --verbose                 Increase program verbosity  (default=off)\n  -d, --debug                   Increase program verbosity  (default=off)\n  -a, --ipcAddress=ADDRESS      Connect to Server IPC Address\n                                  (default=`127.0.0.1\')\n  -p, --ipcPort=PORT            Connect to Server IPC port  (default=`54321\')\n\n Mode: XML Definition\n  -x, --xmlFile=FILENAME        Load XML Definition file\n\n Mode: Cmdline Definition\n  -o, --objectID=ID             Object ID\n  -j, --objectName=NAME         Object name\n  -m, --objectMandatory         Object is required or optional  (default=off)\n  -y, --objectInstances=TYPE    Object supports single or multiple instances\n                                  (possible values="single", "multiple"\n                                  default=`single\')\n  -r, --resourceID=ID           Resource ID\n  -n, --resourceName=NAME       Resource Name\n  -t, --resourceType=TYPE       Resource Type  (possible values="opaque",\n                                  "integer", "float", "boolean",\n                                  "string", "time", "objlink", "none")\n  -u, --resourceInstances=VALUE Resource supports single or multiple instances\n                                  (possible values="single", "multiple")\n  -q, --resourceRequired=VALUE  Resource is required or optional  (possible\n                                  values="optional", "mandatory")\n  -k, --resourceOperations=VALUE\n                                Resource Operation  (possible values="r",\n                                  "w", "e", "rw")\n"""

    def test_define_help(self):
        expectedStdout = self.usage
        expectedStderr = ""
        expectedCode = 0
        helpOptions = ("--help","-h")

        for option in helpOptions:
            code, stdout, stderr = server_define(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)

    def test_define_version(self):
        expectedStdout = "awa-server-define 1.0\n"
        expectedStderr = ""
        expectedCode = 0
        versionOptions = ("--version","-V")

        for option in versionOptions:
            code, stdout, stderr = server_define(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)

    def test_define_no_args(self):
        expectedStdout = self.usage
        expectedStderr = ""
        expectedCode = 1

        code, stdout, stderr = server_define(self.config)
        self.assertEqual(expectedStdout, stdout)
        self.assertEqual(expectedStderr, stderr)
        self.assertEqual(expectedCode, code)
