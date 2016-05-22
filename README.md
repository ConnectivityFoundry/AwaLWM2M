
![Imagination Technologies Limited logo](doc/img.png)

----

## Awa LightweightM2M.

[![Build Status](https://travis-ci.org/FlowM2M/AwaLWM2M.svg?branch=master)](https://travis-ci.org/FlowM2M/AwaLWM2M)
[![License (3-Clause BSD)](https://img.shields.io/badge/license-BSD%203--Clause-blue.svg?style=flat-square)](http://opensource.org/licenses/BSD-3-Clause)
[![Coverity Scan](https://scan.coverity.com/projects/8186/badge.svg)](https://scan.coverity.com/projects/flowm2m-awalwm2m)

The Internet of Things (IoT) is currently fragmented by numerous proprietary methods of device management. In order for the IoT to reach its full potential, devices from different vendors must be able to communicate effectively with each other. This is where open standards such as the Open Mobile Alliance's (OMA) Lightweight Machine to Machine protocol (LWM2M) become important.

The LWM2M protocol has been designed to be highly efficient in terms of data transfer and memory footprint, making it  suitable for deployment on larger gateway devices as well as the more constrained devices.

Awa LWM2M is an implementation of the OMA Lightweight M2M protocol that provides a secure and standards compliant device management solution to simplify the development of M2M applications by providing an intuitive API that enables customization without the need for an intimate knowledge of M2M protocols.

![Awa application overview](doc/Awa_application_overview.png)

Awa LWM2M is a development suite that provides a number of components and tools which can be combined in various ways depending on requirement. For example:


* When running on a larger Linux based device, Awa LWM2M can be deployed as a series of daemons that interact with your application via the Awa API.
* For more constrained devices, your application code can be built against the [Awa static API](doc/Awa_Static_API.md) and compiled along with the Awa LWM2M client code into a binary to be deployed on your device.

*Regardless of the method, adding LWM2M support for your device is simply a matter of incorporating any objects you need into your own M2M application.*

----

###  Getting started.

The easiest way to get started with Awa LWM2M is on a Linux PC.  The following instructions are based on the Ubuntu Linux distribution and assume that the user is familiar with the GNU compiler toolchain, and with the process of installing packages using the package manager.

Firstly, to obtain a copy of the Awa LWM2M source code:

 * Sign up for a Github account

 * Install Git:  ```` sudo apt-get install git ````

 * Clone the repository: ```` git clone https://github.com/FlowM2M/AwaLWM2M.git ````


Further information can be found in the [Quick start guide](doc/starters_guide.md).


----

### Documentation.

This project assumes a basic knowledge of The Open Mobile Alliance's (OMA) LWM2M, its functionality and services. For more detail, see our [introductory LWM2M overview](doc/lwm2m_overview.md).

Awa LWM2M documentation is available both at a general level (project information, user and developer guides), and a technical level (the API guide). All documentation is available in this repository. The *doc* directory contains information relating to the project in general, and the *api/doc* directory contains the lower level documentation for the Awa API.

Note that for our purposes the terms *user* and *developer* have the following definitions:

* *User* - An M2M application developer who uses the tools and libraries supplied by this project as the foundation of, or enhancement to, their own M2M application.
* *Developer* - A developer who develops for and contributes to the Awa LightweightM2M project.


#### General documentation.

* For project users:

	* For build instructions, see the [Quick start guide](doc/starters_guide.md).
	* Examples of how to use the tools can be found in the [User guide](doc/userguide.md).
	* A sample application tutorial can be found [here](doc/example_app.md).

* For contributors:

	* An overview of the system can be found in the [Contributing developer guide](doc/developer_guide.md).
	* Information regarding the testing framework, can be found in the [Testing](doc/testing.md) guide.

#### API guide.

The Awa API documentation is available as a Doxygen presentation which is generated via the following process.

  1. Install [Doxygen ](http://www.stack.nl/~dimitri/doxygen/download.html): ```` sudo apt-get install doxygen````

  2. Generate the documentation: ```` make docs````

The output can be found in the api/doc/html directory and viewed by opening index.html with your web browser.

For convenience you can also find the latest version of this documentation [here](http://flowm2m.github.io/AwaLWM2M-docs/index.html).

----

### Contributing.

We welcome all contributions to this project and we give credit where it's due. Anything from enhancing functionality to improving documentation and bug reporting - it's all good.

Find out more in the [contributor guide](CONTRIBUTING.md).

### Credits.

We would like to thank all of our current [contributors](CONTRIBUTORS).

We would also like to acknowledge and thank the authors of the following projects.

* libcoap : http://sourceforge.net/projects/libcoap/
* googletest : https://code.google.com/p/googletest/
* jsmn : https://github.com/zserge/jsmn


### Development tasks.

A list of ongoing development tasks can be seen [here](https://github.com/FlowM2M/AwaLWM2M/wiki/Development-task-list).


----

### License information.

* All code and documentation developed by Imagination Technologies Limited is licensed under the [BSD 3-clause license](LICENSE).
* LibCoAP by Olaf Bergmann is licensed under the GNU General Public License (GPL), Version 2 or higher, OR the simplified BSD license.
* Jsmn by Serge A. Zaitsev is licensed under the MIT license.

----


----
----
