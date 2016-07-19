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

class _DefaultsStandalone(object):
    """Do not use this class outside of this module."""

    SPAWN_CLIENT_DAEMON = False
    SPAWN_SERVER_DAEMON = False

    SERVER_IPC_PORT = 54321
    CLIENT_IPC_PORT = 12345

    SERVER_ADDRESS = "127.0.0.1"
    SERVER_COAP_PORT = 5683
    CLIENT_COAP_PORT = 6000

    #DAEMON_PATH = "../.."

    SERVER_LOG_FILE = "awa_serverd.log"
    CLIENT_LOG_FILE = "awa_clientd.log"

    BOOTSTRAP_CONFIG_FILE = "../../localhost.bsc"

    CLIENT_ENDPOINT_NAME = "TestClient"

class _DefaultsSpawn(object):
    """Do not use this class outside of this module."""

    SPAWN_CLIENT_DAEMON = True
    SPAWN_SERVER_DAEMON = True

    SERVER_IPC_PORT = range(61731, 61741)
    CLIENT_IPC_PORT = range(61742, 61752)

    SERVER_ADDRESS = "127.0.0.1"
    SERVER_COAP_PORT = 6101
    CLIENT_COAP_PORT = 6102

    #DAEMON_PATH = "../.."

    SERVER_LOG_FILE = "awa_serverd.log"
    CLIENT_LOG_FILE = "awa_clientd.log"

    BOOTSTRAP_CONFIG_FILE = "../../localhost.bsc"

    CLIENT_ENDPOINT_NAME = "imgClient"

# select a default configuration class
_DefaultConfigurationClass = _DefaultsSpawn

g_portIndex = 0

class Config(object):
    def __init__(self, configuration=_DefaultConfigurationClass):
        self._configurationClass = configuration

        # attempt to select from a list of available ports (avoids 'listener: failed to bind socket').
        # if this fails, the ports are single values
        try:
            global g_portIndex
            self._serverIpcPort = self._configurationClass.SERVER_IPC_PORT[g_portIndex % len(self._configurationClass.SERVER_IPC_PORT)]
            self._clientIpcPort = self._configurationClass.CLIENT_IPC_PORT[g_portIndex % len(self._configurationClass.CLIENT_IPC_PORT)]
            g_portIndex += 1
            #print "Config using rotating ports. Client IPC port: %d Server IPC port: %d" % (self._clientIpcPort, self._serverIpcPort)
            #pprint(self._configurationClass.SERVER_IPC_PORT)
        except TypeError:  # server / client IPC ports are single values
            self._serverIpcPort = self._configurationClass.SERVER_IPC_PORT
            self._clientIpcPort = self._configurationClass.CLIENT_IPC_PORT
            self._bootstrapConfigFile = self._configurationClass.BOOTSTRAP_CONFIG_FILE
            self._objectDefinitionsFile = self._configurationClass.OBJECT_DEFINITIONS_FILE
            #print "Config using single ports. Client IPC port: %d Server IPC port: %d" % (self._clientIpcPort, self._serverIpcPort)

    @property
    def spawnServerDaemon(self):
        return self._configurationClass.SPAWN_SERVER_DAEMON

    @property
    def spawnClientDaemon(self):
        return self._configurationClass.SPAWN_CLIENT_DAEMON

    @property
    def serverIpcPort(self):
        return self._serverIpcPort

    @property
    def clientIpcPort(self):
        return self._clientIpcPort

    @property
    def serverAddress(self):
        return self._configurationClass.SERVER_ADDRESS

    @property
    def serverCoapPort(self):
        return self._configurationClass.SERVER_COAP_PORT

    @property
    def clientCoapPort(self):
        return self._configurationClass.CLIENT_COAP_PORT

    @property
    def serverLogFile(self):
        return self._configurationClass.SERVER_LOG_FILE

    @property
    def clientLogFile(self):
        return self._configurationClass.CLIENT_LOG_FILE

    @property
    def bootstrapConfigFile(self):
        return self._configurationClass.BOOTSTRAP_CONFIG_FILE

    @property
    def objectDefinitionsFile(self):
        return self._configurationClass.OBJECT_DEFINITIONS_FILE

    @property
    def clientEndpointName(self):
        return self._configurationClass.CLIENT_ENDPOINT_NAME

    @property
    def serverIpc(self):
        return "udp://127.0.0.1:" + str(self.serverIpcPort)

    @property
    def clientIpc(self):
        return "udp://127.0.0.1:" + str(self.clientIpcPort)

    @property
    def getServerHost(self):
        return "localhost"

    @property
    def getClientHost(self):
        return "localhost"
