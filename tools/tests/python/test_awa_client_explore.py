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

# Tests related to getting defined objects or resources using basic client explore operations 

import unittest
import re

import common
import tools_common

def client_explore(config, *args):
    return tools_common.run_client(config, tools_common.CLIENT_EXPLORE, *args)

class TestExplore(tools_common.AwaTest):

    @unittest.skip("maintenance headache")
    def test_explore(self):
        expectedStdout = """Object ID:0 name:LWM2MSecurity minInstances:1 maxInstances:65535
       Resource: ID:0 name:LWM2MServerURI type:2 minInstances:1 maxInstances:1 operations:1
       Resource: ID:1 name:BootstrapServer type:5 minInstances:1 maxInstances:1 operations:1
       Resource: ID:2 name:SecurityMode type:3 minInstances:1 maxInstances:1 operations:1
       Resource: ID:3 name:PublicKeyorIDentity type:6 minInstances:1 maxInstances:1 operations:1
       Resource: ID:4 name:ServerPublicKeyorIDentity type:6 minInstances:1 maxInstances:1 operations:1
       Resource: ID:5 name:SecretKey type:6 minInstances:1 maxInstances:1 operations:1
       Resource: ID:6 name:SMSSecurityMode type:3 minInstances:1 maxInstances:1 operations:1
       Resource: ID:7 name:SMSBindingKeyParameters type:6 minInstances:1 maxInstances:1 operations:1
       Resource: ID:8 name:SMSBindingSecretKeys type:6 minInstances:1 maxInstances:1 operations:1
       Resource: ID:9 name:LWM2MServerSMSNumber type:3 minInstances:1 maxInstances:1 operations:1
       Resource: ID:10 name:ShortServerID type:3 minInstances:0 maxInstances:1 operations:1
       Resource: ID:11 name:ClientHoldOffTime type:3 minInstances:1 maxInstances:1 operations:1
Object ID:1 name:LWM2MServer minInstances:1 maxInstances:65535
       Resource: ID:0 name:ShortServerID type:3 minInstances:1 maxInstances:1 operations:2
       Resource: ID:1 name:Lifetime type:3 minInstances:1 maxInstances:1 operations:4
       Resource: ID:2 name:DefaultMinimumPeriod type:3 minInstances:0 maxInstances:1 operations:4
       Resource: ID:3 name:DefaultMaximumPeriod type:3 minInstances:0 maxInstances:1 operations:4
       Resource: ID:4 name:Disable type:1 minInstances:0 maxInstances:1 operations:5
       Resource: ID:5 name:DisableTimeout type:3 minInstances:0 maxInstances:1 operations:4
       Resource: ID:6 name:NotificationStoringWhenDisabledorOffline type:5 minInstances:1 maxInstances:1 operations:4
       Resource: ID:7 name:Binding type:2 minInstances:1 maxInstances:1 operations:4
       Resource: ID:8 name:RegistrationUpdateTrigger type:1 minInstances:1 maxInstances:1 operations:5
Object ID:2 name:LWM2MAccessControl minInstances:0 maxInstances:65535
       Resource: ID:0 name:ObjectID type:3 minInstances:1 maxInstances:1 operations:2
       Resource: ID:1 name:ObjectInstanceID type:3 minInstances:1 maxInstances:1 operations:2
       Resource: ID:2 name:ACL type:10 minInstances:0 maxInstances:65535 operations:4
       Resource: ID:3 name:AccessControlOwner type:3 minInstances:1 maxInstances:1 operations:4
Object ID:3 name:Device minInstances:1 maxInstances:1
       Resource: ID:0 name:Manufacturer type:2 minInstances:0 maxInstances:1 operations:2
       Resource: ID:1 name:ModelNumber type:2 minInstances:0 maxInstances:1 operations:2
       Resource: ID:2 name:SerialNumber type:2 minInstances:0 maxInstances:1 operations:2
       Resource: ID:3 name:FirmwareVersion type:2 minInstances:0 maxInstances:1 operations:2
       Resource: ID:4 name:Reboot type:1 minInstances:1 maxInstances:1 operations:5
       Resource: ID:5 name:FactoryReset type:1 minInstances:0 maxInstances:1 operations:5
       Resource: ID:6 name:AvailablePowerSources type:10 minInstances:0 maxInstances:65535 operations:2
       Resource: ID:7 name:PowerSourceVoltage type:10 minInstances:0 maxInstances:65535 operations:2
       Resource: ID:8 name:PowerSourceCurrent type:10 minInstances:0 maxInstances:65535 operations:2
       Resource: ID:9 name:BatteryLevel type:3 minInstances:0 maxInstances:1 operations:2
       Resource: ID:10 name:MemoryFree type:3 minInstances:0 maxInstances:1 operations:2
       Resource: ID:11 name:ErrorCode type:10 minInstances:1 maxInstances:65535 operations:2
       Resource: ID:12 name:ResetErrorCode type:1 minInstances:0 maxInstances:1 operations:5
       Resource: ID:13 name:CurrentTime type:7 minInstances:0 maxInstances:1 operations:4
       Resource: ID:14 name:UTCOffset type:2 minInstances:0 maxInstances:1 operations:4
       Resource: ID:15 name:Timezone type:2 minInstances:0 maxInstances:1 operations:4
       Resource: ID:16 name:SupportedBindingandModes type:2 minInstances:1 maxInstances:1 operations:2
       Resource: ID:17 name:DeviceType type:2 minInstances:0 maxInstances:1 operations:2
       Resource: ID:18 name:HardwareVersion type:2 minInstances:0 maxInstances:1 operations:2
       Resource: ID:19 name:SoftwareVersion type:2 minInstances:0 maxInstances:1 operations:2
       Resource: ID:20 name:BatteryStatus type:3 minInstances:0 maxInstances:1 operations:2
       Resource: ID:21 name:MemoryTotal type:3 minInstances:0 maxInstances:1 operations:2
Object ID:4 name:ConnectivityMonitoring minInstances:0 maxInstances:1
       Resource: ID:0 name:NetworkBearer type:3 minInstances:1 maxInstances:1 operations:2
       Resource: ID:1 name:AvailableNetworkBearer type:10 minInstances:1 maxInstances:65535 operations:2
       Resource: ID:2 name:RadioSignalStrength type:3 minInstances:1 maxInstances:1 operations:2
       Resource: ID:3 name:LinkQuality type:3 minInstances:0 maxInstances:1 operations:2
       Resource: ID:4 name:ADDRESSes type:9 minInstances:1 maxInstances:65535 operations:2
       Resource: ID:5 name:RouterADDRESSe type:9 minInstances:0 maxInstances:65535 operations:2
       Resource: ID:6 name:LinkUtilization type:3 minInstances:0 maxInstances:1 operations:2
       Resource: ID:7 name:APN type:9 minInstances:0 maxInstances:65535 operations:2
       Resource: ID:8 name:CellID type:3 minInstances:0 maxInstances:1 operations:2
       Resource: ID:9 name:SMNC type:3 minInstances:0 maxInstances:1 operations:2
       Resource: ID:10 name:SMCC type:3 minInstances:0 maxInstances:1 operations:2
Object ID:7 name:ConnectivityStatistics minInstances:0 maxInstances:1
       Resource: ID:0 name:SMSTxCounter type:3 minInstances:0 maxInstances:1 operations:2
       Resource: ID:1 name:SMSRxCounter type:3 minInstances:0 maxInstances:1 operations:2
       Resource: ID:2 name:TxData type:3 minInstances:0 maxInstances:1 operations:2
       Resource: ID:3 name:RxData type:3 minInstances:0 maxInstances:1 operations:2
       Resource: ID:4 name:MaxMessageSize type:3 minInstances:0 maxInstances:1 operations:2
       Resource: ID:5 name:AverageMessageSize type:3 minInstances:0 maxInstances:1 operations:2
       Resource: ID:6 name:StartOrReset type:1 minInstances:1 maxInstances:1 operations:5
Object ID:5 name:FirmwareUpdate minInstances:0 maxInstances:1
       Resource: ID:0 name:Package type:6 minInstances:1 maxInstances:1 operations:3
       Resource: ID:1 name:PackageURI type:2 minInstances:1 maxInstances:1 operations:3
       Resource: ID:2 name:Update type:1 minInstances:1 maxInstances:1 operations:5
       Resource: ID:3 name:State type:3 minInstances:1 maxInstances:1 operations:2
       Resource: ID:4 name:UpdateSupportedObjects type:5 minInstances:0 maxInstances:1 operations:4
       Resource: ID:5 name:UpdateResult type:3 minInstances:1 maxInstances:1 operations:2
Object ID:6 name:Location minInstances:0 maxInstances:1
       Resource: ID:0 name:Latitude type:2 minInstances:1 maxInstances:1 operations:2
       Resource: ID:1 name:Longitude type:2 minInstances:1 maxInstances:1 operations:2
       Resource: ID:2 name:Altitude type:2 minInstances:0 maxInstances:1 operations:2
       Resource: ID:3 name:Uncertainty type:2 minInstances:0 maxInstances:1 operations:2
       Resource: ID:4 name:Velocity type:6 minInstances:0 maxInstances:1 operations:2
       Resource: ID:5 name:Timestamp type:7 minInstances:1 maxInstances:1 operations:2
Object ID:1000 name:Object1000 minInstances:1 maxInstances:1
       Resource: ID:100 name:Resource100 type:2 minInstances:0 maxInstances:1 operations:4
       Resource: ID:101 name:Resource101 type:3 minInstances:0 maxInstances:1 operations:4
       Resource: ID:102 name:Resource102 type:4 minInstances:0 maxInstances:1 operations:4
       Resource: ID:103 name:Resource103 type:5 minInstances:0 maxInstances:1 operations:4
       Resource: ID:104 name:Resource104 type:7 minInstances:0 maxInstances:1 operations:4
       Resource: ID:105 name:Resource105 type:6 minInstances:0 maxInstances:1 operations:4
       Resource: ID:106 name:Resource106 type:8 minInstances:0 maxInstances:1 operations:4
       Resource: ID:107 name:Resource107 type:1 minInstances:0 maxInstances:1 operations:5
       Resource: ID:200 name:Resource200 type:9 minInstances:0 maxInstances:65535 operations:4
       Resource: ID:201 name:Resource201 type:10 minInstances:0 maxInstances:65535 operations:4
       Resource: ID:202 name:Resource202 type:11 minInstances:0 maxInstances:65535 operations:4
       Resource: ID:203 name:Resource203 type:12 minInstances:0 maxInstances:65535 operations:4
       Resource: ID:204 name:Resource204 type:14 minInstances:0 maxInstances:65535 operations:4
       Resource: ID:205 name:Resource205 type:13 minInstances:0 maxInstances:65535 operations:4
       Resource: ID:206 name:Resource206 type:15 minInstances:0 maxInstances:65535 operations:4
Object ID:2000 name:Object2000 minInstances:1 maxInstances:65535
       Resource: ID:100 name:Resource100 type:2 minInstances:0 maxInstances:1 operations:4
       Resource: ID:101 name:Resource101 type:3 minInstances:0 maxInstances:1 operations:4
       Resource: ID:102 name:Resource102 type:4 minInstances:0 maxInstances:1 operations:4
       Resource: ID:103 name:Resource103 type:5 minInstances:0 maxInstances:1 operations:4
       Resource: ID:104 name:Resource104 type:7 minInstances:0 maxInstances:1 operations:4
       Resource: ID:105 name:Resource105 type:6 minInstances:0 maxInstances:1 operations:4
       Resource: ID:106 name:Resource106 type:8 minInstances:0 maxInstances:1 operations:4
       Resource: ID:107 name:Resource107 type:1 minInstances:0 maxInstances:1 operations:5
       Resource: ID:200 name:Resource200 type:9 minInstances:0 maxInstances:65535 operations:4
       Resource: ID:201 name:Resource201 type:10 minInstances:0 maxInstances:65535 operations:4
       Resource: ID:202 name:Resource202 type:11 minInstances:0 maxInstances:65535 operations:4
       Resource: ID:203 name:Resource203 type:12 minInstances:0 maxInstances:65535 operations:4
       Resource: ID:204 name:Resource204 type:14 minInstances:0 maxInstances:65535 operations:4
       Resource: ID:205 name:Resource205 type:13 minInstances:0 maxInstances:65535 operations:4
       Resource: ID:206 name:Resource206 type:15 minInstances:0 maxInstances:65535 operations:4
"""
        expectedStderr = ""
        expectedCode = 0

        result = client_explore(self.config, "")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)



    @unittest.skip("maintenance headache")
    def test_explore_object(self):
        expectedStdout = """Object ID:1 name:LWM2MServer minInstances:1 maxInstances:65535
       Resource: ID:0 name:ShortServerID type:3 minInstances:1 maxInstances:1 operations:2
       Resource: ID:1 name:Lifetime type:3 minInstances:1 maxInstances:1 operations:4
       Resource: ID:2 name:DefaultMinimumPeriod type:3 minInstances:0 maxInstances:1 operations:4
       Resource: ID:3 name:DefaultMaximumPeriod type:3 minInstances:0 maxInstances:1 operations:4
       Resource: ID:4 name:Disable type:1 minInstances:0 maxInstances:1 operations:5
       Resource: ID:5 name:DisableTimeout type:3 minInstances:0 maxInstances:1 operations:4
       Resource: ID:6 name:NotificationStoringWhenDisabledorOffline type:5 minInstances:1 maxInstances:1 operations:4
       Resource: ID:7 name:Binding type:2 minInstances:1 maxInstances:1 operations:4
       Resource: ID:8 name:RegistrationUpdateTrigger type:1 minInstances:1 maxInstances:1 operations:5
"""
        expectedStderr = ""
        expectedCode = 0
	
        result = client_explore(self.config, "/1")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    @unittest.skip("maintenance headache")
    def test_explore_multiple_object(self):
        expectedStdout = """Object ID:1 name:LWM2MServer minInstances:1 maxInstances:65535
       Resource: ID:0 name:ShortServerID type:3 minInstances:1 maxInstances:1 operations:2
       Resource: ID:1 name:Lifetime type:3 minInstances:1 maxInstances:1 operations:4
       Resource: ID:2 name:DefaultMinimumPeriod type:3 minInstances:0 maxInstances:1 operations:4
       Resource: ID:3 name:DefaultMaximumPeriod type:3 minInstances:0 maxInstances:1 operations:4
       Resource: ID:4 name:Disable type:1 minInstances:0 maxInstances:1 operations:5
       Resource: ID:5 name:DisableTimeout type:3 minInstances:0 maxInstances:1 operations:4
       Resource: ID:6 name:NotificationStoringWhenDisabledorOffline type:5 minInstances:1 maxInstances:1 operations:4
       Resource: ID:7 name:Binding type:2 minInstances:1 maxInstances:1 operations:4
       Resource: ID:8 name:RegistrationUpdateTrigger type:1 minInstances:1 maxInstances:1 operations:5
Object ID:2 name:LWM2MAccessControl minInstances:0 maxInstances:65535
       Resource: ID:0 name:ObjectID type:3 minInstances:1 maxInstances:1 operations:2
       Resource: ID:1 name:ObjectInstanceID type:3 minInstances:1 maxInstances:1 operations:2
       Resource: ID:2 name:ACL type:10 minInstances:0 maxInstances:65535 operations:4
       Resource: ID:3 name:AccessControlOwner type:3 minInstances:1 maxInstances:1 operations:4
"""
        expectedStderr = ""
        expectedCode = 0

        result = client_explore(self.config, "/1 /2")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    @unittest.skip("maintenance headache")
    def test_explore_object_instance(self):
        expectedStdout = """Object ID:1 name:LWM2MServer minInstances:1 maxInstances:65535
       Resource: ID:0 name:ShortServerID type:3 minInstances:1 maxInstances:1 operations:2
       Resource: ID:1 name:Lifetime type:3 minInstances:1 maxInstances:1 operations:4
       Resource: ID:2 name:DefaultMinimumPeriod type:3 minInstances:0 maxInstances:1 operations:4
       Resource: ID:3 name:DefaultMaximumPeriod type:3 minInstances:0 maxInstances:1 operations:4
       Resource: ID:4 name:Disable type:1 minInstances:0 maxInstances:1 operations:5
       Resource: ID:5 name:DisableTimeout type:3 minInstances:0 maxInstances:1 operations:4
       Resource: ID:6 name:NotificationStoringWhenDisabledorOffline type:5 minInstances:1 maxInstances:1 operations:4
       Resource: ID:7 name:Binding type:2 minInstances:1 maxInstances:1 operations:4
       Resource: ID:8 name:RegistrationUpdateTrigger type:1 minInstances:1 maxInstances:1 operations:5
"""
        expectedStderr = ""
        expectedCode = 0

        result = client_explore(self.config, "/1/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)


    @unittest.skip("maintenance headache")
    def test_explore_resource(self):
        expectedStdout = """Object ID:3 name:Device minInstances:1 maxInstances:1
       Resource: ID:0 name:Manufacturer type:2 minInstances:0 maxInstances:1 operations:2
"""
        expectedStderr = ""
        expectedCode = 0

        result = client_explore(self.config, "/3/0/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    @unittest.skip("maintenance headache")
    def test_explore_multiple_resources(self):
        expectedStdout = """Object ID:3 name:Device minInstances:1 maxInstances:1
       Resource: ID:0 name:Manufacturer type:2 minInstances:0 maxInstances:1 operations:2
Object ID:4 name:ConnectivityMonitoring minInstances:0 maxInstances:1
       Resource: ID:1 name:AvailableNetworkBearer type:10 minInstances:1 maxInstances:65535 operations:2
"""
        expectedStderr = ""
        expectedCode = 0

        result = client_explore(self.config, "/3/0/0 /4/0/1")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    @unittest.skip("maintenance headache")
    def test_explore_non_exsistence_object(self):
        expectedStdout = "Object 23456 not defined\n"
        expectedStderr = ""
        expectedCode = 0

        result = client_explore(self.config, "/23456")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    @unittest.skip("maintenance headache")
    def test_explore_non_exsistence_resource(self):
        expectedStdout = """Object ID:3 name:Device minInstances:1 maxInstances:1
Resource 1111 not defined 
"""
        expectedStderr = ""
        expectedCode = 0

        result = client_explore(self.config, "/3/0/1111")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    @unittest.skip("maintenance headache")
    def test_explore_invalid_path(self):
        expectedStdout = ""
        expectedStderr = "Target /-1/0/0 is not valid\n"
        expectedCode = 0

        result = client_explore(self.config, "/-1/0/0")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    @unittest.skip("maintenance headache")
    def test_explore_invalid_object(self):
        expectedStdout = ""
        expectedStderr = "Target /@@adf%%% is not valid\n"
        expectedCode = 0

        result = client_explore(self.config, "/@@adf%%%")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

    @unittest.skip("maintenance headache")
    def test_explore_invalid_resource(self):
        expectedStdout = ""
        expectedStderr = "Target /3/0/%@%$ is not valid\n"
        expectedCode = 0

        result = client_explore(self.config, "/3/0/%@%$")
        self.assertEqual(expectedStdout, result.stdout)
        self.assertEqual(expectedStderr, result.stderr)
        self.assertEqual(expectedCode,   result.code)

class TestExploreBasic(tools_common.BasicTestCase):

    @unittest.skip("maintenance headache")
    def test_get_help(self):
        expectedStdout = """awa-client-explore 1.0

LWM2M Client Objects/Resource Explore Operation

Usage: awa-client-explore [OPTIONS]... [PATHS]...

  -h, --help                  Print help and exit
  -V, --version               Print version and exit
  -v, --verbose               Increase program verbosity  (default=off)
  -d, --debug                 Increase program verbosity  (default=off)
  -a, --ipcAddress=ADDRESS  Connect to Client IPC Address
                                (default=`127.0.0.1')
  -p, --ipcPort=PORT          Connect to Client IPC port  (default=`12345')
  -q, --quiet                 Print values only (quiet)  (default=off)

Specify one or more object, object instance and resource paths
in the format "/O/I/R", separated by spaces. For example:

   /3 /4 /4/0/7 /5/0/0 /5

"""
        expectedStderr = ""
        expectedCode = 0
        helpOptions = ("--help","-h")

        for option in helpOptions:
            code, stdout, stderr = client_explore(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)

    @unittest.skip("maintenance headache")
    def test_get_version(self):
        expectedStdout = "awa-client-explore 1.0\n"
        expectedStderr = ""
        expectedCode = 0
        versionOptions = ("--version","-V")

        for option in versionOptions:
            code, stdout, stderr = client_explore(self.config, option)
            self.assertEqual(expectedStdout, stdout)
            self.assertEqual(expectedStderr, stderr)
            self.assertEqual(expectedCode, code)
