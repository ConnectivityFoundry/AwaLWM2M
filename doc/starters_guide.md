
![Imagination Technologies Limited logo](img.png)

----


# Awa LightweightM2M.



----

## Quick start guide.


### Building the code.


#### Building under Linux.

Install the dependencies:


```
 sudo apt-get update
 sudo apt-get install build-essential gcc git mercurial python cmake python-nose python-lxml  
```

Run make in the root directory of the Awa LWM2M repository


````$ make````


This will produce the following executable files:

* build/core/src/client/awa_clientd
* build/core/src/server/awa_serverd
* build/core/src/bootstrapd/awa_bootstrapd

 
For usage information see the [User guide](userguide.md)


#### The cmake build.


The build can be created out-of-source, such that all build artefacts are placed in a dedicated directory.


First, create a suitable directory for the build output:


````$ mkdir build ````


From this directory, run cmake, passing the path to the root directory of the repository as a parameter:
````

    $ cd build
    $ cmake ..
````


This generates a Makefile in the build directory, which can be run to generate the build artefacts:


````$ make ````


Once the build is complete you may optionally *install* the resulting binaries to a specified directory, for example:


````$ make DESTDIR=./install install ````


----

----

