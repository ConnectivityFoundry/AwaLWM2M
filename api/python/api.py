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

# Preliminary API to support testing

import ipc
import ipc_lwm2m_client as client
import ipc_lwm2m_server as server

class ServerAPI(object):

    def __init__(self, ipcAddress, ipcPort):
        self._ipc = "udp://" + ipcAddress + ":" + str(ipcPort)

        # Connect
        request = server.ConnectRequest(session_id=None)
        response = ipc.send_request_and_receive_response(self._ipc, request.serialize())
        self._session_id = server.ConnectResponse(response).session_id
        print("Session ID %s" % (self._session_id,))

    def __del__(self):
        # Disconnect
        if self._session_id is not None:
            request = server.DisconnectRequest(session_id=self._session_id)
            response = ipc.send_request_and_receive_response(self._ipc, request.serialize())
            #print "data " + ipc.receive_datagram(self._ipc)
            #import pdb; pdb.set_trace()

    def GetClientList(self, clientID):
        request = server.ListClientsRequest(session_id=self._session_id)
        response = ipc.send_request_and_receive_response(self._ipc, request.serialize())
        return server.ListClientsResponse(response).getClientIDs()

class ClientAPI(object):
    # TODO
    pass
