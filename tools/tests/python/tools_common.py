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

import subprocess
import os
import os.path
import unittest
from collections import namedtuple
import re

import common
import config

try:
    LWM2M_TOOLS_PATH = os.environ['LWM2M_TOOLS_PATH']
except KeyError:
    LWM2M_TOOLS_PATH = '.'

# binaries to test
CLIENT_DEFINE       = os.path.join(LWM2M_TOOLS_PATH, "./awa-client-define")
CLIENT_GET          = os.path.join(LWM2M_TOOLS_PATH, "./awa-client-get")
CLIENT_SET          = os.path.join(LWM2M_TOOLS_PATH, "./awa-client-set")
CLIENT_DELETE       = os.path.join(LWM2M_TOOLS_PATH, "./awa-client-delete")
CLIENT_SUBSCRIBE    = os.path.join(LWM2M_TOOLS_PATH, "./awa-client-subscribe")
CLIENT_EXPLORE      = os.path.join(LWM2M_TOOLS_PATH, "./awa-client-explore")

SERVER_CREATE           = os.path.join(LWM2M_TOOLS_PATH, "./awa-server-create")
SERVER_DEFINE           = os.path.join(LWM2M_TOOLS_PATH, "./awa-server-define")
SERVER_DELETE           = os.path.join(LWM2M_TOOLS_PATH, "./awa-server-delete")
SERVER_EXECUTE          = os.path.join(LWM2M_TOOLS_PATH, "./awa-server-execute")
SERVER_LIST_CLIENTS     = os.path.join(LWM2M_TOOLS_PATH, "./awa-server-list-clients")
SERVER_OBSERVE          = os.path.join(LWM2M_TOOLS_PATH, "./awa-server-observe")
SERVER_READ             = os.path.join(LWM2M_TOOLS_PATH, "./awa-server-read")
SERVER_WRITE            = os.path.join(LWM2M_TOOLS_PATH, "./awa-server-write")
SERVER_WRITE_ATTRIBUTES = os.path.join(LWM2M_TOOLS_PATH, "./awa-server-write-attributes")

# tuples for tool response and dummy object creation
RunResult = namedtuple("RunResult", ["code", "stdout", "stderr"])
CustomObject = namedtuple("CustomObject", ["name", "ID", "mandatory", "instances", "resources"])
CustomResource = namedtuple("CustomResource", ["name", "ID", "type", "instances", "required", "operations"])

class DefaultTestConfiguration(object):

    SPAWN_CLIENT_DAEMON = True
    SPAWN_SERVER_DAEMON = True

    #SERVER_IPC_PORT = 61631
    #CLIENT_IPC_PORT = 61632

    #SERVER_COAP_PORT = range(9871, 9881)
    #CLIENT_COAP_PORT = range(9882, 9892)

    SERVER_IPC_PORT = range(61631, 61641)
    CLIENT_IPC_PORT = range(61642, 61652)

    SERVER_COAP_PORT = 9871
    CLIENT_COAP_PORT = 9872

    SERVER_ADDRESS = "127.0.0.1"

    DAEMON_PATH = "../.."

    SERVER_LOG_FILE = "awa_serverd.log"
    CLIENT_LOG_FILE = "awa_clientd.log"

    BOOTSTRAP_CONFIG_FILE = "../bootstrap.config"

    CLIENT_ENDPOINT_NAME = "TestClient"

class NoClientTestConfiguration(DefaultTestConfiguration):

    SPAWN_CLIENT_DAEMON = False


def strip_prefix(line):
    """Remove the [filename:lineno] prefix in a Log line, if present."""
    m = re.match(r"^(\[.+:\d+\]\s*)(.*)", line)
    if m:
        return m.group(2)
    else:
        return line

def strip_prefixes(text):
    output = ""
    for line in text.splitlines():
        output += strip_prefix(line) + "\n"
    return output

def run(program, *args):
    assert os.path.isfile(program), "invalid binary or path: " + program
    command = " ".join([ program ] + list(args))
    #print command
    child = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = child.communicate()
    return RunResult(child.returncode, strip_prefixes(stdout), strip_prefixes(stderr))

def run_pipe_stdin(program, inputText, *args):
    assert os.path.isfile(program), "invalid binary or path: " + program
    command = " ".join([ program ] + list(args))
    print command
    child = subprocess.Popen(command, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = child.communicate(inputText)
    return RunResult(child.returncode, strip_prefixes(stdout), strip_prefixes(stderr))

def run_non_blocking(program, *args):
    assert os.path.isfile(program), "invalid binary or path: " + program
    command = " ".join([ program ] + list(args))
    print command
    return subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

def non_blocking_get_run_result(child):
    stdout, stderr = child.communicate()
    return RunResult(child.returncode, strip_prefixes(stdout), strip_prefixes(stderr))

def run_client(config, program, *args):
    return run(program,
               "--ipcPort=%d" % (config.clientIpcPort, ),
               *args)

def run_server(config, program, *args):
    return run(program,
               "--ipcPort=%d" % (config.serverIpcPort, ),
               *args)

def run_server_with_client(config, program, *args):
    return run_server(config, program, "--clientID=%s" % (config.clientEndpointName,), *args)

def run_server_with_client_stdin(config, program, inputText, *args):
    ipcPort = config.serverIpcPort
    endpoint = config.clientEndpointName
    return run_pipe_stdin(program, inputText, "--ipcPort=%d" % (ipcPort,),
                          "--clientID=%s" % (endpoint,),
                          "--stdin",
                          *args)

def create_define_command(customObjects):
    params = []
    for object in customObjects:
        params += [
            "--objectID=%d" % (object.ID,),
            "--objectName='%s'" % (object.name,),
            "--objectInstances=%s" % (object.instances,),
        ]

        if object.mandatory:
            params += [ "--objectMandatory" ]

        for resource in object.resources:
            params += [
                "--resourceID=%d" % (resource.ID,),
                "--resourceName='%s'" % (resource.name,),
                "--resourceType=%s" % (resource.type,),
                "--resourceInstances=%s" % (resource.instances,),
                "--resourceRequired=%s" % (resource.required,),
                "--resourceOperations=%s" % (resource.operations,),
            ]

        #print params
    return params

class BasicTestCase(unittest.TestCase):
    """TestCase for basic tool usage without daemons"""
    def setUp(self):
        self.config = config.Config(DefaultTestConfiguration)

class AwaTest(common.SpawnDaemonsTestCase):

    def __init__(self, *args, **kwargs):
        super(AwaTest, self).__init__(*args, **kwargs)

    def setUp(self):
        common.setConfigurationClass(DefaultTestConfiguration)
        super(AwaTest, self).setUp()
        self.define_fixture_objects()

    def define_fixture_objects(self):
        # create custom objects
        customObjects = (
            CustomObject("Object1000", 1000, False, "single", (
                    CustomResource("Resource100", 100, "string",  "single", "optional", "rw"),
                    CustomResource("Resource101", 101, "integer", "single", "optional", "rw"),
                    CustomResource("Resource102", 102, "float",   "single", "optional", "rw"),
                    CustomResource("Resource103", 103, "boolean", "single", "optional", "rw"),
                    CustomResource("Resource104", 104, "time",    "single", "optional", "rw"),
                    CustomResource("Resource105", 105, "opaque",  "single", "optional", "rw"),
                    CustomResource("Resource106", 106, "objlink", "single", "optional", "rw"),
                    CustomResource("Resource107", 107, "none",    "single", "optional", "e"),

                    CustomResource("Resource200", 200, "string",  "multiple", "optional", "rw"),
                    CustomResource("Resource201", 201, "integer", "multiple", "optional", "rw"),
                    CustomResource("Resource202", 202, "float",   "multiple", "optional", "rw"),
                    CustomResource("Resource203", 203, "boolean", "multiple", "optional", "rw"),
                    CustomResource("Resource204", 204, "time",    "multiple", "optional", "rw"),
                    CustomResource("Resource205", 205, "opaque",  "multiple", "optional", "rw"),
                    CustomResource("Resource206", 206, "objlink", "multiple", "optional", "rw"),
            )),

            CustomObject("Object2000", 2000, False, "multiple", (
                    CustomResource("Resource100", 100, "string",  "single", "optional", "rw"),
                    CustomResource("Resource101", 101, "integer", "single", "optional", "rw"),
                    CustomResource("Resource102", 102, "float",   "single", "optional", "rw"),
                    CustomResource("Resource103", 103, "boolean", "single", "optional", "rw"),
                    CustomResource("Resource104", 104, "time",    "single", "optional", "rw"),
                    CustomResource("Resource105", 105, "opaque",  "single", "optional", "rw"),
                    CustomResource("Resource106", 106, "objlink", "single", "optional", "rw"),
                    CustomResource("Resource107", 107, "none",    "single", "optional", "e"),

                    CustomResource("Resource200", 200, "string",  "multiple", "optional", "rw"),
                    CustomResource("Resource201", 201, "integer", "multiple", "optional", "rw"),
                    CustomResource("Resource202", 202, "float",   "multiple", "optional", "rw"),
                    CustomResource("Resource203", 203, "boolean", "multiple", "optional", "rw"),
                    CustomResource("Resource204", 204, "time",    "multiple", "optional", "rw"),
                    CustomResource("Resource205", 205, "opaque",  "multiple", "optional", "rw"),
                    CustomResource("Resource206", 206, "objlink", "multiple", "optional", "rw"),
            )),
        )

        print "Creating custom objects"

        # FIXME: define tool can currently only define individual objects
        for obj in customObjects:
            params = create_define_command((obj,))
            result = test_awa_server_define.server_define(self.config, *params)
            self.assertEqual(0, result.code)

            if self.config.spawnClientDaemon is True:
                result = test_awa_client_define.client_define(self.config, *params)
                self.assertEqual(0, result.code)
                # create our objects and optional object instances
                createCommand = ["--create /%d" % (obj.ID,)]
                for resource in obj.resources:
                    createCommand += ["--create /%d/0/%d" % (obj.ID, resource.ID)]
                result = test_awa_client_set.client_set(self.config, " ".join(createCommand))
                self.assertEqual(0, result.code)


# avoid circular dependencies
import test_awa_client_define
import test_awa_server_define
import test_awa_client_set
