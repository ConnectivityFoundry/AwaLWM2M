FROM ubuntu:16.04
MAINTAINER David Antliff <david.antliff@imgtec.com>

ENV DEBIAN_FRONTEND noninteractive

# install package dependencies
RUN apt-get update -yq && apt-get install -yq \
	apt-utils \
	git \
	curl \
	make \
	build-essential \
	libssl-dev \
	zlib1g-dev \
	libbz2-dev \
	libreadline-dev \
	libxml2-dev \
	libxslt-dev \
	doxygen \
	graphviz \
	cmake
# apt-get clean is automatic for Ubuntu images

# install pyenv
RUN useradd -m build
WORKDIR /home/build
ENV HOME /home/build
USER build

RUN git clone https://github.com/yyuu/pyenv.git .pyenv
ENV PYENV_ROOT $HOME/.pyenv
ENV PATH $PYENV_ROOT/shims:$PYENV_ROOT/bin:$PATH

# install python 2.7.11 within pyenv, pip
RUN pyenv install 2.7.11 && pyenv global 2.7.11 && pyenv rehash

# copy in Awa sources
USER root
COPY . $HOME/AwaLWM2M
RUN chown build:build -R $HOME/AwaLWM2M

# build AwaLWM2M and tests
USER build
WORKDIR $HOME/AwaLWM2M
RUN pip install -r ci/requirements.txt
RUN make

# build docs
RUN make docs

# run tests
RUN make tests

# install
USER root
RUN make install
