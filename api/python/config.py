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

    BOOTSTRAP_CONFIG_FILE = "../../bootstrap-localhost.config"

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

    BOOTSTRAP_CONFIG_FILE = "../lwm2m_ipc/bootstrap.config"

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
