# Docker Guide

Docker can be used to build and run Awa LWM2M.

This is useful because it isolates the Awa build environment from your own environment. This means you do not need to install the dependencies yourself - the Docker build process will do that for you. This means you do not need to install gcc, cmake, etc.

## Building the docker image

First, install Docker as per the [Docker documentation](https://docs.docker.com/engine/installation/). This guide assumes that the Docker installation process has created a **docker** user, avoiding the need to run `sudo` with docker commands.

Check out the Awa LWM2M repository from [GitHub](https://github.com/FlowM2M/AwaLWM2M):

``` $ git clone https://github.com/FlowM2M/AwaLWM2M.git```

Run docker to build the image with:

``` $ docker build -t flowm2m/awalwm2m -f ci/Dockerfile```

This will install all dependencies and build the Awa binaries, documentation and run the unit tests. It will also install Awa binaries into the container.


## Using the pre-build docker image

TODO


## Using the container

Once you have a docker image, you may now run Awa binaries directly.

To run the bootstrap daemon (with the localhost config):

``` $ docker run -it flowm2m/awalwm2m awa_bootstrapd --config core/bootstrap-localhost.config```

To run the server daemon:

``` $ docker run -it flowm2m/awalwm2m awa_serverd```

To run the client daemon, which will bootstrap and register with the server:

``` $ docker run -it flowm2m/awalwm2m awa_clientd --endpointName Test1 --bootstrap coap://127.0.0.1:5683```

Awa tools can also be run:

``` $ docker run -it flowm2m/awalwm2m awa-client-get /3```

``` $ docker run -it flowm2m/awalwm2m awa-server-list-clients```

``` $ docker run -it flowm2m/awalwm2m awa-server-read --clientID Test1 /3/0```


See the [Docker documentation](https://docs.docker.com) for more information about Docker.
