# Common Test Functions and Classes

import unittest
import base64
import time

import ipc
import overlord
import config
import client_low

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
    api = client_low.API("127.0.0.1", ipcPort)
    found = False
    while not found:
        print "waiting"
        clients = api.GetClientList(clientID)
        if len(clients) > 0:
            if clientID in clients:
                found = True
        if not found:
            time.sleep(0.01)

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

            self._serverDaemon.spawn()

        print "Starting client daemon"
        if self.config.spawnClientDaemon:
            self._clientDaemon = overlord.LWM2MClientDaemon(self.config.clientIpcPort,
                                                            self.config.clientCoapPort,
                                                            self.config.clientLogFile,
                                                            self.config.clientEndpointName,
                                                            self.config.bootstrapConfigFile)
            self._clientDaemon.spawn()

            # wait for client to register with server
            waitForClient(self.config.clientEndpointName, self.config.serverIpcPort)

    def tearDown(self):
        # tearDown functionality moved to doCleanups as it will be called even if the test crashes
        pass

    def doCleanups(self):
        # kill client first
        try:
            self._clientDaemon.terminate()
        except AttributeError:
            pass

        try:
            self._serverDaemon.terminate()
        except AttributeError:
            pass

    def sendClientRequest(self, requestType, responseType, path, value):
        """Send a client request, using a path tuple of the form: (ObjectID, ObjectInstanceID, ResourceID, ResourceInstanceID)."""
        return _sendRequest(self.config.clientIpc, requestType, responseType, path, value)

    def sendServerRequest(self, requestType, responseType, clientID, path, value):
        """Send a server request, using a path tuple of the form (ClientID, (ObjectID, ObjectInstanceID, ResourceID, ResourceInstanceID))."""
        return _sendRequest(self.config.serverIpc, requestType, responseType, (clientID, path), value)
