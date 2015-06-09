# Tests related to server-side operations only

import unittest
import subprocess
import time

import overlord
import common
from collections import namedtuple

import tools_common
from tools_common import CustomObject
from tools_common import CustomResource
from tools_common import client_define
from tools_common import server_create
from tools_common import server_define
from tools_common import server_delete
from tools_common import server_execute
from tools_common import server_list_clients
from tools_common import server_observe
from tools_common import server_read
from tools_common import server_write

class TestServer(tools_common.AwaTest):

    def test_read_write(self):
        # test that a single resource can be set and retrieved
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/3/0/15=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        expectedStdout = "    Timezone[3/0/15]: abc\n"
        expectedStderr = ""
        expectedCode = 0

        result = server_read(self.config, "/3/0/15")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_read_write_single_resource_string(self):
        # test that a single string resource can be set and retrieved
        manufacturer = "ACME Corp."
        expectedStdout = "    Resource100[1000/0/100]: %s\n" % (manufacturer,)
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/100=\"%s\"" % (manufacturer,))
        self.assertEqual(expectedCode, result.code)

        result = server_read(self.config, "/1000/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_read_write_single_resource_integer(self):
        # test that a single integer resource can be set and retrieved
        value = 3
        expectedStdout = "    Resource101[1000/0/101]: %d\n" % (value,)
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "--verbose /1000/0/101=%i" % (value,))
        self.assertEqual(expectedCode, result.code)

        result = server_read(self.config, "/1000/0/101")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_read_write_single_resource_float(self):
        # test that a single float resource can be set and retrieved
        value = 3.5
        expectedStdout = "    Resource102[1000/0/102]: %f\n" % (value,)
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/102=%f" % (value,))
        self.assertEqual(expectedCode, result.code)

        result = server_read(self.config, "/1000/0/102")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_read_write_single_resource_boolean_true(self):
        # test that a single boolean resource can be set and retrieved
        value = True

        expectedStdout = "Write /1000/0/103/0 <- True\n"
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "--verbose /1000/0/103=%r" % (value,))

        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        expectedStdout = "    Resource103[1000/0/103]: %r\n" % (value,)

        result = server_read(self.config, "/1000/0/103")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_read_write_single_resource_boolean_false(self):
        # test that a single boolean resource can be set and retrieved
        value = False

        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/103=%r" % (value,))

        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        expectedStdout = "    Resource103[1000/0/103]: %r\n" % (value,)

        result = server_read(self.config, "/1000/0/103")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_read_write_single_resource_boolean_numbers(self):
        # test correct values for setting boolean resource with integers: 0,1,2 (False,True,True)
        for value in range(3):
            expectedStdout = ""
            expectedStderr = ""
            expectedCode = 0

            result = server_write(self.config, "/1000/0/103=%d" % (value,))

            self.assertEqual(expectedStdout, result.stdout)
            self.assertEqual(expectedStderr, result.stderr)
            self.assertEqual(expectedCode, result.code)

            expectedStdout = "    Resource103[1000/0/103]: %r\n" % (bool(value),)
            #print "Boolean expectedStdout: %s" % (expectedStdout,)
            result = server_read(self.config, "/1000/0/103")
            self.assertEqual(expectedStdout, result.stdout)
            self.assertEqual(expectedStderr, result.stderr)
            self.assertEqual(expectedCode, result.code)

    def test_read_write_single_resource_time(self):
        # test that a single time resource (64 bit integer) can be set and retrieved
        value = 1442972971
        expectedStdout = "    Resource104[1000/0/104]: %d\n" % (value,)
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/104=%d" % (value,))
        self.assertEqual(expectedCode, result.code)

        result = server_read(self.config, "/1000/0/104")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    @unittest.skip("Objlink type not supported")
    def test_read_write_single_resource_objlink(self):
        # test that a single object link resource can be set and retrieved
        link = "/3/0"
        expectedStdout = "    Resource106[1000/0/106]: ObjectLink[%s]\n" % (link.strip('/').replace('/', ':'))
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/106=\"%s\"" % (link,))
        self.assertEqual(expectedCode, result.code)

        result = server_read(self.config, "/1000/0/106")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_write_single_resource_none(self):
        # test that a single none resource (executable) cannot be set
        value = 12345
        expectedStdout = "FlowDeviceMgmt_Pull failed : (-2) The requested resource/instance/object does not exist\n"
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/107=%d" % (value,))
        self.assertEqual(expectedCode, result.code)

    def test_read_single_resource_none(self):
        # test that a single none resource (executable) cannot be retrieved
        expectedStdout = "FlowDeviceMgmtServer_Read failed : (-11) Request Failed\n"
        expectedStderr = ""
        expectedCode = 0

        result = server_read(self.config, "/1000/0/107")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_read_write_multiple_resources_same_instance(self):
        # test that multiple resources from the same instance can be set and retrieved with a single command
        timezone = "ACME Corp."
        currentTime = 123456789
        expectedStdout = \
"""    Timezone[3/0/15]: %s
    CurrentTime[3/0/13]: %d
""" % (timezone, currentTime)
        expectedStderr = ""
        expectedCode = 0

        server_write(self.config,
                     "/3/0/15=\"%s\"" % (timezone,),
                     "/3/0/13=%d" % (currentTime,))

        result = server_read(self.config, "/3/0/15", "/3/0/13")

        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_read_write_multiple_resources_different_instances(self):
        # test that multiple resources from different instances can be set and retrieved with a single command
        timezone = "ACME Corp."
        modelNumber = "1234567890"
        expectedStdout = \
"""    Timezone[3/0/15]: %s
    Resource100[1000/0/100]: %s
""" % (timezone, modelNumber)
        expectedStderr = ""
        expectedCode = 0

        server_write(self.config, "/3/0/15=\"%s\"" % (timezone,), "/1000/0/100=\"%s\"" % (modelNumber,))

        result = server_read(self.config, "/3/0/15", "/1000/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_read_write_invalid_value(self):
        # first set an initial value for an integer resource.
        # Then set the integer resource with a string value. Currently will set the value to 0

        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/101=12345")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        expectedStdout = "    Resource101[1000/0/101]: 12345\n"
        expectedStderr = ""
        expectedCode = 0

        result = server_read(self.config, "/1000/0/101")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/101=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        expectedStdout = "    Resource101[1000/0/101]: 0\n"
        expectedStderr = ""
        expectedCode = 0

        result = server_read(self.config, "/1000/0/101")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_read_write_delete_read(self):
        # test that we can no longer read the value of a resource after it has been deleted
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/100=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        expectedStdout = "    Resource100[1000/0/100]: abc\n"
        expectedStderr = ""
        expectedCode = 0

        result = server_read(self.config, "/1000/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_delete(self.config, "/1000/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        expectedStdout = "FlowDeviceMgmtServer_Read failed : (-11) Request Failed\n"
        expectedStderr = ""
        expectedCode = 0

        result = server_read(self.config, "/1000/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_set_multiple_instances_on_single_instance_object(self):
        #test that we can't create multiple object instances on a single instance object
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/100=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        expectedStdout = "FlowDeviceMgmtServer_Write failed : (-11) Request Failed\n" # AwaError_IDInvalid?
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/1/100=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_set_multiple_instances_on_single_instance_resource(self):
        #test that we can't set multiple resource instances on a single instance resource
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/100/0=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        expectedStdout = "FlowDeviceMgmtServer_Write failed : (-11) Request Failed\n" # AwaError_IDInvalid?
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/100/1=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_read_write_only_resource(self):
        # test that we can't read from a write only resource
        customObjects = (
            CustomObject("Object1001", 1001, False, "single", (
                    CustomResource("Resource100", 100, "string",  "single", "optional", "w"),
            )),
        )

        #define and set the write only object
        params = tools_common.create_define_command(customObjects)
        result = client_define(self.config, *params)
        self.assertEqual(0, result.code)
        result = server_define(self.config, *params)
        self.assertEqual(0, result.code)

        result = server_write(self.config, "/1001/0/100=abc")
        self.assertEqual(0, result.code)

        expectedStdout = "FlowDeviceMgmtServer_Read failed : (-11) Request Failed\n"
        expectedStderr = ""
        expectedCode = 0

        # attempt to read a write only object, should fail
        result = server_read(self.config, "/1001/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_write_read_only_resource(self):
        # test that we can't write to a read only resource
        customObjects = (
            CustomObject("Object1001", 1001, False, "single", (
                    CustomResource("Resource100", 100, "string",  "single", "optional", "r"),
            )),
        )
        params = tools_common.create_define_command(customObjects)
        result = client_define(self.config, *params)
        self.assertEqual(0, result.code)
        result = server_define(self.config, *params)
        self.assertEqual(0, result.code)

        # FlowDeviceMgmtServer_Write failed : (-11) The requested operation is not permitted\n
        expectedStdout = "FlowDeviceMgmtServer_Write failed : (-11) Request Failed\n"
        expectedStderr = ""
        expectedCode = 0

        # set a read only object
        result = server_write(self.config, "/1001/0/100=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_delete_custom_object_instance(self):
        # test we can delete a custom object instance
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/100=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_delete(self.config, "/1000/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_server_read_deleted_instance(self):
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/1000/0/100=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        result = server_delete(self.config, "/1000/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        expectedStdout = "FlowDeviceMgmtServer_Read failed : (-11) Request Failed\n"
        expectedStderr = ""
        expectedCode = 0

        result = server_read(self.config, "/1000/0/100")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_execute_multiple_resources(self):
        # test we can execute multiple executable resources with one command
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        #first create a custom object instance by setting a writable resource inside it
        result = server_write(self.config, "/1000/0/100=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        #run both executable resources
        result = server_execute(self.config, "/3/0/4 /1000/0/107")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    def test_observe_resource_single_change(self):
        # test that we can observe to a single change of value on a specified resource

        # open server observe subprocess. Only wait for a single change to the resource
        port = self.config.serverIpcPort
        clientEndpointName = self.config.clientEndpointName
        observeProcess = tools_common.run_non_blocking(tools_common.SERVER_OBSERVE,
                                                    "--verbose --ipcPort %i --clientID %s --waitCount 1 --waitTime 5 /3/0/15" % (port, clientEndpointName))

        # wait for observe process to start up
        self.assertEqual(observeProcess.stdout.readline(), "Observe /3/0/15\n")
        self.assertEqual(observeProcess.stdout.readline(), "Waiting for 1 notifications over 5 seconds:\n")

        # do write command
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        result = server_write(self.config, "/3/0/15=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        # read observe output
        expectedStdout = "Notify 1:\n    Timezone[3/0/15]: abc\nClean up\n"
        expectedStderr = ""
        expectedCode = 0

        result = tools_common.non_blocking_get_run_result(observeProcess)

        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_observe_multiple_resources_single_change(self):
        # test that we can observe to a single change of value on
        # one of the multiple resources specified

        # open client observe subprocess. Only wait for a single change to one of the resources
        port = self.config.serverIpcPort
        clientEndpointName = self.config.clientEndpointName
        observeProcess = tools_common.run_non_blocking(tools_common.SERVER_OBSERVE,
                                                    "--verbose --ipcPort %i --clientID %s --waitCount 1 --waitTime 5 /3/0/14 /3/0/15" % (port, clientEndpointName))

        # wait for observe process to start up
        self.assertEqual("Observe /3/0/14\n", observeProcess.stdout.readline())
        self.assertEqual("Observe /3/0/15\n", observeProcess.stdout.readline())
        self.assertEqual("Waiting for 1 notifications over 5 seconds:\n", observeProcess.stdout.readline())

        # do set command and expect notifications from the observe subprocess
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        # set the first resource
        result = server_write(self.config, "/3/0/14=abc")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        expectedStdout = "Notify 1:\n    UTCOffset[3/0/14]: abc\nClean up\nClean up\n"
        expectedStderr = ""
        expectedCode = 0

        result = tools_common.non_blocking_get_run_result(observeProcess)

        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    def test_observe_multiple_resources_single_change_2(self):
        # test that we can observe to a single change of value on
        # one of the multiple resources specified

        # open client observe subprocess. Only wait for a single change to one of the resources
        port = self.config.serverIpcPort
        clientEndpointName = self.config.clientEndpointName
        observeProcess = tools_common.run_non_blocking(tools_common.SERVER_OBSERVE,
                                                    "--verbose --ipcPort %i --clientID %s --waitCount 1 --waitTime 5 /3/0/14 /3/0/15" % (port, clientEndpointName))

        # wait for observe process to start up
        self.assertEqual("Observe /3/0/14\n", observeProcess.stdout.readline())
        self.assertEqual("Observe /3/0/15\n", observeProcess.stdout.readline())
        self.assertEqual("Waiting for 1 notifications over 5 seconds:\n", observeProcess.stdout.readline())

        # do set command and expect notifications from the observe subprocess
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        # set the second resource
        result = server_write(self.config, "/3/0/15=1234")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

        expectedStdout = "Notify 1:\n    Timezone[3/0/15]: 1234\nClean up\nClean up\n"
        expectedStderr = ""
        expectedCode = 0

        result = tools_common.non_blocking_get_run_result(observeProcess)

        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)

    @unittest.skip("Currently unstable - TODO")
    def test_observe_resource_multiple_changes(self):
        # test that we can observe to multiple changes of value on a specified resource
        num_changes = 5
        # open client observe subprocess. Wait for 5 changes to the resource
        port = self.config.serverIpcPort
        clientEndpointName = self.config.clientEndpointName
        observeProcess = tools_common.run_non_blocking(tools_common.SERVER_OBSERVE,
                                                    "--verbose --ipcPort %i --clientID %s --waitCount %i --waitTime 5 /3/0/0" % (port, clientEndpointName, num_changes))

        # wait for observe process to start up
        self.assertEqual(observeProcess.stdout.readline(), "Observe /3/0/0\n")
        self.assertEqual(observeProcess.stdout.readline(), "Waiting for 5 notifications over 5 seconds:\n")

        # do get command
        expectedStdout = ""
        expectedStderr = ""
        expectedCode = 0

        for i in range(num_changes):
            result = server_write(self.config, "/3/0/0=%i" % (i,))
            self.assertEqual(expectedStdout, result.stdout)
            self.assertEqual(expectedStderr, result.stderr)
            self.assertEqual(expectedCode, result.code)
            #time.sleep(0.5) # give time for observe tool to notice change

        # read observe output
        expectedStdout = "Notify 1:\n    Manufacturer[3/0/0]: 0\nNotify 2:\n    Manufacturer[3/0/0]: 1\nNotify 3:\n    Manufacturer[3/0/0]: 2\nNotify 4:\n    Manufacturer[3/0/0]: 3\nNotify 5:\n    Manufacturer[3/0/0]: 4\nClean up\n"
        expectedStderr = ""
        expectedCode = 0

        result = tools_common.non_blocking_get_run_result(observeProcess)

        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode, result.code)
