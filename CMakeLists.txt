cmake_minimum_required (VERSION 2.8.12.2)

project (lwm2m)

# Add *global* compiler definitions like this:
#add_definitions (-DMY_DEFINITION=1)

# enable make debugging (or use 'make VERBOSE=1')
#set (CMAKE_VERBOSE_MAKEFILE ON)

enable_testing ()

# TODO: set TARGET_CONFIGURE_OPTS from environment?

# build tests, but disable running by default
option (BUILD_TESTS "Build tests" ON)
option (RUN_TESTS "Enable tests" OFF)
option (WITH_JSON "Enable JSON support" ON)
# cmdline: cmake -DBUILD_TESTS=ON -DRUN_TESTS=OFF ..

option (ENABLE_GCOV "Enable code coverage check" OFF)
# cmake -DENABLE_GCOV=ON ..

# can't run tests if you don't build them:
if (RUN_TESTS)
  set (BUILD_TESTS ON)
endif ()

if (CMAKE_CROSSCOMPILING)
  set (BUILD_TESTS OFF)
  set (RUN_TESTS OFF)
  set (ENABLE_GCOV OFF)
endif ()

# determine git revision as VERSION
#execute_process (COMMAND git describe --abbrev=4 --dirty --always --tags OUTPUT_VARIABLE VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
file (STRINGS "VERSION" VERSION)

# enable debugging info for all components
# TODO: separate debug and release builds
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb")
add_compile_options (-g)

# build libraries
add_subdirectory (lib)

# build core daemons first, as API tests require them
add_subdirectory (core)
add_subdirectory (api)
add_subdirectory (tools)

# install systemd files
install (
  DIRECTORY systemd/
  DESTINATION /
)
