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

"""
LWM2M Server IPC Interface
"""

from lxml import etree
import ipc_core

class IpcError(Exception):
    pass

class ContentNull(ipc_core.IpcContent):
    """There is no content in the message."""
    def __init__(self, element=None):
        pass

    def __str__(self):
        return "NoContent"

    def getElement(self):
        return None

    def getValue(self, path):
        return None

class ContentClientList(ipc_core.IpcContent):
    """The content is a list of client IDs."""
    def __init__(self, element=None):
        super(ContentClientList, self).__init__()
        if element is not None:
            e_clientIDs = element.findall("Content/Clients/Client/ID")
            self._clientIDList = []
            for ID in e_clientIDs:
                self._clientIDList.append(ID.text)

    def __str__(self):
        return str(self._clientIDList)

    def getClientIDs(self):
        return tuple(self._clientIDList)

class ContentClientID(ipc_core.IpcContent):

    class Client(object):
        def __init__(self, ID=None, model=None):
            self.ID = ID
            self.model = model

    """As Content, requests and responses have a "Clients" element with one or more "Client" children, each with a "ClientID" child."""
    def __init__(self, element=None):
        super(ContentClientID, self).__init__(None)
        self._clients = {}
        if element is not None:
            e_content = element.find("Content")
            if e_content is not None:
                e_clients = e_content.find("Clients")
                if e_clients is not None:
                    client_nodes = e_clients.findall("Client")
                    for e_client in client_nodes:
                        ID = e_client.find("ID").text
                        self._clients[ID] = self.Client(ID, ipc_core.DataModel(e_client))

    def __str__(self):
        res = ""
        for client in self._clients.values():
            res += "ClientID %s, Model %s" % (client.ID, client.model)
        return res

    def add(self, clientID, path, value, label):
        try:
            model = self._clients[clientID].model
        except KeyError:
            model = ipc_core.DataModel()
            self._clients[clientID] = self.Client(clientID, model)
        model.add(path, value, label)

    def getElement(self):
        e_content = etree.Element("Content")
        e_clients = etree.Element("Clients")
        for client_id in self._clients:
            e_client = etree.Element("Client")
            e_client.append(ipc_core.TElement("ID", str(client_id)))
            e_model = self._clients[client_id].model.getElement()
            # strip the outer container
            for child in e_model.getchildren():
                e_client.append(child)
            e_clients.append(e_client)
        e_content.append(e_clients)
        return e_content

    def getValue(self, client_id_and_path):
        """Path is a tuple: (ClientID, (O, I, R, i))"""
        assert len(client_id_and_path) == 2
        client_id, path = client_id_and_path
        return self._clients[client_id].model.getValue(path)

    def getClientIDs(self):
        # could use dictionary keys, or take ID from each client
        IDs = [ client.ID for client in self._clients.values() ]
        return tuple(IDs)

class IpcRequest(ipc_core.IpcRequest):
    ContentType = ContentClientID
    PathLabel = None

    def add(self, client_id_and_path, value=None):
        if len(client_id_and_path) != 2:
            raise IpcError("add requires a tuple of form (ClientID, (O, I, R, ...))")
        client_id, path = client_id_and_path
        if not ipc_core.is_sequence(path) or not ipc_core.validateModelPath(path, type(self).SupportedModelPaths):
            raise IpcError("Unsupported path")
        self._content.add(client_id, path, value, self.PathLabel)

    # TODO: consider replacing add() above, and promoting this to superclass, somehow:
    #def add(self, path, *args):
    #    self._content.add(path[0], path[1], args[0])

class IpcResponse(ipc_core.IpcResponse):
    ContentType = ContentClientID


class IpcNotification(ipc_core.IpcNotification):
    ContentType = ContentClientID


# Messages

## Connect
class ConnectRequest(IpcRequest):
    MessageType = "Connect"
    ContentType = ContentNull

class ConnectResponse(IpcResponse):
    MessageType = "Connect"
    ContentType = ContentNull
    # TODO: handle definitions in response

## Disconnect
class DisconnectRequest(IpcRequest):
    MessageType = "Disconnect"
    ContentType = ContentNull

class DisconnectResponse(IpcResponse):
    MessageType = "Disconnect"
    ContentType = ContentNull

## Define
# TODO: need metadata first

## ListClients
class ListClientsRequest(IpcRequest):
    MessageType = "ListClients"
    ContentType = ContentNull

class ListClientsResponse(IpcResponse):
    MessageType = "ListClients"
    ContentType = ContentClientList

    def getClientIDs(self):
        return self._content.getClientIDs()

## Write
class WriteRequest(IpcRequest):
    MessageType = "Write"
    SupportedModelPaths = ( "O", "OI", "OIR", "OIRi" )

class WriteResponse(IpcResponse):
    MessageType = "Write"

## Read
class ReadRequest(IpcRequest):
    MessageType = "Read"
    SupportedModelPaths = ( "O", "OI", "OIR" )
    def add(self, path, value=None):
        super(ReadRequest, self).add(path, None)

class ReadResponse(IpcResponse):
    MessageType = "Read"

## WriteAttributes

## Execute
class ExecuteRequest(IpcRequest):
    MessageType = "Execute"

class ExecuteResponse(IpcResponse):
    MessageType = "Execute"

## Create is part of Write

## Delete
class DeleteRequest(IpcRequest):
    MessageType = "Delete"
    SupportedModelPaths = ( "OI", "OIR" )
    def add(self, path, value=None):
        super(DeleteRequest, self).add(path, None)

class DeleteResponse(IpcResponse):
    MessageType = "Delete"

## Observe
class ObserveRequest(IpcRequest):
    MessageType = "Observe"
    SupportedModelPaths = ( "O", "OI", "OIR" )
    def add(self, path, value=None):
        super(ObserveRequest, self).add(path, None)

class ObserveResponse(IpcResponse):
    MessageType = "Observe"

## CancelObserve
class CancelObserveRequest(IpcRequest):
    MessageType = "Observe"
    SupportedModelPaths = ( "O", "OI", "OIR" )
    PathLabel = "CancelObserve"
    def add(self, path, value=None):
        super(CancelObserveRequest, self).add(path, None)

class CancelObserveResponse(IpcResponse):
    MessageType = "Observe"


## Observe Notification

class ObserveNotification(IpcNotification):
    MessageType = "Notify"


## Register Notification
# TODO

## Deregister Notification
# TODO

## Update Notification
# TODO

