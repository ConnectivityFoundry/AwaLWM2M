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

# Common Test Functions and Classes

import unittest
import base64
import time

import ipc
import overlord
import config
import api

g_ConfigurationClass = None  # use default

def setConfigurationClass(_configuration):
    """Override the configuration class."""
    global g_ConfigurationClass
    g_ConfigurationClass = _configuration

def encodeString(string):
    return base64.encodestring(string).strip()

def decodeString(string):
    return base64.decodestring(string)

def _sendRequest(ipcChannel, requestType, responseType, path, value):
    request = requestType()
    request.add(path, value)
    #print(request.serialize())
    response_xml = ipc.send_request_and_receive_response(ipcChannel, request.serialize())
    #print(response_xml)
    response = responseType(response_xml)
    return response


def waitForClient(clientID, ipcPort):
    """Block until client endpoint name appears in the list of registered clients on the server."""
    server_api = api.ServerAPI("127.0.0.1", ipcPort)
    found = False
    while not found:
        print "waiting"
        clients = server_api.GetClientList(clientID)
        if len(clients) > 0:
            if clientID in clients:
                found = True
        if not found:
            time.sleep(0.01)
    del server_api

class SpawnDaemonsTestCase(unittest.TestCase):
    """TestCase class that spawns LWM2M Server and Client daemons before each test, then kills them afterwards."""

    def setUp(self):
        if g_ConfigurationClass:
            print("Using configuration override: " + str(g_ConfigurationClass))
            self.config = config.Config(g_ConfigurationClass)  # use override
        else:
            print("Using default configuration")
            self.config = config.Config()  # use default

        # start server first
        print "Starting server daemon"
        if self.config.spawnServerDaemon:
            self._serverDaemon = overlord.LWM2MServerDaemon(self.config.serverIpcPort,
                                                            self.config.serverAddress,
                                                            self.config.serverCoapPort,
                                                            self.config.serverLogFile)

            self.addCleanup(self._serverDaemon.terminate)
            self._serverDaemon.spawn()

        print "Starting client daemon"
        if self.config.spawnClientDaemon:
            self._clientDaemon = overlord.LWM2MClientDaemon(self.config.clientIpcPort,
                                                            self.config.clientCoapPort,
                                                            self.config.clientLogFile,
                                                            self.config.clientEndpointName,
                                                            self.config.bootstrapConfigFile)
            self.addCleanup(self._clientDaemon.terminate)
            self._clientDaemon.spawn()

            # wait for client to register with server
            waitForClient(self.config.clientEndpointName, self.config.serverIpcPort)

    def tearDown(self):
        # tearDown functionality moved to cleanup functions as
        # they will be called even if setUp raises an exception
        pass

    def sendClientRequest(self, requestType, responseType, path, value):
        """Send a client request, using a path tuple of the form: (ObjectID, ObjectInstanceID, ResourceID, ResourceInstanceID)."""
        return _sendRequest(self.config.clientIpc, requestType, responseType, path, value)

    def sendServerRequest(self, requestType, responseType, clientID, path, value):
        """Send a server request, using a path tuple of the form (ClientID, (ObjectID, ObjectInstanceID, ResourceID, ResourceInstanceID))."""
        return _sendRequest(self.config.serverIpc, requestType, responseType, (clientID, path), value)
