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

#!/usr/bin/env python

import sys
import os
import os.path
import signal
import time
import socket
import struct
import errno

try:
    BOOTSTRAP_SERVER_BINARY = os.environ['LWM2M_BOOTSTRAPD_BIN']
except KeyError:
    # use PATH to locate
    BOOTSTRAP_SERVER_BINARY = "awa_bootstrapd"

try:
    SERVER_BINARY = os.environ['LWM2M_SERVERD_BIN']
except KeyError:
    # use PATH to locate
    SERVER_BINARY = "awa_serverd"

try:
    CLIENT_BINARY = os.environ['LWM2M_CLIENTD_BIN']
except KeyError:
    # use PATH to locate
    CLIENT_BINARY = "awa_clientd"

IPC_TIMEOUT = 10  # seconds

CONTENT_TYPE_MAP = { "text/plain" : 0,
                     "application/octet-stream" : 42,
                     "application/json" : 50,
                     "application/vnd.oma.lwm2m+text" : 1541,
                     "application/vnd.oma.lwm2m+tlv" : 1542,
                     "application/vnd.oma.lwm2m+json" : 1543,
                     }

class OverlordException(Exception):
    pass

def getContentTypeID(contentType):
    try:
        ID = CONTENT_TYPE_MAP[contentType]
    except AttributeError:
        raise OverlordException("Unknown content type '%s'" % (contentType,))
    return ID

def waitForIPC(ipcPort, timeout, request):
    """Timeout is in seconds."""

    # assume 127.0.0.1 for now
    address = "127.0.0.1"
    port = int(ipcPort)

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    # overall timeout in microseconds
    timeout_us = timeout * 1000000

    # set socket timeout (10 ms per attempt)
    sec = 0
    usec = 10000
    timeval = struct.pack('ll', sec, usec)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVTIMEO, timeval)

    count = 0
    maxCount = timeout_us / (sec * 1000000 + usec)
    response = False
    while not response and count < maxCount:
        sock.sendto(request, (address, port))
        try:
            data, addr = sock.recvfrom(65536)
        except socket.error as serr:
            if serr.errno != errno.EAGAIN:  # EAGAIN == Resource Temporarily Unavailable
                raise serr
        else:
            response = len(data) > 0
        count += 1

    sock.close()
    return response

class Daemon(object):

    def __init__(self):
        self._pid = 0

    def spawn(self, wait=False, silent=False):
        if not os.path.isfile(self._file):
            raise OverlordException("%s not found" % (self._file, ))

        """Fork and exec the target process."""
        pid = os.fork()

        if pid == 0:
            # child

            if silent:
                fd = os.open("/dev/null", os.O_APPEND)
                os.dup2(fd, sys.stdout.fileno())
            sys.stdout.flush()

            os.execvp(self._file, self._args)
            exit(1)

        elif pid > 0:
            # parent
            if wait:
                pid, status = os.wait()
        else:
            os.perror("fork failed")
            pid = -1

        self._pid = pid

    def kill(self, silent=False):
        if self._pid > 0:
            os.kill(self._pid, signal.SIGKILL)
            self._pid = 0

    def terminate(self, silent=False):
        if self._pid > 0:
            os.kill(self._pid, signal.SIGTERM)
            self._pid = 0

    def interrupt(self, silent=False):
        if self._pid > 0:
            os.kill(self._pid, signal.SIGINT)
            self._pid = 0

    def waitForIPC(self, ipcPort, timeout):
        return waitForIPC(ipcPort, timeout, self._ipcWaitRequest)

class LWM2MBootstrapServerDaemon(Daemon):
    def __init__(self, address, coapPort, configFile, logFile):
        super(LWM2MBootstrapServerDaemon, self).__init__()
        self._file = BOOTSTRAP_SERVER_BINARY
        self._args = [ BOOTSTRAP_SERVER_BINARY,
                       "--verbose",
                       "--ip", address,
                       "--port", str(coapPort),
                       "--config", configFile,
                       "--logfile", logFile,
        ]

    def spawn(self, silent=False):
        super(LWM2MBootstrapServerDaemon, self).spawn(silent)

        if not silent:
            print("LWM2M Bootstrap Server: pid %d" % (self._pid,))

    def kill(self, silent=False):
        pid = self._pid
        super(LWM2MBootstrapServerDaemon, self).kill(silent)
        if not silent:
            print("LWM2M Bootstrap Server: pid %d killed" % (pid,))


class LWM2MServerDaemon(Daemon):
    def __init__(self, ipcPort, address, coapPort, logFile, contentType="application/vnd.oma.lwm2m+tlv"):
        super(LWM2MServerDaemon, self).__init__()
        contentTypeID = getContentTypeID(contentType)

        self._file = SERVER_BINARY
        self._args = [ SERVER_BINARY,
                       "--verbose",
                       "--ip", address,
                       "--port", str(coapPort),
                       "--ipcPort", str(ipcPort),
                       "--logFile", logFile,
                       "--contentType", str(contentTypeID),
        ]
#        self._file = "/usr/bin/valgrind"
#        self._args = [ "/usr/bin/valgrind",
#                       "--leak-check=full", "--show-leak-kinds=all", "--track-origins=yes",
#                       SERVER_BINARY,
#                       "--verbose",
#                       "--ip", address,
#                       "--port", str(coapPort),
#                       "--ipcPort", str(ipcPort),
#                       "--logFile", logFile,
#                       "--contentType", str(contentTypeID),
#        ]
        self._ipcWaitRequest = "<Request><Type>ListClients</Type></Request>"
        self._ipcPort = ipcPort

    def spawn(self, silent=False):
        super(LWM2MServerDaemon, self).spawn(silent)

        # wait for server IPC
        if not self.waitForIPC(self._ipcPort, IPC_TIMEOUT):
            raise Exception("Server IPC timed out")

        if not silent:
            print("LWM2M Server: pid %d, IPC port %d" % (self._pid, self._ipcPort))

    def kill(self, silent=False):
        pid = self._pid
        super(LWM2MServerDaemon, self).kill(silent)
        if not silent:
            print("LWM2M Server: pid %d killed" % (pid,))


class LWM2MClientDaemon(Daemon):
    def __init__(self, ipcPort, coapPort, logFile, endpointName, bootstrapConfigFile, objectDefinitionsFile):
        super(LWM2MClientDaemon, self).__init__()
        self._file = CLIENT_BINARY
        self._args = [ CLIENT_BINARY,
                       "--verbose",
                       "--port", str(coapPort),
                       "--ipcPort", str(ipcPort),
                       "--logFile", logFile,
                       "--endPointName", endpointName,
                       "--factoryBootstrap", bootstrapConfigFile,
                       "--objDefs", objectDefinitionsFile,
        ]
        self._ipcWaitRequest = "<Request><Type>Get</Type><Content><ObjectID>3</ObjectID><InstanceID>0</InstanceID><PropertyID>15</PropertyID></Content></Request>"
        self._ipcPort = ipcPort


    def spawn(self, silent=False):
        super(LWM2MClientDaemon, self).spawn(silent)

        # wait for server IPC
        if not self.waitForIPC(self._ipcPort, IPC_TIMEOUT):
            raise Exception("Client IPC timed out on port %d", self._ipcPort)

        if not silent:
            print("LWM2M Client: pid %d, IPC port %d" % (self._pid, self._ipcPort))

    def kill(self, silent=False):
        pid = self._pid
        super(LWM2MClientDaemon, self).kill(silent)
        if not silent:
            print("LWM2M Client: pid %d killed" % (pid,))


def SpawnDaemons(config):

    server = LWM2MServerDaemon(config.daemonPath, config.serverIpcPort, config.serverCoapPort, config.serverLogFile)
    client = LWM2MClientDaemon(config.daemonPath, config.clientIpcPort, config.clientCoapPort, config.clientLogFile, config.clientEndpointName, config.bootstrapConfigFile, config.objectDefinitionsFile)

    server.spawn()

    client.spawn()

    return (client, server)

def KillDaemons(daemons):
    for daemon in daemons:
        daemon.kill()

def main():
    import config
    config = config.Config()
    daemons = SpawnDaemons(config)
    raw_input("Hit enter")
    KillDaemons(daemons)


if __name__ == "__main__":
    main()
