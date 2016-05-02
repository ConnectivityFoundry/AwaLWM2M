
![](img.png)
----

# LWM2M overview

## What is LWM2M?
Lightweight M2M (LWM2M) is a set of protocols defined by the Open Mobile Alliance (OMA) for machine-to-machine (M2M) or Internet of Things (IoT) device management and communications.
LWM2M is primarily designed for the management of devices that are resource constrained, having limited memory, processing power or battery life, thus the protocols involved are required to have a small code footprint, to promote power efficient communications and to minimise data models. LWM2M connectivity is via Constrained Application Protocol (CoAP), secured using Datagram Transport Layer Security (DTLS).
Above all, OMA LWM2M provides for an open standard, open source, secure, inter-operable and scalable device management and communications solution employing established and future proof protocols.
 
### Summary of LWM2M services:

* A simple object based resource model  
* Resource observation and notification  
* Create, read, update, delete, and configure device attributes  
* Support for TLV, JSON, Plain text, and opaque data formats  
* User Datagram Protocol (UDP) support  
* Security via DTLS  
* LWM2M bootstrap server support  
* LWM2M management server support  

### Basic LWM2M functionality includes:

* LWM2M server/client models  
* Device bootstrapping, registration and connectivity  
* Access control  
* Firmware updates  
* Device metrics  


Further information:  
[OMA LWM2M technical specifications ](http://technical.openmobilealliance.org/Technical/technical-information/release-program/current-releases/oma-lightweightm2m-v1-0)  
[CoAP.](http://coap.technology/)  

## What is device management?
An IoT Application may be built on a widely distributed network composed of numerous devices of varying types and complexity. Device management is the means by which an IoT Application can: 

* Accept new devices onto its network via a process of registration  
* Authenticate device connectivity  
* Manage M2M communications  
* Store and manage device data on a per device basis  
* Impose rules and housekeeping practices on stored device data  
* Safely remove redundant devices  
* Upgrade device firmware 'over-the-air'  
* Apply a command to a particular device or group of devices based on type, (to reset a device for example)  
* Gather metrics or diagnostics about a device or group of devices  
* Apply default settings to a particular type of device, (or store defaults in such a way that the device can access them)  
* Apply or revoke device access, control and/or ownership  
* Log and respond to device error states  

The above list is by no means exhaustive and device management requirements can become complex and unwieldy as the scale of the IoT Application increases. It is important to implement a device management strategy that supports the operation of a device throughout its entire life cycle.

## The device life cycle

### Manufacturing

While not mandatory, options are available to embed unique security keys at the point of device manufacture to negate the possibility of device ‘spoofing’.

### Network on-boarding

This process adds the new device to the IoT network. Depending on the type of device, on-boarding verifies device authentication (using either embedded or soft security keys), performs LWM2M bootstrapping, and often involves a service discovery stage.

### Provisioning

An extension of the on-boarding process, provisioning caters for device registration with available support services and resources such as data storage, metrics, user association (ownership) and settings storage.

### In service

This is when the device performs the task for which it was designed. During this stage the device may still be actively managed, will provide diagnostic or state information and may be field or OTA upgradeable.

### End of life

Device end of life offers two options:

* **Replacement.** Device replacement demands that the application is contiguous in terms of data and/or service such that the swapping out of the device is a seamless process.
* **Redundancy.** While a device may become redundant, the data that it has provided during its in service state may be of value to the application, thus provision for data archiving may be required.

## LWM2M architecture

LWM2M provides four interfaces, each designed for specific purpose as shown below:

* **Bootstrap** -  used to provide LWM2M management server credentials to a LWM2M client for registration purposes.  
* **Client registration** - enables the LWM2M client to register with one or more LWM2M servers.  
* **Device management and service enablement** - used by the LWM2M Server to access an Object Instance or Resource on the client side. The main operation are: read, write, delete, execute, write attribute and discover.  
* **Information reporting** -  used by the LWM2M Server to observe any changes in a resource on a LWM2M client and to receive notifications from the client when an observed value changes.  


## The Object/Instance/Resource (O/I/R) model.

A Resource is a single, typed, item of data which is exposed by a LWM2M client for consumption by a control or management application. As an example, a remote temperature sensing device could expose several items of data such as:

* Its manufacturer (type string)  
* Its unique serial number (type string)  
* Its unique identifier or device name (type string)  
* Its present status (type integer)  
* Its present sensor measurement value (type float)  
* Its geographic location (type array of integer)  
* Its present RTC time (type integer)  
and so on.

Each of the above would be defined as an individual *Resource* associated with that device, which is made available (directly or indirectly) to the managing application.  
Resources are defined in terms of:

* **data type** - the data type of the value of the resource (integer, string, float, array ...)  
* **access control** - depending on allowed operations  
* **multiplicity** - whether the resource may have more than one concurrent instance (boolean), and whether or not the resource is mandatory (boolean)  
* **operation** - the types of operation that may be performed on the resource (read, write, readWrite, execute)  
 
The collection which contains and associates a group of *Resource definitions* is called an *Object*, and an instance of such an Object is called an *Object Instance* (or  just an Instance). It follows that an *Object Instance* contains **actual Resource values**, whereas an Object contains only the definition of Resources.  
An Object then, is a named collection of individual Resource definitions which can be mapped directly to a device or to a software component for the purpose of data sharing.  
In the case of a device which is complex enough to have several discrete functions, multiple Objects could be defined on the same device.  
It is also possible to host multiple Instances of the same Object on a device should the need arise. Since the same principle applies to Resources, the means to address a Resource follows a semantic approach such that:

**1000/0/1**  

Addresses:  Object **1000** / Instance **0** / Resource **1** 

The strength of the above *Object/Instance/Resource* Model lies in its ability to support interoperability between unrelated devices or applications such that each exposed Resource is fully described to any function that consumes it.  
Client Objects are required to be duplicated on the LWM2M Server and/or any management application that interacts with the client via LWM2M. The Client application is responsible for updating the server Resource data when the respective Client Resource value changes. This approach ensures that any application that queries the server will have access to the most up-to-date Resource information and that the last reported state of that Resource will still be available in the event that the device hosting it goes off-line.
 
 
## IPSO object definitions

The Internet Protocol for the networking of Smart Objects (IPSO) Alliance seeks to register extensible smart object definitions based on the OMA LWM2M object resource model, with the ultimate aim of ensuring device and application interoperability via an agreed set of objects, resources, properties, attributes and operations.
Since IPSO has pre-defined data models for a wide variety of device types, based on the OMA LWM2M standard, they are readily usable by any application that implements OMA LWM2M.
 
## Awa LightweightM2M

Awa LightweightM2M is an implementation of the OMA LWM2M protocol that provides a secure and standards compliant device management solution to simplify the development of M2M applications by providing an intuitive API that enables customization without the need for an intimate knowledge of M2M protocols.  
Awa LightweightM2M provides a number of components and tools which can be combined in various ways depending on requirement. For example:

* When running on a larger Linux based device, Awa LWM2M can be deployed as a series of daemons that interact with your application via the Awa API.  
* For more constrained devices, your application code can be built against the Awa Static API and compiled along with the Awa LightweightM2M client code into a binary to be deployed on a device.  

Awa LightweightM2M is available as open source and is supported by a strong developer community. Learn more at https://github.com/FlowM2M/AwaLWM2M

----

----

