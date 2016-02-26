
![](img.png)
----

# Awa LightweightM2M. 

## User guide.

This document is aimed at application developers who are using the Awa LightweightM2M libraries and tools as a foundation, or enhancement of their own M2M applications.

Developers who aim to contribute to the Awa LightweightM2M project are referred to the [contributor guide](../CONTRIBUTING.md) and the [developer guide](developer_guide.md).

----



### Contents.

* [Introduction.](userguide.md#introduction)  
* [The LWM2M client.](userguide.md#the-lwm2m-client)  
    * [The Awa client daemon](userguide.md#the-awa-client-daemon)  
* [The LWM2M server.](userguide.md#the-lwm2m-server)  
    * [The Awa server daemon](userguide.md#the-awa-server-daemon)  
* [The LWM2M Bootstrap server.](userguide.md#the-lwm2m-bootstrap-server) 
    * [The Awa bootstrap server daemon](userguide.md#the-awa-bootstrap-server-daemon)  


----

### Introduction.
LWM2M is a protocol that allows client resources to be accessed by a server. In the Awa library the client and server are daemons (individual processes), each having its own respective API interface over an inter-process communication interface (IPC). The client API is for use exclusively with the client daemon, and the server API is for use exclusively with the server daemon.  
A suite of tools is provided to exercise the main functionality of the API for both the client and server. Later sections describe the configuration of the Awa client and server daemons, along with examples of tools use.

A bootstrap server daemon is also provided that implements the LWM2M bootstrapping protocol which instructs LWM2M clients which LWM2M server to connect to. Later sections describe the configuration of the Awa bootstrap server.

### The LWM2M object model.

----

![LWM2M object model](LWM2M_object_referencing.png)

----


LWM2M is based on an object model such that:
* An object is defined to reresent a device or client application (the LWM2M client), or a LWM2M device management function. 
* Each object is defined on both the client and the server.
* The client hosted object holds the current status of the device. The server hosted object holds the device's status on the server. 
* One or more instances of an object may exist simultaneously but each will have a unique instance identifier.
* An object is composed of one or more resources, grouped under a single object identifier.
* Each resource is fully described and is uniquely addressable within the object.
* One or more instances of a resource may exist simultaneously within an object but each will have a unique instance identifier.

Object and resource identifiers are 16 bit integers. Object and resource *instance* identifiers are 8 bit integers. 

A resource instance is accessed via its parent object instance using a semantic approach:

````{objectID}/{object instance}/{resource ID}/{resource instance}````

In the case where a single instance of an object and resource exists the address resolves to ````{objectID}/{ResourceID}````

The semantic approach allows resources to be written to directly: ````1000/0/1/2='this value"````
