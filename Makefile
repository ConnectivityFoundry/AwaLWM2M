##
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
#
##

# The purpose of this makefile is to encapsulate the build system (e.g. cmake)
# within a simple make interface, so that common tasks are simple to run.
#
# If desired, this makefile can be ignored and cmake run directly:
#
#   $ mkdir builddir
#   $ cd builddir
#   $ cmake ..
#

# Default location for out-of-source build results:
BUILD_DIR:=./build

# Default path prefix for installing build results:
INSTALL_PREFIX:=/

# Use 'make install DESTDIR=/path' to install into a staging directory.
# Note that the resulting path will include INSTALL_PREFIX.


###############################################################################
# Rules

ifeq ($(DEBUG),)
#  override CMAKE_OPTIONS+=-DCMAKE_BUILD_TYPE=Release
else
  override CMAKE_OPTIONS+=-DCMAKE_BUILD_TYPE=Debug
endif

override CMAKE_OPTIONS+=-DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX}

# absolute path of BUILD_DIR
BUILD_DIR_ABS:=$(shell pwd)/$(BUILD_DIR)

all: $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR) --no-print-directory
	@echo "Build complete in directory $(BUILD_DIR)"

install: all
	$(MAKE) -C $(BUILD_DIR) --no-print-directory install

$(BUILD_DIR)/Makefile:
	mkdir -p $(BUILD_DIR)
	( cd $(BUILD_DIR) && cmake .. $(CMAKE_OPTIONS) )

.PHONY: doc docs
doc docs:
	$(MAKE) -C api doc

.PHONY: test tests
test tests: all gtest_tests tool_tests

.PHONY: tool_tests tools_tests
tool_tests tools_tests:
	$(MAKE) -C tools tests BUILD_DIR=$(BUILD_DIR)

# gtest runners
TEST_SRC_BIN:=$(BUILD_DIR)/core/tests/test_core_runner
TEST_STATIC_API_BIN:=$(BUILD_DIR)/api/tests/test_static_api_runner
TEST_API_BIN:=$(BUILD_DIR)/api/tests/test_api_runner
TEST_TOOLS_BIN:=$(BUILD_DIR)/tools/tests/gtest/test_tools_runner

$(TEST_SRC_BIN): all
$(TEST_STATIC_API_BIN): all
$(TEST_API_BIN): all
$(TEST_TOOLS_BIN): all

TEST_SRC_XML:=$(TEST_SRC_BIN)_out.xml
TEST_STATIC_API_XML:=$(TEST_STATIC_API_BIN)_out.xml
TEST_API_XML:=$(TEST_API_BIN)_out.xml
TEST_TOOLS_XML:=$(TEST_TOOLS_BIN)_out.xml

ifneq ($(TEST_FILTER),)
  GTEST_OPTIONS+=--gtest_filter=$(TEST_FILTER)
endif

# determine absolute binary paths
export LWM2M_CLIENTD_BIN:=$(BUILD_DIR_ABS)/core/src/client/awa_clientd
export LWM2M_SERVERD_BIN:=$(BUILD_DIR_ABS)/core/src/server/awa_serverd
export LWM2M_BOOTSTRAPD_BIN:=$(BUILD_DIR_ABS)/core/src/bootstrap/awa_bootstrapd
export LIBCOAP_COAP_CLIENT:=$(BUILD_DIR_ABS)/lib/libcoap/examples/coap-client

# gtest-based tests:
TEST_PATHS=--clientDaemonPath=$(LWM2M_CLIENTD_BIN) \
  --serverDaemonPath=$(LWM2M_SERVERD_BIN) \
  --bootstrapDaemon=$(LWM2M_BOOTSTRAPD_BIN) \
  --coapClientPath=$(LIBCOAP_COAP_CLIENT) \
  --bootstrapConfig=api/tests/bootstrap-gtest.config

.PHONY: gtest_tests
gtest_tests: $(TEST_SRC_BIN) $(TEST_API_BIN) $(TEST_TOOLS_BIN) $(TEST_STATIC_API_BIN)
	$(TEST_SRC_BIN)        $(TEST_OPTIONS) $(GTEST_OPTIONS) --gtest_output="xml:$(TEST_SRC_XML)"        $(TEST_PATHS)
	$(TEST_STATIC_API_BIN) $(TEST_OPTIONS) $(GTEST_OPTIONS) --gtest_output="xml:$(TEST_STATIC_API_XML)" $(TEST_PATHS)
	$(TEST_API_BIN)        $(TEST_OPTIONS) $(GTEST_OPTIONS) --gtest_output="xml:$(TEST_API_XML)"        $(TEST_PATHS)
	$(TEST_TOOLS_BIN)      $(TEST_OPTIONS) $(GTEST_OPTIONS) --gtest_output="xml:$(TEST_TOOLS_XML)"      $(TEST_PATHS)

.PHONY: gdb_tests
gdb_tests: $(TEST_SRC_BIN) $(TEST_API_BIN) $(TEST_TOOLS_BIN) $(TEST_STATIC_API_BIN)
	gdb -quiet -ex=r --args $(TEST_SRC_BIN)        --gtest_break_on_failure $(GTEST_OPTIONS) $(TEST_PATHS)
	gdb -quiet -ex=r --args $(TEST_API_BIN)        --gtest_break_on_failure $(GTEST_OPTIONS) $(TEST_PATHS)
	gdb -quiet -ex=r --args $(TEST_STATIC_API_BIN) --gtest_break_on_failure $(GTEST_OPTIONS) $(TEST_PATHS)
	gdb -quiet -ex=r --args $(TEST_TOOLS_BIN)      --gtest_break_on_failure $(GTEST_OPTIONS) $(TEST_PATHS)

VALGRIND_SUPPRESSIONS:=$(realpath ci/valgrind.suppress)
VALGRIND_OPTIONS:=--error-exitcode=1 --leak-check=full --show-leak-kinds=all --track-origins=yes --suppressions=$(VALGRIND_SUPPRESSIONS)

.PHONY: valgrind_tests
valgrind_tests: $(TEST_SRC_BIN) $(TEST_API_BIN) $(TEST_TOOLS_BIN) $(TEST_STATIC_API_BIN)
	valgrind $(VALGRIND_OPTIONS) $(TEST_SRC_BIN)        $(GTEST_OPTIONS) $(TEST_PATHS)
	valgrind $(VALGRIND_OPTIONS) $(TEST_API_BIN)        $(GTEST_OPTIONS) $(TEST_PATHS)
	valgrind $(VALGRIND_OPTIONS) $(TEST_STATIC_API_BIN) $(GTEST_OPTIONS) $(TEST_PATHS)
	valgrind $(VALGRIND_OPTIONS) $(TEST_TOOLS_BIN)      $(GTEST_OPTIONS) $(TEST_PATHS)

VALGRIND_LOG_DIR:=./valgrind
VALGRIND_LOG_OPTIONS:=--xml=yes --xml-file=$(VALGRIND_LOG_DIR)/valgrind.%p.xml --log-file=$(VALGRIND_LOG_DIR)/valgrind.%p.log

.PHONY: valgrind_comprehensive
valgrind_comprehensive: $(TEST_SRC_BIN) $(TEST_API_BIN) $(TEST_TOOLS_BIN) $(TEST_STATIC_API_BIN)
	mkdir -p $(VALGRIND_LOG_DIR)
	valgrind $(VALGRIND_OPTIONS) --trace-children=yes $(VALGRIND_LOG_OPTIONS) $(TEST_SRC_BIN)        $(GTEST_OPTIONS) $(TEST_PATHS)
	valgrind $(VALGRIND_OPTIONS) --trace-children=yes $(VALGRIND_LOG_OPTIONS) $(TEST_API_BIN)        $(GTEST_OPTIONS) $(TEST_PATHS)
	valgrind $(VALGRIND_OPTIONS) --trace-children=yes $(VALGRIND_LOG_OPTIONS) $(TEST_STATIC_API_BIN) $(GTEST_OPTIONS) $(TEST_PATHS)
	valgrind $(VALGRIND_OPTIONS) --trace-children=yes $(VALGRIND_LOG_OPTIONS) $(TEST_TOOLS_BIN)      $(GTEST_OPTIONS) $(TEST_PATHS)

.PHONY: cppcheck
cppcheck:
	cppcheck core api tools lib \
        --quiet \
        --suppressions-list=ci/cppcheck.suppress --inline-suppr --suppress=missingIncludeSystem \
        --std=c99 --std=c++11 --enable=all \
        --xml --xml-version=2 2> cppcheck-result.xml

.PHONY: clean api-clean

api-clean:
	$(MAKE) -C api clean

clean: api-clean
	rm -rf $(BUILD_DIR)
	rm -rf $(VALGRIND_LOG_DIR)
	rm -f *.log
	rm -f *_out.xml
	rm -f cppcheck-result.xml
