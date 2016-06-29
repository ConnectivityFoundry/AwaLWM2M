#!/bin/bash
# Script to build AwaLWM2M and execute CI within a docker container
#
#  Define DOCKER_BUILD_EXTRA_OPTIONS if you wish to add/override image build options.
#  Define DOCKER_RUN_EXTRA_OPTIONS if you wish to add/override container run options.
#

# only enable Docker pseudoterminal if a TTY is present:
if [ -t 1 ]; then
  TERMINAL_OPT=-t
else
  TERMINAL_OPT=
fi

# create/update the build environment image:
docker build \
  -t flowm2m/awalwm2m.ci \
  -f ci/Dockerfile.ci \
  $DOCKER_BUILD_EXTRA_OPTIONS \
  .

# use the image to run the CI process:
docker run \
  -v $(pwd):/home/build/AwaLWM2M \
  -w /home/build/AwaLWM2M \
  -i $TERMINAL_OPT \
  $DOCKER_RUN_EXTRA_OPTIONS \
  flowm2m/awalwm2m.ci \
  ci/ci.sh $@
