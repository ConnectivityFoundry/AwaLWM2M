# Docker Guide

Docker can be used to build and run Awa LWM2M within a container.

This is useful because it isolates the Awa build environment from your own environment. This means you do not need to install the dependencies yourself - the docker build process will do that for you. This means you do not need to install gcc, cmake, etc.

See the [Docker documentation](https://docs.docker.com) for more information about Docker.

## Building the docker image

First, install Docker as per the [Docker documentation](https://docs.docker.com/engine/installation/). This guide assumes that the Docker installation process has created a *docker* user, avoiding the need to run `sudo` with docker commands.

Clone the Awa LWM2M repository from [GitHub](https://github.com/FlowM2M/AwaLWM2M):

``` $ git clone https://github.com/FlowM2M/AwaLWM2M.git```

Run docker to build the image with:

``` $ docker build -t flowm2m/awalwm2m -f ci/Dockerfile .```

This will install all dependencies and build the Awa binaries, documentation and run the unit tests within the docker image. It will also install Awa binaries into the image.

Note: when Awa builds, it runs a suite of unit tests to check that it is functioning correctly. Many of these tests intentionally trigger errors, which will be displayed in the docker build log as red errors. These can be ignored.

## Using the pre-build docker image

A pre-built docker image is also available from the docker registry:

 TODO


## Using the container

Once you have a docker image, you may now run Awa binaries directly.

To run the bootstrap daemon (with the localhost bootstrap config):

``` $ docker run --rm -p 15685:15685/udp --name awa_bootstrapd -it flowm2m/awalwm2m awa_bootstrapd --config core/bootstrap-docker.config```

To run the server daemon:

``` $ docker run --rm -p 5683:5683/udp -p 54321:54321/udp --name awa_serverd -it flowm2m/awalwm2m awa_serverd```

To run the client daemon, which will bootstrap and register with the server:

``` $ docker run --rm -p -p 6000:6000/udp 12345:12345/udp --link awa_bootstrapd:awa_bootstrapd --link awa_serverd:awa_serverd --name awa_clientd -it flowm2m/awalwm2m awa_clientd --endpointName Client1 --bootstrap coap://awa_bootstrapd:5683```

Awa tools may also be used:

``` $ docker run --rm --link awa_serverd:ipc -it flowm2m/awalwm2m awa-server-list-clients -a ipc```

``` $ docker run --rm --link awa_clientd:ipc -it flowm2m/awalwm2m awa-client-get -a ipc /3```

``` $ docker run --rm --link awa_clientd:ipc -it flowm2m/awalwm2m awa-server-read -a ipc --clientID Client1 /3/0```


## Ease of use

Obviously having to specify all those docker parameters each time is annoying. To make this easier, create the following aliases:

``` $ alias ac docker run --rm --link awa_clientd:ipc -it flowm2m/awalwm2m```

``` $ alias as docker run --rm --link awa_serverd:ipc -it flowm2m/awalwm2m```

Now you can simply run:

``` $ as awa-server-list-clients -a ipc```

``` $ ac awa-client-get -a ipc /3```

``` $ as awa-server-read -a ipc --clientID Client1 /3/0```


## Docker Compose

 TODO
