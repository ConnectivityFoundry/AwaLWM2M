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

import unittest

import xml_test_case

import ipc_core
import ipc_lwm2m_client as client


################################################################################
# REQUEST tests
################################################################################
class TestLWM2MClientIpcRequests(xml_test_case.XmlTestCase):

# SET REQUEST tests

    def test_set_request_resource_instance(self):
        request = client.SetRequest()
        request.add((1, 2, 3, 4), "VALUE")
        expectedXml = """
        <Request>
          <Type>Set</Type>
          <Content>
            <Objects>
              <Object>
                <ID>1</ID>
                <ObjectInstance>
                  <ID>2</ID>
                  <Resource>
                    <ID>3</ID>
                    <ResourceInstance>
                      <ID>4</ID>
                      <Value>VALUE</Value>
                    </ResourceInstance>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())


    def test_set_request_multiple_resource_instances(self):
        request = client.SetRequest()
        request.add((0, 0, 0, 1), "VALUE1")
        request.add((0, 0, 0, 2), "VALUE2")
        request.add((0, 0, 0, 3), "VALUE3")
        expectedXml = """
        <Request>
          <Type>Set</Type>
          <Content>
            <Objects>
              <Object>
                <ID>0</ID>
                <ObjectInstance>
                  <ID>0</ID>
                  <Resource>
                    <ID>0</ID>
                    <ResourceInstance>
                      <ID>1</ID>
                      <Value>VALUE1</Value>
                    </ResourceInstance>
                    <ResourceInstance>
                      <ID>2</ID>
                      <Value>VALUE2</Value>
                    </ResourceInstance>
                    <ResourceInstance>
                      <ID>3</ID>
                      <Value>VALUE3</Value>
                    </ResourceInstance>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_set_request_single_resource(self):
        # resourceID must be zero
        request = client.SetRequest()
        request.add((1, 2, 3), "VALUE")
        expectedXml = """
        <Request>
          <Type>Set</Type>
          <Content>
            <Objects>
              <Object>
                <ID>1</ID>
                <ObjectInstance>
                  <ID>2</ID>
                  <Resource>
                    <ID>3</ID>
                    <Value>VALUE</Value>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Request>
        """
        #print(str(request._content._model))
        self.assertEqualXml(expectedXml, request.serialize())

    def test_set_request_multiple_resources(self):
        request = client.SetRequest()
        request.add((3, 0, 0), "VALUE1")
        request.add((3, 0, 1), "VALUE2")
        expectedXml = """
        <Request>
          <Type>Set</Type>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
                <ObjectInstance>
                  <ID>0</ID>
                  <Resource>
                    <ID>0</ID>
                    <Value>VALUE1</Value>
                  </Resource>
                  <Resource>
                    <ID>1</ID>
                    <Value>VALUE2</Value>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_set_request_multiple_object_instances(self):
        request = client.SetRequest()
        request.add((3, 0, 0), "VALUE1")
        request.add((3, 1, 0), "VALUE2")
        expectedXml = """
        <Request>
          <Type>Set</Type>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
                <ObjectInstance>
                  <ID>0</ID>
                  <Resource>
                    <ID>0</ID>
                    <Value>VALUE1</Value>
                  </Resource>
                </ObjectInstance>
                <ObjectInstance>
                  <ID>1</ID>
                  <Resource>
                    <ID>0</ID>
                    <Value>VALUE2</Value>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_set_request_complex(self):
        request = client.SetRequest()
        request.add((7, 0, 0), "VALUE1")
        request.add((7, 0, 1), "VALUE2")
        request.add((7, 1, 0), "VALUE3")
        request.add((7, 1, 9, 0), "VALUE4")
        request.add((7, 8, 0), "VALUE5")
        expectedXml = """
        <Request>
          <Type>Set</Type>
          <Content>
            <Objects>
              <Object>
                <ID>7</ID>
                <ObjectInstance>
                  <ID>0</ID>
                  <Resource>
                    <ID>0</ID>
                    <Value>VALUE1</Value>
                  </Resource>
                  <Resource>
                    <ID>1</ID>
                    <Value>VALUE2</Value>
                  </Resource>
                </ObjectInstance>
                <ObjectInstance>
                  <ID>1</ID>
                  <Resource>
                    <ID>0</ID>
                    <Value>VALUE3</Value>
                  </Resource>
                  <Resource>
                    <ID>9</ID>
                    <ResourceInstance>
                      <ID>0</ID>
                      <Value>VALUE4</Value>
                    </ResourceInstance>
                  </Resource>
                </ObjectInstance>
                <ObjectInstance>
                  <ID>8</ID>
                  <Resource>
                    <ID>0</ID>
                    <Value>VALUE5</Value>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_set_request_multiple_objects(self):
        request = client.SetRequest()
        request.add((0, 0, 0), "VALUE1")
        request.add((1, 0, 0), "VALUE2")
        expectedXml = """
        <Request>
          <Type>Set</Type>
          <Content>
            <Objects>
              <Object>
                <ID>0</ID>
                <ObjectInstance>
                  <ID>0</ID>
                  <Resource>
                    <ID>0</ID>
                    <Value>VALUE1</Value>
                  </Resource>
                </ObjectInstance>
              </Object>
              <Object>
                <ID>1</ID>
                <ObjectInstance>
                  <ID>0</ID>
                  <Resource>
                    <ID>0</ID>
                    <Value>VALUE2</Value>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_set_request_invalid_path(self):
        request = client.SetRequest()
        print request
        with self.assertRaises(ipc_core.IpcError) as cm:
            request.add((0, 0), "VALUE1")
        self.assertEqual("Unsupported path", cm.exception.message)

        with self.assertRaises(ipc_core.IpcError) as cm:
            request.add((0,), "VALUE2")
        self.assertEqual("Unsupported path", cm.exception.message)

        with self.assertRaises(ipc_core.IpcError) as cm:
            request.add(None, "VALUE3")
        self.assertEqual("Unsupported path", cm.exception.message)

# GET REQUEST tests

    def test_get_request_single_resource_instance(self):
        request = client.GetRequest()
        request.add((0, 0, 0, 0))
        expectedXml = """
        <Request>
          <Type>Get</Type>
          <Content>
            <Objects>
              <Object>
                <ID>0</ID>
                <ObjectInstance>
                  <ID>0</ID>
                  <Resource>
                    <ID>0</ID>
                    <ResourceInstance>
                      <ID>0</ID>
                    </ResourceInstance>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_get_request_single_resource(self):
        request = client.GetRequest()
        request.add((1, 2, 3))
        expectedXml = """
        <Request>
          <Type>Get</Type>
          <Content>
            <Objects>
              <Object>
                <ID>1</ID>
                <ObjectInstance>
                  <ID>2</ID>
                  <Resource>
                    <ID>3</ID>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_get_request_two_resources(self):
        request = client.GetRequest()
        request.add((1, 2, 3))
        expectedXml = """
        <Request>
          <Type>Get</Type>
          <Content>
            <Objects>
              <Object>
                <ID>1</ID>
                <ObjectInstance>
                  <ID>2</ID>
                  <Resource>
                    <ID>3</ID>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_get_request_single_instance(self):
        request = client.GetRequest()
        request.add((7, 4))
        expectedXml = """
        <Request>
          <Type>Get</Type>
          <Content>
            <Objects>
              <Object>
                <ID>7</ID>
                <ObjectInstance>
                  <ID>4</ID>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_get_request_single_object(self):
        request = client.GetRequest()
        request.add((3,))
        expectedXml = """
        <Request>
          <Type>Get</Type>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
              </Object>
            </Objects>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())


# SUBSCRIBE-TO-EXECUTE REQUEST tests

    def test_subscribe_to_execute_request_single_resource(self):
        request = client.SubscribeToExecuteRequest()
        request.add((3, 0, 4))
        expectedXml = """
        <Request>
          <Type>Subscribe</Type>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
                <ObjectInstance>
                  <ID>0</ID>
                  <Resource>
                    <ID>4</ID>
                    <SubscribeToExecute/>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_subscribe_to_execute_request_invalid(self):
        request = client.SubscribeToExecuteRequest()
        with self.assertRaises(ipc_core.IpcError) as cm:
            request.add((3, 0))
        self.assertEqual("Unsupported path", cm.exception.message)
        with self.assertRaises(ipc_core.IpcError) as cm:
            request.add((3,))
        self.assertEqual("Unsupported path", cm.exception.message)
        with self.assertRaises(ipc_core.IpcError) as cm:
            request.add(None)
        self.assertEqual("Unsupported path", cm.exception.message)


# SUBSCRIBE-TO-CHANGE REQUEST tests

    def test_subscribe_to_change_request_single_resource(self):
        request = client.SubscribeToChangeRequest()
        request.add((3, 2, 1))
        expectedXml = """
        <Request>
          <Type>Subscribe</Type>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
                <ObjectInstance>
                  <ID>2</ID>
                  <Resource>
                    <ID>1</ID>
                    <SubscribeToChange/>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_subscribe_to_change_request_single_object_instance(self):
        request = client.SubscribeToChangeRequest()
        request.add((3, 0))
        expectedXml = """
        <Request>
          <Type>Subscribe</Type>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
                <ObjectInstance>
                  <ID>0</ID>
                  <SubscribeToChange/>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_subscribe_to_change_request_object(self):
        # object-targeted change subscriptions are not supported
        request = client.SubscribeToChangeRequest()
        with self.assertRaises(ipc_core.IpcError) as cm:
            request.add((3,))
        self.assertEqual("Unsupported path", cm.exception.message)

# CANCEL SUBSCRIBE TOC CHANGE REQUEST tests

    def test_unsubscribe_request_single_resource(self):
        request = client.CancelSubscribeToChangeRequest()
        request.add((3, 2, 1))
        expectedXml = """
        <Request>
          <Type>Subscribe</Type>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
                <ObjectInstance>
                  <ID>2</ID>
                  <Resource>
                    <ID>1</ID>
                    <CancelSubscribeToChange/>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_cancel_subscribe_to_change_request_multiple_resources(self):
        request = client.CancelSubscribeToChangeRequest()
        request.add((3, 2, 1))
        request.add((3, 2, 2))
        request.add((3, 3, 1))
        expectedXml = """
        <Request>
          <Type>Subscribe</Type>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
                <ObjectInstance>
                  <ID>2</ID>
                  <Resource>
                    <ID>1</ID>
                    <CancelSubscribeToChange/>
                  </Resource>
                  <Resource>
                    <ID>2</ID>
                    <CancelSubscribeToChange/>
                  </Resource>
                </ObjectInstance>
                <ObjectInstance>
                  <ID>3</ID>
                  <Resource>
                    <ID>1</ID>
                    <CancelSubscribeToChange/>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_cancel_subscribe_to_change_request_single_object_instance(self):
        request = client.CancelSubscribeToChangeRequest()
        request.add((3, 2))
        expectedXml = """
        <Request>
          <Type>Subscribe</Type>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
                <ObjectInstance>
                  <ID>2</ID>
                  <CancelSubscribeToChange/>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_cancel_subscribe_to_change_request_invalid(self):
        # object-targeted unsubscriptions are not supported
        request = client.CancelSubscribeToChangeRequest()
        with self.assertRaises(ipc_core.IpcError) as cm:
            request.add((3,))
        self.assertEqual("Unsupported path", cm.exception.message)

    def test_cancel_subscribe_to_execute_request_invalid(self):
        # object- and object-instance- targeted unsubscriptions are not supported
        request = client.CancelSubscribeToExecuteRequest()
        with self.assertRaises(ipc_core.IpcError) as cm:
            request.add((3,))
        self.assertEqual("Unsupported path", cm.exception.message)
        with self.assertRaises(ipc_core.IpcError) as cm:
            request.add((3,0))
        self.assertEqual("Unsupported path", cm.exception.message)

    def test_delete_request_resource(self):
        request = client.DeleteRequest()
        request.add((3, 2, 1))
        expectedXml = """
        <Request>
          <Type>Delete</Type>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
                <ObjectInstance>
                  <ID>2</ID>
                  <Resource>
                    <ID>1</ID>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_delete_request_object_instance(self):
        request = client.DeleteRequest()
        request.add((3, 2))
        expectedXml = """
        <Request>
          <Type>Delete</Type>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
                <ObjectInstance>
                  <ID>2</ID>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_delete_request_object(self):
        request = client.DeleteRequest()
        with self.assertRaises(ipc_core.IpcError) as cm:
            request.add((3,))
        self.assertEqual("Unsupported path", cm.exception.message)




################################################################################
# RESPONSE tests
################################################################################
class TestLWM2MClientIpcResponses(unittest.TestCase):

# SET RESPONSE tests

    def test_set_response(self):
        responseXml = """
        <Response>
          <Type>Set</Type>
          <Code>200</Code>
        </Response>
        """
        response = client.SetResponse(responseXml)
        self.assertEqual("Set", response.type)
        self.assertEqual("200", response.code)


# GET RESPONSE tests

    def test_get_response(self):
        responseXml = """
        <Response>
          <Type>Get</Type>
          <Code>200</Code>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
                <ObjectInstance>
                  <ID>0</ID>
                  <Resource>
                    <ID>0</ID>
                    <Value>YWJj</Value>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Response>
        """
        response = client.GetResponse(responseXml)
        print(response); print(response.serialize())
        self.assertEqual("Get", response.type)
        self.assertEqual("200", response.code)

        self.assertEqual("YWJj", response.getValue((3, 0, 0)))
        self.assertEqual(None, response.getValue((3, 0, 0, 1)))
        self.assertEqual(None, response.getValue((3, 0, 1, 0)))
        self.assertEqual(None, response.getValue((3, 1, 0, 0)))
        self.assertEqual(None, response.getValue((4, 0, 0, 0)))

    def test_get_response_simple(self):
        #Test the simpler version of the schema, for single entities
        responseXml = """
        <Response>
          <Type>Get</Type>
          <Code>401</Code>
          <Content>
            <Objects>
              <Object>
              <ID>3</ID>
                <ObjectInstance>
                  <ID>9</ID>
                  <Resource>
                    <ID>42</ID>
                    <ResourceInstance>
                      <ID>17</ID>
                      <Value>YWJj</Value>
                    </ResourceInstance>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Response>
        """
        response = client.GetResponse(responseXml)
        self.assertEqual("Get", response.type)
        self.assertEqual("401", response.code)

        self.assertEqual("YWJj", response.getValue((3, 9, 42, 17)))
        self.assertEqual(None, response.getValue((3, 9, 42, 18)))

    def test_get_response_multiple_resource_instances(self):
        responseXml = """
        <Response>
          <Type>Get</Type>
          <Code>500</Code>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
                <ObjectInstance>
                  <ID>0</ID>
                  <Resource>
                    <ID>0</ID>
                    <ResourceInstance>
                      <ID>0</ID>
                      <Value>YWJj</Value>
                    </ResourceInstance>
                    <ResourceInstance>
                      <ID>1</ID>
                      <Value>ABCD</Value>
                    </ResourceInstance>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Response>
        """
        response = client.GetResponse(responseXml)
        self.assertEqual("Get", response.type)
        self.assertEqual("500", response.code)

        self.assertEqual("YWJj", response.getValue((3, 0, 0, 0)))
        self.assertEqual("ABCD", response.getValue((3, 0, 0, 1)))
        self.assertEqual(None, response.getValue((3, 0, 0, 2)))

    def test_get_response_multiple_resources(self):
        responseXml = """
        <Response>
          <Type>Get</Type>
          <Code>200</Code>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
                <ObjectInstance>
                  <ID>0</ID>
                  <Resource>
                    <ID>0</ID>
                    <Value>YWJj</Value>
                  </Resource>
                  <Resource>
                    <ID>1</ID>
                    <ResourceInstance>
                      <ID>0</ID>
                      <Value>ASDF</Value>
                    </ResourceInstance>
                    <ResourceInstance>
                      <ID>1</ID>
                      <Value>ZXCV</Value>
                    </ResourceInstance>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Response>
        """
        response = client.GetResponse(responseXml)
        self.assertEqual("Get", response.type)
        self.assertEqual("200", response.code)

        self.assertEqual("YWJj", response.getValue((3, 0, 0)))
        self.assertEqual("ASDF", response.getValue((3, 0, 1, 0)))
        self.assertEqual("ZXCV", response.getValue((3, 0, 1, 1)))
        self.assertEqual(None, response.getValue((3, 0, 1, 2)))
        self.assertEqual(None, response.getValue((3, 0, 2)))

    def test_get_response_multiple_object_instances(self):
        responseXml = """
        <Response>
          <Type>Get</Type>
          <Code>0</Code>
          <Content>
            <Objects>
              <Object>
                <ID>19</ID>
                <ObjectInstance>
                  <ID>0</ID>
                  <Resource>
                    <ID>0</ID>
                    <Value>TYUIZ</Value>
                  </Resource>
                  <Resource>
                    <ID>1</ID>
                    <Value>QWERTY</Value>
                  </Resource>
                </ObjectInstance>
                <ObjectInstance>
                  <ID>5</ID>
                  <Resource>
                    <ID>2</ID>
                    <ResourceInstance>
                      <ID>1</ID>
                      <Value>YWJj</Value>
                    </ResourceInstance>
                  </Resource>
                  <Resource>
                    <ID>99</ID>
                    <ResourceInstance>
                      <ID>42</ID>
                      <Value>ASDF</Value>
                    </ResourceInstance>
                    <ResourceInstance>
                      <ID>44</ID>
                      <Value>ZXCV</Value>
                    </ResourceInstance>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Response>
        """
        response = client.GetResponse(responseXml)
        self.assertEqual("Get", response.type)
        self.assertEqual("0", response.code)

        self.assertEqual("TYUIZ", response.getValue((19, 0, 0)))
        self.assertEqual("QWERTY", response.getValue((19, 0, 1)))
        self.assertEqual(None, response.getValue((19, 0, 2)))
        self.assertEqual(None, response.getValue((19, 5, 0)))
        self.assertEqual("YWJj", response.getValue((19, 5, 2, 1)))
        self.assertEqual("ASDF", response.getValue((19, 5, 99, 42)))
        self.assertEqual("ZXCV", response.getValue((19, 5, 99, 44)))
        self.assertEqual(None, response.getValue((19, 5, 99, 43)))
        self.assertEqual(None, response.getValue((19, 5, 100, 0)))

    def test_get_response_multiple_objects(self):
        responseXml = """
        <Response>
          <Type>Get</Type>
          <Code>200</Code>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
                <ObjectInstance>
                  <ID>2</ID>
                  <Resource>
                    <ID>1</ID>
                    <ResourceInstance>
                      <ID>0</ID>
                      <Value>QWERTY</Value>
                    </ResourceInstance>
                  </Resource>
                </ObjectInstance>
              </Object>
              <Object>
                <ID>4</ID>
                <ObjectInstance>
                  <ID>5</ID>
                  <Resource>
                    <ID>6</ID>
                    <ResourceInstance>
                      <ID>7</ID>
                      <Value>ASDFG</Value>
                    </ResourceInstance>
                  </Resource>
                </ObjectInstance>
              </Object>
              <Object>
                <ID>33</ID>
                <ObjectInstance>
                  <ID>44</ID>
                  <Resource>
                    <ID>55</ID>
                    <ResourceInstance>
                      <ID>66</ID>
                      <Value>TYUIO</Value>
                    </ResourceInstance>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Response>
        """
        response = client.GetResponse(responseXml)
        self.assertEqual("Get", response.type)
        self.assertEqual("200", response.code)

        self.assertEqual("QWERTY", response.getValue((3, 2, 1, 0)))
        self.assertEqual(None, response.getValue((2, 2, 1, 0)))
        self.assertEqual("ASDFG", response.getValue((4, 5, 6, 7)))
        self.assertEqual(None, response.getValue((5, 5, 6, 7)))
        self.assertEqual("TYUIO", response.getValue((33, 44, 55, 66)))
        self.assertEqual(None, response.getValue((32, 44, 55, 66)))


    def test_subscribe_to_execute_response(self):
        responseXml = """
        <Response>
          <Type>Subscribe</Type>
          <Code>223</Code>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
                <ObjectInstance>
                  <ID>0</ID>
                  <Resource>
                    <ID>4</ID>
                    <SubscribeToExecute/>
                    <Result>
                      <Error>Success</Error>
                    </Result>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Response>
        """
        response = client.SubscribeToExecuteResponse(responseXml)
        self.assertEqual("Subscribe", response.type)
        self.assertEqual("223", response.code)
        # TODO:
        #  - test response paths are exposed
        #  - test response results are exposed
        #  - test labels are exposed

    def test_subscribe_to_change_response(self):
        responseXml = """
        <Response>
          <Type>Subscribe</Type>
          <Code>223</Code>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
                <ObjectInstance>
                  <ID>0</ID>
                  <Resource>
                    <ID>4</ID>
                    <SubscribeToChange/>
                    <Result>
                      <Error>Success</Error>
                    </Result>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Response>
        """
        response = client.SubscribeToChangeResponse(responseXml)
        self.assertEqual("Subscribe", response.type)
        self.assertEqual("223", response.code)
        # TODO:
        #  - test response paths are exposed
        #  - test response results are exposed
        #  - test labels are exposed

    def test_cancel_subscribe_to_change_response(self):
        responseXml = """
        <Response>
          <Type>Subscribe</Type>
          <Code>200</Code>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
                <ObjectInstance>
                  <ID>0</ID>
                  <Resource>
                    <ID>4</ID>
                    <SubscribeToChange/>
                    <Result>
                      <Error>Success</Error>
                    </Result>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Response>
        """
        response = client.CancelSubscribeToChangeResponse(responseXml)
        self.assertEqual("Subscribe", response.type)
        self.assertEqual("200", response.code)
        # TODO:
        #  - test response paths are exposed
        #  - test response results are exposed
        #  - test labels are exposed

    def test_cancel_subscribe_to_execute_response(self):
        responseXml = """
        <Response>
          <Type>Subscribe</Type>
          <Code>200</Code>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
                <ObjectInstance>
                  <ID>0</ID>
                  <Resource>
                    <ID>4</ID>
                    <SubscribeToExecute/>
                    <Result>
                      <Error>Success</Error>
                    </Result>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Response>
        """
        response = client.CancelSubscribeToChangeResponse(responseXml)
        self.assertEqual("Subscribe", response.type)
        self.assertEqual("200", response.code)
        # TODO:
        #  - test response paths are exposed
        #  - test response results are exposed
        #  - test labels are exposed

    def test_delete_response(self):
        responseXml = """
        <Response>
          <Type>Delete</Type>
          <Code>234</Code>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
                <ObjectInstance>
                  <ID>0</ID>
                  <Resource>
                    <ID>4</ID>
                    <Result>
                      <Error>Success</Error>
                    </Result>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Response>
        """
        response = client.DeleteResponse(responseXml)
        self.assertEqual("Delete", response.type)
        self.assertEqual("234", response.code)
        # TODO:
        #  - test response paths are exposed
        #  - test response results are exposed



# INVALID RESPONSE tests

    def test_response_invalid_root(self):
        responseXml = """
        <NotResponse>
          <Type>Set</Type>
          <Code>200</Code>
        </NotResponse>
        """
        with self.assertRaises(ipc_core.IpcError) as cm:
            response = client.SetResponse(responseXml)
        self.assertEqual(cm.exception.message, "Invalid response")

    def test_response_invalid_type(self):
        responseXml = """
        <Response>
          <Type>NotSet</Type>
          <Code>200</Code>
        </Response>
        """
        with self.assertRaises(ipc_core.IpcError) as cm:
            response = client.SetResponse(responseXml)
        self.assertEqual(cm.exception.message, "Invalid message type")

    def test_response_missing_type(self):
        responseXml = """
        <Response>
          <Code>200</Code>
        </Response>
        """
        with self.assertRaises(ipc_core.IpcError) as cm:
            response = client.GetResponse(responseXml)
        self.assertEqual(cm.exception.message, "Missing message type")

    def test_response_missing_code(self):
        responseXml = """
        <Response>
          <Type>Set</Type>
        </Response>
        """
        with self.assertRaises(ipc_core.IpcError) as cm:
            response = client.SetResponse(responseXml)
        self.assertEqual(cm.exception.message, "Missing response code")



################################################################################
# NOTIFICATION tests
################################################################################
class TestLWM2MClientIpcNotifications(unittest.TestCase):

# EXECUTE NOTIFICATION tests

    def test_execute_notification(self):
        notificationXml = """
        <Response>
          <Type>ExecuteNotification</Type>
          <Code>557</Code>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
                <ObjectInstance>
                  <ID>0</ID>
                  <Resource>
                    <ID>4</ID>
                    <Parameter>
                      <Name>Param1</Name>
                      <Value>VALUE</Value>
                    </Parameter>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Response>
        """
        notification = client.ExecuteNotification(notificationXml)
        self.assertEqual("ExecuteNotification", notification.type)
        self.assertEqual("557", notification.code)
        # TODO:
        #  - test parameters

    def test_change_notification(self):
        notificationXml = """
        <Response>
          <Type>ChangeNotification</Type>
          <Code>non-numeric</Code>
          <Content>
            <Objects>
              <Object>
                <ID>3</ID>
                <ObjectInstance>
                  <ID>0</ID>
                  <Resource>
                    <ID>0</ID>
                    <ChangeType>Modified</ChangeType>
                    <Value>VALUE1</Value>
                  </Resource>
                  <Resource>
                    <ID>1</ID>
                    <ChangeType>Deleted</ChangeType>
                  </Resource>
                  <Resource>
                    <ID>0</ID>
                    <ChangeType>Created</ChangeType>
                    <Value>VALUE2</Value>
                  </Resource>
                </ObjectInstance>
              </Object>
            </Objects>
          </Content>
        </Response>
        """
        notification = client.ChangeNotification(notificationXml)
        self.assertEqual("ChangeNotification", notification.type)
        self.assertEqual("non-numeric", notification.code)
        # TODO:
        #  - test changetype label
        #  - test modified has value
        #  - test created has value
