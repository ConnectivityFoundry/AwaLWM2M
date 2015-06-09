# Preliminary API to support testing

import ipc
import ipc_lwm2m_client as client
import ipc_lwm2m_server as server

class API(object):

    def __init__(self, ipcAddress, ipcPort):
        self._ipc = "udp://" + ipcAddress + ":" + str(ipcPort)

    def GetClientList(self, clientID):
        request = server.ListClientsRequest()
        response = ipc.send_request_and_receive_response(self._ipc, request.serialize())
        return server.ListClientsResponse(response).getClientIDs()
