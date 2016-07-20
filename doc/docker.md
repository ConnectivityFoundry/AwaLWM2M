![](images/img.png)
----

# Using Awa with Docker

Docker can be used to build and run Awa LWM2M within a container, which is useful because it isolates the Awa build environment from your own environment. You don't have to install the dependencies, (gcc, cmake, etc.) yourself, the docker build process will do that.

For more information about Docker, see the [Docker documentation](https://docs.docker.com).

## Building the docker image

First install Docker as per the [Docker installation notes](https://docs.docker.com/engine/installation/). This guide assumes that the installation process has created a *docker user*, avoiding the need to run `sudo` with docker commands.

Now clone the Awa LWM2M repository from [GitHub](https://github.com/FlowM2M/AwaLWM2M):

``` $ git clone https://github.com/FlowM2M/AwaLWM2M.git```

Alternatively, if you're using an existing repository, ensure you do ```make clean``` first.

Run docker to build the image with:

``` $ docker build -t flowm2m/awalwm2m -f ci/Dockerfile .```

This will install all dependencies and build the Awa binaries and documentation, and run the unit tests within the docker image. It will also install Awa binaries into the image.

**Note.** *When Awa builds, it runs a suite of unit tests to check that it is functioning correctly. Several of the tests intentionally trigger errors which will be displayed in the docker build log as red errors. These can be ignored.*

## Using the container

Once you have a docker image, you can run Awa binaries directly.

To run the bootstrap daemon (with the localhost bootstrap config), use:

``` $ docker run --rm -p 15685:15685/udp --name awa_bootstrapd -it flowm2m/awalwm2m awa_bootstrapd --config config/docker.bsc```

To run the server daemon:

``` $ docker run --rm -p 5683:5683/udp -p 54321:54321/udp --name awa_serverd -it flowm2m/awalwm2m awa_serverd```

To run the client daemon, which will bootstrap and register with the server:

``` $ docker run --rm -p 6000:6000/udp -p 12345:12345/udp --link awa_bootstrapd:awa_bootstrapd --link awa_serverd:awa_serverd --name awa_clientd -it flowm2m/awalwm2m awa_clientd --endPointName Client1 --bootstrap coap://awa_bootstrapd:15685```

Awa tools may also be used:

``` $ docker run --rm --link awa_serverd:ipc -it flowm2m/awalwm2m awa-server-list-clients -a ipc```

``` $ docker run --rm --link awa_clientd:ipc -it flowm2m/awalwm2m awa-client-get -a ipc /3```

``` $ docker run --rm --link awa_serverd:ipc -it flowm2m/awalwm2m awa-server-read -a ipc --clientID Client1 /3/0```

The `--link awa_serverd:ipc` option creates a DNS name `ipc` within the container, that is used by the tool to locate the associated daemon's IPC service.

## Making things easier

Having to specify all the above docker parameters each time is a little tedious. To make things easier, create the following aliases:

``` $ alias ac='docker run --rm --link awa_clientd:ipc -it flowm2m/awalwm2m'``` - for client operations.  

``` $ alias as='docker run --rm --link awa_serverd:ipc -it flowm2m/awalwm2m'``` - for server operations.  

Now you can simply run:

``` $ as awa-server-list-clients -a ipc```

``` $ ac awa-client-get -a ipc /3```

``` $ as awa-server-read -a ipc --clientID Client1 /3/0```

Another option is to run the container as a shell and run the commands directly. You must still use the -a option to specify the address of the associated daemon:

```
$ docker run --rm --link awa_serverd:is --link awa_clientd:ic -it flowm2m/awalwm2m
# awa-server-list-clients -a is
# awa-client-get -a ic /3
```

----

----
