#!/bin/bash -x

#/************************************************************************************************************************
# Copyright (c) 2016, Imagination Technologies Limited and/or its affiliated group companies.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
# following conditions are met:
#     1. Redistributions of source code must retain the above copyright notice, this list of conditions and the
#        following disclaimer.
#     2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
#        following disclaimer in the documentation and/or other materials provided with the distribution.
#     3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote
#        products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
# USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#************************************************************************************************************************/

set -o errexit

BUILD_DIR=build

# clean out existing build artefacts
make clean
rm -rf $BUILD_DIR
rm -rf tools/tools_tests.xml

# concatenate with any existing CMAKE_OPTIONS environment variable:
#   ENABLE_GCOV=ON  :  enable gcov profiling
#
CMAKE_OPTIONS="$CMAKE_OPTIONS -DENABLE_GCOV=ON"

if [ x$BUILD_OPENWRT != "x" ]
then
  # build for OpenWRT
  export PATH=$PATH:/opt/microchip/xc32/v1.34/bin
  export PATH=/repos/creator-git/dists/openwrt/staging_dir/target-mipsel_mips32_uClibc-0.9.33.2/host/bin:/repos/creator-git/dists/openwrt/staging_dir/toolchain-mipsel_mips32_gcc-4.9-linaro_uClibc-0.9.33.2/bin:/repos/creator-git/dists/openwrt/staging_dir/toolchain-mipsel_mips32_gcc-4.9-linaro_uClibc-0.9.33.2/bin:/repos/creator-git/dists/openwrt/staging_dir/host/bin:/repos/creator-git/dists/openwrt/staging_dir/host/bin:$PATH
  export STAGING_DIR="/repos/creator-git/dists/openwrt/staging_dir/target-mipsel_mips32_uClibc-0.9.33.2"
  export ac_cv_func_malloc_0_nonnull=yes
  rm -rf build.openwrt
  make BUILD_DIR=build.openwrt CMAKE_OPTIONS=-DCMAKE_TOOLCHAIN_FILE=ci/openwrt-toolchain.cmake
fi

# setup lcov
LCOV_TRACEFILE=awa_test.info
mkdir -p $BUILD_DIR
(
  cd $BUILD_DIR
  lcov --rc lcov_branch_coverage=1 --zerocounters --directory .
  lcov --rc lcov_branch_coverage=1 --capture --initial --directory . --output-file $LCOV_TRACEFILE || true
)

# build for x86 and run test cases
make BUILD_DIR=$BUILD_DIR CMAKE_OPTIONS="$CMAKE_OPTIONS"
make BUILD_DIR=$BUILD_DIR CMAKE_OPTIONS="$CMAKE_OPTIONS" tests

# prepare coverage results
(
  cd $BUILD_DIR
  LCOV_TRACEFILE_FILTERED=$(basename -s .info $LCOV_TRACEFILE)_filtered.info
  lcov --rc lcov_branch_coverage=1 --no-checksum --directory . --capture --output-file $LCOV_TRACEFILE
  lcov --rc lcov_branch_coverage=1 --remove $LCOV_TRACEFILE \
       "api/tests/*" "api/src/unsupported*" "core/tests/*" "/usr/*" "$BUILD_DIR/*-src/*" "*_cmdline.c" \
       --output-file $LCOV_TRACEFILE_FILTERED
  mkdir -p lcov-html
  (
    cd lcov-html
    genhtml --rc genhtml_branch_coverage=1 ../$LCOV_TRACEFILE_FILTERED
  )

  # prepare cobertura coverage results
  python ../ci/lcov_cobertura.py $LCOV_TRACEFILE_FILTERED -b ../
)

# run cppcheck
make cppcheck BUILD_DIR=$BUILD_DIR

# build docs
make docs

# legacy CI support
ln -sfn $BUILD_DIR .build_x86
