"""
LWM2M Client IPC Interface
"""

from lxml import etree
from ipc_core import IpcRequest, IpcResponse, IpcNotification


## Define
# TODO: need metadata

## Set
class SetRequest(IpcRequest):
    MessageType = "Set"
    SupportedModelPaths = ( "OIR", "OIRi")

class SetResponse(IpcResponse):
    MessageType = "Set"

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

