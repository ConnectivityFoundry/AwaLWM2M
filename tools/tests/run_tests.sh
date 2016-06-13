#!/bin/bash

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

if [ x$BUILD_DIR == "x" ] ; then
  # assume a sensible default for local development:
  BUILD_DIR=../../build
fi

BUILD_DIR_ABS=$PWD/$BUILD_DIR

if [ x"${LWM2M_TOOLS_PATH}" == "x" ] ; then
  LWM2M_TOOLS_PATH=$BUILD_DIR_ABS/tools
fi

if ! [[ "$LWM2M_TOOLS_PATH" = /* ]] ; then
  echo "LWM2M_TOOLS_PATH should be set to an absolute path"
  exit 1
fi

if [ x"${LWM2M_CLIENTD_BIN}" == "x" ] ; then
  LWM2M_CLIENTD_BIN=$BUILD_DIR_ABS/core/src/client/awa_clientd
fi

if ! [[ "$LWM2M_CLIENTD_BIN" = /* ]] ; then
  echo "LWM2M_CLIENTD_BIN should be set to an absolute path"
  exit 1
fi

if [ x"${LWM2M_SERVERD_BIN}" == "x" ] ; then
  LWM2M_SERVERD_BIN=$BUILD_DIR_ABS/core/src/server/awa_serverd
fi

if ! [[ "$LWM2M_SERVERD_BIN" = /* ]] ; then
  echo "LWM2M_SERVERD_BIN should be set to an absolute path"
  exit 1
fi

if [ x"${LWM2M_BOOTSTRAPD_BIN}" == "x" ] ; then
  LWM2M_BOOTSTRAPD_BIN=$BUILD_DIR_ABS/core/src/bootstrap/awa_bootstrapd
fi

if ! [[ "$LWM2M_BOOTSTRAPD_BIN" = /* ]] ; then
  echo "LWM2M_BOOTSTRAPD_BIN should be set to an absolute path"
  exit 1
fi

export LWM2M_TOOLS_PATH
export LWM2M_CLIENTD_BIN
export LWM2M_SERVERD_BIN
export LWM2M_BOOTSTRAPD_BIN

echo "Using Environment Variables:"
echo "  LWM2M_TOOLS_PATH=$LWM2M_TOOLS_PATH"
echo "  LWM2M_CLIENTD_BIN=$LWM2M_CLIENTD_BIN"
echo "  LWM2M_SERVERD_BIN=$LWM2M_SERVERD_BIN"
echo "  LWM2M_BOOTSTRAPD_BIN=$LWM2M_BOOTSTRAPD_BIN"

PYTHONPATH=../../api/python:$PYTHONPATH nosetests -w python $@
