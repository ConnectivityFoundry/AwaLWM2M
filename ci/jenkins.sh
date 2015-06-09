#!/bin/bash

set -o errexit

export PATH=$PATH:/opt/microchip/xc32/v1.34/bin
#export PATH=/opt/rh/devtoolset-2/root/usr/bin:$PATH
export PATH=/repos/creator-git/dists/openwrt/staging_dir/target-mipsel_mips32_uClibc-0.9.33.2/host/bin:/repos/creator-git/dists/openwrt/staging_dir/toolchain-mipsel_mips32_gcc-4.9-linaro_uClibc-0.9.33.2/bin:/repos/creator-git/dists/openwrt/staging_dir/toolchain-mipsel_mips32_gcc-4.9-linaro_uClibc-0.9.33.2/bin:/repos/creator-git/dists/openwrt/staging_dir/host/bin:/repos/creator-git/dists/openwrt/staging_dir/host/bin:$PATH
export STAGING_DIR="/repos/creator-git/dists/openwrt/staging_dir/target-mipsel_mips32_uClibc-0.9.33.2"
export ac_cv_func_malloc_0_nonnull=yes
#export VERSION=$(git describe --abbrev=4 --dirty --always --tags)

# clean out old build stuff.
make clean
(cd lib && git clean -dfx)
rm -rf lib/libcoap/config.h   # still needed?
rm -rf lwm2m*.tgz
rm .build_openwrt -rf
rm .build_x86 -rf
rm -rf tests/lwm2m_ipc/nosetests.xml
rm -rf tools/tools_tests.xml

# Build for OpenWRT
make BUILD_DIR=.build_openwrt CMAKE_OPTIONS=-DCMAKE_TOOLCHAIN_FILE=ci/openwrt-toolchain.cmake

# setup lcov
mkdir .build_x86
(cd .build_x86; lcov --zerocounters --directory .; lcov --capture --initial --directory . --output-file test_lwm2m || true)

# Build for x86 and run test cases
make BUILD_DIR=.build_x86 CMAKE_OPTIONS="-DENABLE_GCOV=ON"
make BUILD_DIR=.build_x86 CMAKE_OPTIONS="-DENABLE_GCOV=ON" tests

# Python LWM2M tests
(cd tests
  # don't stop build on failure:
  (cd lwm2m && BUILD_DIR=../../.build_x86 ./auto_run_lwm2m_server_testcases || true)
  (cd lwm2m && BUILD_DIR=../../.build_x86 ./auto_run_lwm2m_client_testcases || true)
)

# parse coverage results
(cd .build_x86;
 lcov --no-checksum --directory . --capture --output-file tmp_test_lwm2m.info; 
 lcov --remove tmp_test_lwm2m.info "api/tests/*" --remove tmp_test_lwm2m.info "api/src/unsupported*" --output-file test_lwm2m.info;
 mkdir -p lcov-html; 
 cd lcov-html; genhtml ../test_lwm2m.info)

make cppcheck BUILD_DIR=.build_x86
