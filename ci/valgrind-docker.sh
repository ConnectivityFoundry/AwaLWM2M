#!/bin/bash
# Script to build AwaLWM2M and execute Valgrind within a docker container

# only enable Docker pseudoterminal if a TTY is present:
if [ -t 1 ]; then
  TERMINAL_OPT=-t
else
  TERMINAL_OPT=
fi

# create/update the build environment image:
docker build -t flowm2m/awalwm2m.ci -f ci/Dockerfile.ci .

# use the image to run the Valgrind process:
docker run \
  -v $(pwd):/home/build/AwaLWM2M \
  -e CMAKE_OPTIONS="-DWITH_LIBCOAP=ON" \
  -w /home/build/AwaLWM2M \
  -i $TERMINAL_OPT \
  flowm2m/awalwm2m.ci \
  ci/valgrind.sh
