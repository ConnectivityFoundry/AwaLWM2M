
![Imagination Technologies Limited logo](images/img.png)

----


# Awa LightweightM2M



----

## Quick start guide


### Building the code


#### Building under Linux

Install the dependencies:


```
 sudo apt-get update
 sudo apt-get install build-essential gcc git mercurial python cmake python-nose python-lxml
```

Run make in the root directory of the Awa LWM2M repository


```$ make```


This will produce the following executable files:

* build/core/src/client/awa_clientd
* build/core/src/server/awa_serverd
* build/core/src/bootstrap/awa_bootstrapd


For usage information see the [User guide](userguide.md)

##### Additional commands

The variable `INSTALL_PREFIX` can be specified to modify the path where the build results will be installed *at configuration time*. For example:

```$ make INSTALL_PREFIX=/usr```

Note that this must be set at the very first invocation of make, or after `make clean`. Setting it later will not affect the install path.

By default, this is set to `/` which will result in build artefacts being installed into /bin, /lib and /include.

The variable `DESTDIR` can be set at the time `make install` is called. The final path is a combination of `INSTALL_PREFIX` and `DESTDIR`.

For example, the following will build and install such that binaries are placed in /tmp/usr/bin:

```
    $ make INSTALL_PREFIX=/usr
    $ make install DESTDIR=/tmp
```

If you want to install Awa into /usr/local/bin, the following commands are suggested:

```
    $ make
    $ make install DESTDIR=/usr/local
```

#### The cmake build


The build can be created out-of-source, such that all build artefacts are placed in a dedicated directory.


First, create a suitable directory for the build output:


```$ mkdir build ```


From this directory, run cmake, passing the path to the root directory of the repository as a parameter:

```
    $ cd build
    $ cmake ..
```


This generates a Makefile in the build directory, which can be run to generate the build artefacts:


```$ make ```


Once the build is complete you may optionally *install* the resulting binaries to a specified directory, for example:


```$ make DESTDIR=./install install ```


Cmake options can be passed to cmake from the top-level makefile with the variable `CMAKE_OPTIONS`. For example:

```$ make CMAKE_OPTIONS=-DBUILD_TESTS=OFF```


----

----
