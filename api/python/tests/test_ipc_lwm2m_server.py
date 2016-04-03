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
import ipc_lwm2m_server as server


################################################################################
# REQUEST tests
################################################################################
class TestLWM2MServerIpcRequests(xml_test_case.XmlTestCase):

# GET_CLIENT_LIST REQUEST tests

    def test_get_client_list_request_simple(self):
        request = server.ListClientsRequest()
        expectedXml = """
        <Request>
          <Type>ListClients</Type>
        </Request>
        """
        #print(request); print(request.serialize())
        self.assertEqualXml(expectedXml, request.serialize())


# WRITE REQUEST tests

    @unittest.skip("test multiple clients")
    def test_write_request_multiple_clients(self):
        pass

    def test_write_request_simple(self):
        request = server.WriteRequest()
        request.add(("TestClient1", (1, 2, 3, 4)), "VALUE0")
        expectedXml = """
        <Request>
          <Type>Write</Type>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
                <Objects>
                  <Object>
                    <ID>1</ID>
                    <ObjectInstance>
                      <ID>2</ID>
                      <Resource>
                        <ID>3</ID>
                        <ResourceInstance>
                          <ID>4</ID>
                          <Value>VALUE0</Value>
                        </ResourceInstance>
                      </Resource>
                    </ObjectInstance>
                  </Object>
                </Objects>
              </Client>
            </Clients>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_write_request_multiple_resource_instances(self):
        request = server.WriteRequest()
        request.add(("TestClient1", (1, 2, 3, 4)), "VALUE0")
        request.add(("TestClient1", (1, 2, 3, 5)), "VALUE1")
        request.add(("TestClient1", (1, 2, 3, 9)), "VALUE2")
        expectedXml = """
        <Request>
          <Type>Write</Type>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
                <Objects>
                  <Object>
                    <ID>1</ID>
                    <ObjectInstance>
                      <ID>2</ID>
                      <Resource>
                        <ID>3</ID>
                        <ResourceInstance>
                          <ID>4</ID>
                          <Value>VALUE0</Value>
                        </ResourceInstance>
                        <ResourceInstance>
                          <ID>5</ID>
                          <Value>VALUE1</Value>
                        </ResourceInstance>
                        <ResourceInstance>
                          <ID>9</ID>
                          <Value>VALUE2</Value>
                        </ResourceInstance>
                      </Resource>
                    </ObjectInstance>
                  </Object>
                </Objects>
              </Client>
            </Clients>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_write_request_multiple_resources(self):
        request = server.WriteRequest()
        request.add(("TestClient1", (1, 2, 3, 0)), "VALUE0")
        request.add(("TestClient1", (1, 2, 4, 1)), "VALUE1")
        request.add(("TestClient1", (1, 2, 17, 2)), "VALUE2")
        expectedXml = """
        <Request>
          <Type>Write</Type>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
                <Objects>
                  <Object>
                    <ID>1</ID>
                    <ObjectInstance>
                      <ID>2</ID>
                      <Resource>
                        <ID>3</ID>
                        <ResourceInstance>
                          <ID>0</ID>
                          <Value>VALUE0</Value>
                        </ResourceInstance>
                      </Resource>
                      <Resource>
                        <ID>4</ID>
                        <ResourceInstance>
                          <ID>1</ID>
                          <Value>VALUE1</Value>
                        </ResourceInstance>
                      </Resource>
                      <Resource>
                        <ID>17</ID>
                        <ResourceInstance>
                          <ID>2</ID>
                          <Value>VALUE2</Value>
                        </ResourceInstance>
                      </Resource>
                    </ObjectInstance>
                  </Object>
                </Objects>
              </Client>
            </Clients>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_write_request_multiple_object_instances(self):
        request = server.WriteRequest()
        request.add(("TestClient1", (1, 0, 3, 0)), "VALUE0")
        request.add(("TestClient1", (1, 7, 4, 1)), "VALUE1")
        request.add(("TestClient1", (1, 8, 17, 2)), "VALUE2")
        expectedXml = """
        <Request>
          <Type>Write</Type>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
                <Objects>
                  <Object>
                    <ID>1</ID>
                    <ObjectInstance>
                      <ID>0</ID>
                      <Resource>
                        <ID>3</ID>
                        <ResourceInstance>
                          <ID>0</ID>
                          <Value>VALUE0</Value>
                        </ResourceInstance>
                      </Resource>
                    </ObjectInstance>
                    <ObjectInstance>
                      <ID>7</ID>
                      <Resource>
                        <ID>4</ID>
                        <ResourceInstance>
                          <ID>1</ID>
                          <Value>VALUE1</Value>
                        </ResourceInstance>
                      </Resource>
                    </ObjectInstance>
                    <ObjectInstance>
                      <ID>8</ID>
                      <Resource>
                        <ID>17</ID>
                        <ResourceInstance>
                          <ID>2</ID>
                          <Value>VALUE2</Value>
                        </ResourceInstance>
                      </Resource>
                    </ObjectInstance>
                  </Object>
                </Objects>
              </Client>
            </Clients>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_write_request_multiple_objects(self):
        request = server.WriteRequest()
        request.add(("TestClient1", (1, 0, 3, 0)), "VALUE0")
        request.add(("TestClient1", (2, 7, 4, 1)), "VALUE1")
        request.add(("TestClient1", (10, 8, 17, 2)), "VALUE2")
        expectedXml = """
        <Request>
          <Type>Write</Type>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
                <Objects>
                  <Object>
                    <ID>1</ID>
                    <ObjectInstance>
                      <ID>0</ID>
                      <Resource>
                        <ID>3</ID>
                        <ResourceInstance>
                          <ID>0</ID>
                          <Value>VALUE0</Value>
                        </ResourceInstance>
                      </Resource>
                    </ObjectInstance>
                  </Object>
                  <Object>
                    <ID>2</ID>
                    <ObjectInstance>
                      <ID>7</ID>
                      <Resource>
                        <ID>4</ID>
                        <ResourceInstance>
                          <ID>1</ID>
                          <Value>VALUE1</Value>
                        </ResourceInstance>
                      </Resource>
                    </ObjectInstance>
                  </Object>
                  <Object>
                    <ID>10</ID>
                    <ObjectInstance>
                      <ID>8</ID>
                      <Resource>
                        <ID>17</ID>
                        <ResourceInstance>
                          <ID>2</ID>
                          <Value>VALUE2</Value>
                        </ResourceInstance>
                      </Resource>
                    </ObjectInstance>
                  </Object>
                </Objects>
              </Client>
            </Clients>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_write_request_complex(self):
        request = server.WriteRequest()
        request.add(("TestClient1", (10, 20, 30, 40)), "VALUE0")
        request.add(("TestClient1", (10, 20, 30, 41)), "VALUE1")
        request.add(("TestClient1", (10, 20, 31, 40)), "VALUE2")
        request.add(("TestClient1", (10, 20, 31, 41)), "VALUE3")
        request.add(("TestClient1", (10, 20, 31, 42)), "VALUE4")
        request.add(("TestClient1", (10, 21, 30, 40)), "VALUE5")
        request.add(("TestClient1", (10, 21, 31, 40)), "VALUE6")
        request.add(("TestClient1", (10, 21, 31, 41)), "VALUE7")
        request.add(("TestClient1", (11, 20, 30, 40)), "VALUE8")
        request.add(("TestClient1", (11, 20, 30, 41)), "VALUE9")
        request.add(("TestClient1", (11, 21, 30, 40)), "VALUE10")
        request.add(("TestClient1", (11, 21, 31, 40)), "VALUE11")
        request.add(("TestClient1", (11, 21, 31, 41)), "VALUE12")
        expectedXml = """
        <Request>
          <Type>Write</Type>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
                <Objects>
                  <Object>
                    <ID>10</ID>
                    <ObjectInstance>
                      <ID>20</ID>
                      <Resource>
                        <ID>30</ID>
                        <ResourceInstance>
                          <ID>40</ID>
                          <Value>VALUE0</Value>
                        </ResourceInstance>
                        <ResourceInstance>
                          <ID>41</ID>
                          <Value>VALUE1</Value>
                        </ResourceInstance>
                      </Resource>
                      <Resource>
                        <ID>31</ID>
                        <ResourceInstance>
                          <ID>40</ID>
                          <Value>VALUE2</Value>
                        </ResourceInstance>
                        <ResourceInstance>
                          <ID>41</ID>
                          <Value>VALUE3</Value>
                        </ResourceInstance>
                        <ResourceInstance>
                          <ID>42</ID>
                          <Value>VALUE4</Value>
                        </ResourceInstance>
                      </Resource>
                    </ObjectInstance>
                    <ObjectInstance>
                      <ID>21</ID>
                      <Resource>
                        <ID>30</ID>
                        <ResourceInstance>
                          <ID>40</ID>
                          <Value>VALUE5</Value>
                        </ResourceInstance>
                      </Resource>
                      <Resource>
                        <ID>31</ID>
                        <ResourceInstance>
                          <ID>40</ID>
                          <Value>VALUE6</Value>
                        </ResourceInstance>
                        <ResourceInstance>
                          <ID>41</ID>
                          <Value>VALUE7</Value>
                        </ResourceInstance>
                      </Resource>
                    </ObjectInstance>
                  </Object>
                  <Object>
                    <ID>11</ID>
                    <ObjectInstance>
                      <ID>20</ID>
                      <Resource>
                        <ID>30</ID>
                        <ResourceInstance>
                          <ID>40</ID>
                          <Value>VALUE8</Value>
                        </ResourceInstance>
                        <ResourceInstance>
                          <ID>41</ID>
                          <Value>VALUE9</Value>
                        </ResourceInstance>
                      </Resource>
                    </ObjectInstance>
                    <ObjectInstance>
                      <ID>21</ID>
                      <Resource>
                        <ID>30</ID>
                        <ResourceInstance>
                          <ID>40</ID>
                          <Value>VALUE10</Value>
                        </ResourceInstance>
                      </Resource>
                      <Resource>
                        <ID>31</ID>
                        <ResourceInstance>
                          <ID>40</ID>
                          <Value>VALUE11</Value>
                        </ResourceInstance>
                        <ResourceInstance>
                          <ID>41</ID>
                          <Value>VALUE12</Value>
                        </ResourceInstance>
                      </Resource>
                    </ObjectInstance>
                  </Object>
                </Objects>
              </Client>
            </Clients>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())


# READ REQUEST tests

    @unittest.skip("test multiple clients")
    def test_read_request_multiple_clients(self):
        pass

    def test_read_request_resource_instance_fails(self):
        request = server.ReadRequest()
        with self.assertRaises(server.IpcError) as cm:
            request.add(("TestClient1", (1, 2, 3, 4)))
        self.assertEqual("Unsupported path", cm.exception.message)

    def test_read_request_resource(self):
        request = server.ReadRequest()
        request.add(("TestClient1", (1, 2, 3)))
        expectedXml = """
        <Request>
          <Type>Read</Type>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
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
              </Client>
            </Clients>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_read_request_object_instance(self):
        request = server.ReadRequest()
        request.add(("TestClient1", (1, 2)))
        expectedXml = """
        <Request>
          <Type>Read</Type>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
                <Objects>
                  <Object>
                    <ID>1</ID>
                    <ObjectInstance>
                      <ID>2</ID>
                    </ObjectInstance>
                  </Object>
                </Objects>
              </Client>
            </Clients>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_read_request_object(self):
        request = server.ReadRequest()
        request.add(("TestClient1", (1,)))
        #print(request); print(request.serialize())
        expectedXml = """
        <Request>
          <Type>Read</Type>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
                <Objects>
                  <Object>
                    <ID>1</ID>
                  </Object>
                </Objects>
              </Client>
            </Clients>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

# OBSERVE REQUEST tests

    @unittest.skip("test multiple clients")
    def test_observe_request_multiple_clients(self):
        pass

    def test_observe_request_resource_instance_fails(self):
        request = server.ObserveRequest()
        with self.assertRaises(server.IpcError) as cm:
            request.add(("TestClient1", (1, 2, 3, 4)))
        self.assertEqual("Unsupported path", cm.exception.message)

    def test_observe_request_resource(self):
        request = server.ObserveRequest()
        request.add(("TestClient1", (1, 2, 3)))
        expectedXml = """
        <Request>
          <Type>Observe</Type>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
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
              </Client>
            </Clients>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_observe_request_object_instance(self):
        request = server.ObserveRequest()
        request.add(("TestClient1", (1, 2)))
        expectedXml = """
        <Request>
          <Type>Observe</Type>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
                <Objects>
                  <Object>
                    <ID>1</ID>
                    <ObjectInstance>
                      <ID>2</ID>
                    </ObjectInstance>
                  </Object>
                </Objects>
              </Client>
            </Clients>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_observe_request_object(self):
        request = server.ObserveRequest()
        request.add(("TestClient1", (1,)))
        expectedXml = """
        <Request>
          <Type>Observe</Type>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
                <Objects>
                  <Object>
                    <ID>1</ID>
                  </Object>
                </Objects>
              </Client>
            </Clients>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

# CANCEL OBSERVE REQUEST tests

    def test_cancel_observe_request_resource_instance_fails(self):
        request = server.CancelObserveRequest()
        with self.assertRaises(server.IpcError) as cm:
            request.add(("TestClient1", (1, 2, 3, 4)))
        self.assertEqual("Unsupported path", cm.exception.message)

    def test_cancel_observe_request_resource(self):
        request = server.CancelObserveRequest()
        request.add(("TestClient1", (1, 2, 3)))
        expectedXml = """
        <Request>
          <Type>Observe</Type>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
                <Objects>
                  <Object>
                    <ID>1</ID>
                    <ObjectInstance>
                      <ID>2</ID>
                      <Resource>
                        <ID>3</ID>
                        <CancelObserve/>
                      </Resource>
                    </ObjectInstance>
                  </Object>
                </Objects>
              </Client>
            </Clients>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_cancel_observe_request_object_instance(self):
        request = server.CancelObserveRequest()
        request.add(("TestClient1", (1, 2)))
        expectedXml = """
        <Request>
          <Type>Observe</Type>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
                <Objects>
                  <Object>
                    <ID>1</ID>
                    <ObjectInstance>
                      <ID>2</ID>
                      <CancelObserve/>
                    </ObjectInstance>
                  </Object>
                </Objects>
              </Client>
            </Clients>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

    def test_cancel_observe_request_object(self):
        request = server.CancelObserveRequest()
        request.add(("TestClient1", (3,)))
        expectedXml = """
        <Request>
          <Type>Observe</Type>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
                <Objects>
                  <Object>
                    <ID>3</ID>
                    <CancelObserve/>
                  </Object>
                </Objects>
              </Client>
            </Clients>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())

# EXECUTE REQUEST tests

    def test_execute_request_simple(self):
        request = server.ExecuteRequest()
        request.add(("TestClient1", (1, 2, 3, 4)), "VALUE0")
        expectedXml = """
        <Request>
          <Type>Execute</Type>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
                <Objects>
                  <Object>
                    <ID>1</ID>
                    <ObjectInstance>
                      <ID>2</ID>
                      <Resource>
                        <ID>3</ID>
                        <ResourceInstance>
                          <ID>4</ID>
                          <Value>VALUE0</Value>
                        </ResourceInstance>
                      </Resource>
                    </ObjectInstance>
                  </Object>
                </Objects>
              </Client>
            </Clients>
          </Content>
        </Request>
        """
        self.assertEqualXml(expectedXml, request.serialize())


################################################################################
# RESPONSE tests
################################################################################
class TestLWM2MServerIpcResponses(unittest.TestCase):


## WRITE RESPONSE tests

    def test_write_response_simple(self):
        responseXml = """
        <Response>
          <Type>Write</Type>
          <Code>200</Code>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
                <DefaultWriteMode>Update</DefaultWriteMode>
                <Objects>
                  <Object>
                    <ID>0</ID>
                    <ObjectInstance>
                      <ID>0</ID>
                      <Resource>
                        <ID>0</ID>
                        <Result>
                          <Error>Success</Error>
                        </Result>
                      </Resource>
                    </ObjectInstance>
                  </Object>
                </Objects>
              </Client>
            </Clients>
          </Content>
        </Response>
        """
        response = server.WriteResponse(responseXml)
        self.assertEqual("Write", response.type)
        self.assertEqual("200", response.code)
        self.assertEqual(("TestClient1",), response.content.getClientIDs())
        # TODO:
        #  - test client response as content
        #  - test default write mode
        #  - test path result

## READ RESPONSE tests

    def test_read_response_simple(self):
        responseXml = """
        <Response>
          <Type>Read</Type>
          <Code>200</Code>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
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
              </Client>
            </Clients>
          </Content>
        </Response>
        """
        response = server.ReadResponse(responseXml)
        self.assertEqual("Read", response.type)
        self.assertEqual("200", response.code)
        self.assertEqual(("TestClient1",), response.content.getClientIDs())
        self.assertEqual("VALUE", response.getValue(("TestClient1", (1, 2, 3, 4))))

    def test_read_response_complex(self):
        responseXml = """
        <Response>
          <Type>Read</Type>
          <Code>1234</Code>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
                <Objects>
                  <Object>
                    <ID>10</ID>
                    <ObjectInstance>
                      <ID>20</ID>
                      <Resource>
                        <ID>30</ID>
                        <ResourceInstance>
                          <ID>40</ID>
                          <Value>VALUE0</Value>
                        </ResourceInstance>
                        <ResourceInstance>
                          <ID>41</ID>
                          <Value>VALUE1</Value>
                        </ResourceInstance>
                      </Resource>
                      <Resource>
                        <ID>31</ID>
                        <ResourceInstance>
                          <ID>40</ID>
                          <Value>VALUE2</Value>
                        </ResourceInstance>
                        <ResourceInstance>
                          <ID>41</ID>
                          <Value>VALUE3</Value>
                        </ResourceInstance>
                        <ResourceInstance>
                          <ID>42</ID>
                          <Value>VALUE4</Value>
                        </ResourceInstance>
                      </Resource>
                    </ObjectInstance>
                    <ObjectInstance>
                      <ID>21</ID>
                      <Resource>
                        <ID>30</ID>
                        <ResourceInstance>
                          <ID>40</ID>
                          <Value>VALUE5</Value>
                        </ResourceInstance>
                      </Resource>
                      <Resource>
                        <ID>31</ID>
                        <ResourceInstance>
                          <ID>40</ID>
                          <Value>VALUE6</Value>
                        </ResourceInstance>
                        <ResourceInstance>
                          <ID>41</ID>
                          <Value>VALUE7</Value>
                        </ResourceInstance>
                      </Resource>
                    </ObjectInstance>
                  </Object>
                  <Object>
                    <ID>11</ID>
                    <ObjectInstance>
                      <ID>20</ID>
                      <Resource>
                        <ID>30</ID>
                        <ResourceInstance>
                          <ID>40</ID>
                          <Value>VALUE8</Value>
                        </ResourceInstance>
                        <ResourceInstance>
                          <ID>41</ID>
                          <Value>VALUE9</Value>
                        </ResourceInstance>
                      </Resource>
                    </ObjectInstance>
                    <ObjectInstance>
                      <ID>21</ID>
                      <Resource>
                        <ID>30</ID>
                        <ResourceInstance>
                          <ID>40</ID>
                          <Value>VALUE10</Value>
                        </ResourceInstance>
                      </Resource>
                      <Resource>
                        <ID>31</ID>
                        <ResourceInstance>
                          <ID>40</ID>
                          <Value>VALUE11</Value>
                        </ResourceInstance>
                        <ResourceInstance>
                          <ID>41</ID>
                          <Value>VALUE12</Value>
                        </ResourceInstance>
                      </Resource>
                    </ObjectInstance>
                  </Object>
                </Objects>
              </Client>
            </Clients>
          </Content>
        </Response>
        """
        response = server.ReadResponse(responseXml)
        #print(responseXml); print(response); print(response.serialize())
        self.assertEqual("Read", response.type)
        self.assertEqual("1234", response.code)
        self.assertEqual(("TestClient1",), response.content.getClientIDs())
        self.assertEqual("VALUE0", response.getValue(("TestClient1", (10, 20, 30, 40))))
        self.assertEqual("VALUE1", response.getValue(("TestClient1", (10, 20, 30, 41))))
        self.assertEqual("VALUE2", response.getValue(("TestClient1", (10, 20, 31, 40))))
        self.assertEqual("VALUE3", response.getValue(("TestClient1", (10, 20, 31, 41))))
        self.assertEqual("VALUE4", response.getValue(("TestClient1", (10, 20, 31, 42))))
        self.assertEqual("VALUE5", response.getValue(("TestClient1", (10, 21, 30, 40))))
        self.assertEqual("VALUE6", response.getValue(("TestClient1", (10, 21, 31, 40))))
        self.assertEqual("VALUE7", response.getValue(("TestClient1", (10, 21, 31, 41))))
        self.assertEqual("VALUE8", response.getValue(("TestClient1", (11, 20, 30, 40))))
        self.assertEqual("VALUE9", response.getValue(("TestClient1", (11, 20, 30, 41))))
        self.assertEqual("VALUE10", response.getValue(("TestClient1", (11, 21, 30, 40))))
        self.assertEqual("VALUE11", response.getValue(("TestClient1", (11, 21, 31, 40))))
        self.assertEqual("VALUE12", response.getValue(("TestClient1", (11, 21, 31, 41))))



# GET_CLIENT_LIST RESPONSE tests

    def test_list_clients_response(self):
        responseXml = """
        <Response>
          <Type>ListClients</Type>
          <Code>200</Code>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
                <Status>Registered</Status>
                <Objects>
                  <Object>
                    <ID>2</ID>
                  </Object>
                  <Object>
                    <ID>3</ID>
                    <ObjectInstance>
                      <ID>0</ID>
                    </ObjectInstance>
                  </Object>
                </Objects>
              </Client>
              <Client>
                <ID>TestClient7</ID>
                <Status>Registered</Status>
                <Objects>
                  <Object>
                    <ID>2</ID>
                  </Object>
                  <Object>
                    <ID>3</ID>
                    <ObjectInstance>
                      <ID>0</ID>
                    </ObjectInstance>
                  </Object>
                </Objects>
              </Client>
              <Client>
                <ID>TestClient2</ID>
                <Status>Deregistered</Status>
              </Client>
            </Clients>
          </Content>
        </Response>
        """
        response = server.ListClientsResponse(responseXml)
        self.assertEqual("ListClients", response.type)
        self.assertEqual("200", response.code)
        #self.assertEqual("TestClient1", response.clientID)
        expectedList = ( "TestClient1", "TestClient7", "TestClient2" )
        self.assertEqual(expectedList, response.getClientIDs())

        # TODO:
        #  - test for client status
        #  - test for registered entities


## DELETE RESPONSE tests

    def test_delete_response_simple(self):
        responseXml = """
        <Response>
          <Type>Delete</Type>
          <Code>202</Code>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
                <Objects>
                  <ID>3</ID>
                  <ObjectInstance>
                    <ID>0</ID>
                    <Result>
                      <Error>Success</Error>
                    </Result>
                  </ObjectInstance>
                </Objects>
              </Client>
            </Clients>
          </Content>
        </Response>
        """
        response = server.DeleteResponse(responseXml)
        self.assertEqual("Delete", response.type)
        self.assertEqual("202", response.code)
        self.assertEqual(("TestClient1",), response.content.getClientIDs())
        # TODO:
        #  - test for path result

## EXECUTE RESPONSE tests

    def test_execute_response_simple(self):
        responseXml = """
        <Response>
          <Type>Execute</Type>
          <Code>204</Code>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
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
              </Client>
            </Clients>
          </Content>
        </Response>
        """
        response = server.ExecuteResponse(responseXml)
        self.assertEqual("Execute", response.type)
        self.assertEqual("204", response.code)
        self.assertEqual(("TestClient1",), response.content.getClientIDs())
        # TODO:
        #  - test path result

## OBSERVE RESPONSE tests

    def test_observe_response_simple(self):
        responseXml = """
        <Response>
          <Type>Observe</Type>
          <Code>205</Code>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
                <Objects>
                  <Object>
                    <ID>1</ID>
                    <ObjectInstance>
                      <ID>2</ID>
                      <Resource>
                        <ID>3</ID>
                        <Result>
                          <Error>Success</Error>
                        </Result>
                      </Resource>
                    </ObjectInstance>
                  </Object>
                </Objects>
              </Client>
            </Clients>
          </Content>
        </Response>
        """
        response = server.ObserveResponse(responseXml)
        self.assertEqual("Observe", response.type)
        self.assertEqual("205", response.code)
        self.assertEqual(("TestClient1",), response.content.getClientIDs())
        # TODO:
        #  - test path result

## CANCEL OBSERVE RESPONSE tests

    def test_cancel_observe_response_simple(self):
        responseXml = """
        <Response>
          <Type>Observe</Type>
          <Code>200</Code>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
                <Objects>
                  <Object>
                    <ID>1</ID>
                    <ObjectInstance>
                      <ID>2</ID>
                      <Resource>
                        <ID>3</ID>
                        <CancelObserve/>
                        <Result>
                          <Error>Success</Error>
                        </Result>
                      </Resource>
                    </ObjectInstance>
                  </Object>
                </Objects>
              </Client>
            </Clients>
          </Content>
        </Response>
        """
        response = server.CancelObserveResponse(responseXml)
        self.assertEqual("Observe", response.type)
        self.assertEqual("200", response.code)
        self.assertEqual(("TestClient1",), response.content.getClientIDs())
        # TODO:
        #  - test CancelObserve label
        #  - test path result

## OBSERVE NOTIFY tests

    @unittest.skip("Notifications not yet implemented")
    def test_observe_notify_simple(self):
        notifyXml = """
        <Notification>
          <Type>Observe</Type>
          <Code>205</Code>
          <Content>
            <Clients>
              <Client>
                <ID>TestClient1</ID>
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
              </Client>
            </Clients>
          </Content>
        </Notification>
        """
        notify = server.ObserveNotification(notifyXml)
        self.assertEqual("Notify", notify.type)
        self.assertEqual("205", notify.code)
        #self.assertEqual("TestClient1", notify.clientID)
        self.assertEqual("VALUE", notify.getValue(("TestClient", (1, 2, 3, 4))))
