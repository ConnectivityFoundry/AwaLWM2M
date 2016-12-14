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
LWM2M Client IPC Interface
"""
import xml.etree.ElementTree # py-lxml is not yet available in openwrt
from ipc_core import IpcRequest, IpcResponse, IpcNotification, IpcContent, IpcError


class DefineContent(IpcContent):

    def __init__(self, element=None):
        self._element = element

    def __str__(self):
        return str(self._element)

    def define_content(self, element):
        self._element = element

    def getElement(self):
        return self._element


## Define
# TODO: need metadata
class DefineRequest(IpcRequest):
    MessageType = "Define"
    ContentType = DefineContent

    def define_data(self, element_str, obj_id=None):
        if obj_id is not None:
            pass  #TODO considering to load data
        else:
            try:
                self._content.define_content(xml.etree.ElementTree.fromstring(element_str))
            except:
                raise IpcError("Invalid xml %s", element_str)

## Set
class SetRequest(IpcRequest):
    MessageType = "Set"
    SupportedModelPaths = ("OIR", "OIRi")

class SetResponse(IpcResponse):
    MessageType = "Set"

class CreateRequest(IpcRequest):
    MessageType = "Set"
    PathLabel = "Create"

## Get
class GetRequest(IpcRequest):
    MessageType = "Get"
    SupportedModelPaths = ( "O", "OI", "OIR", "OIRi")

class GetResponse(IpcResponse):
    MessageType = "Get"

## SubscribeToExecute
class SubscribeToExecuteRequest(IpcRequest):
    MessageType = "Subscribe"
    SupportedModelPaths = ( "OIR", )
    PathLabel = "SubscribeToExecute"

class SubscribeToExecuteResponse(IpcResponse):
    MessageType = "Subscribe"
    PathLabel = "SubscribeToExecute"

## CancelSubscribeToExecute
class CancelSubscribeToExecuteRequest(IpcRequest):
    MessageType = "Subscribe"
    SupportedModelPaths = ( "OIR", )
    PathLabel = "CancelSubscribeToExecute"

class CancelSubscribeToExecuteResponse(IpcResponse):
    MessageType = "Ssubscribe"
    PathLabel = "CancelSubscribeToExecute"

## SubscribeToChange
class SubscribeToChangeRequest(IpcRequest):
    MessageType = "Subscribe"
    SupportedModelPaths = ( "OI", "OIR" )
    PathLabel = "SubscribeToChange"

class SubscribeToChangeResponse(IpcResponse):
    MessageType = "Subscribe"
    PathLabel = "SubscribeToChange"

## CancelSubscribeToChange
class CancelSubscribeToChangeRequest(IpcRequest):
    MessageType = "Subscribe"
    SupportedModelPaths = ( "OI", "OIR" )
    PathLabel = "CancelSubscribeToChange"

class CancelSubscribeToChangeResponse(IpcResponse):
    MessageType = "Subscribe"
    PathLabel = "CancelSubscribeToChange"

## Delete
class DeleteRequest(IpcRequest):
    MessageType = "Delete"
    SupportedModelPaths = ( "OI", "OIR" )

class DeleteResponse(IpcResponse):
    MessageType = "Delete"

## ExecuteNotification
class ExecuteNotification(IpcNotification):
    MessageType = "ExecuteNotification"

## ChangeNotification
class ChangeNotification(IpcNotification):
    MessageType = "ChangeNotification"

