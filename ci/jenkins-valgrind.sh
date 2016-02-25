#!/bin/bash
set -o errexit

export ac_cv_func_malloc_0_nonnull=yes
#export VERSION=$(git describe --abbrev=4 --dirty --always --tags)

# clean out old build stuff.
make clean
(cd lib && git clean -dfx)
rm -rf lib/libcoap/config.h   # still needed?
rm -rf lwm2m*.tgz
rm .build_x86 -rf
rm -rf tools/tools_tests.xml

# Build for x86 and run valgrind check
make BUILD_DIR=.build_x86
make BUILD_DIR=.build_x86 valgrind_comprehensive
