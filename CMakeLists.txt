cmake_minimum_required (VERSION 2.8.12.2)

project (lwm2m)

# Add *global* compiler definitions like this:
#add_definitions (-DMY_DEFINITION=1)

# enable make debugging (or use 'make VERBOSE=1')
#set (CMAKE_VERBOSE_MAKEFILE ON)

enable_testing ()

# TODO: set TARGET_CONFIGURE_OPTS from environment?


# Options for supported CoAP and DTLS libraries:
option (WITH_LIBCOAP "Enable libcoap CoAP support)" OFF)
option (WITH_ERBIUM "Enable Erbium CoAP support)" ON)
option (WITH_GNUTLS "Enable GnuTLS DTLS support" OFF)
option (WITH_CYASSL "Enable CyaSSL DTLS support" OFF)
option (WITH_TINYDTLS "Enable TinyDTLS DTLS support" OFF)
option (WITH_MBEDTLS "Enable mbedTLS DTLS support" OFF)
option (WITH_SYSTEMD "Install systemd service files" OFF)


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if (WITH_LIBCOAP)
  message (WARNING "Disabling erbium support")
  set (WITH_ERBIUM OFF)
endif ()

if ((WITH_GNUTLS AND WITH_CYASSL) OR (WITH_GNUTLS AND WITH_TINYDTLS) OR (WITH_CYASSL AND WITH_TINYDTLS))
  message (FATAL_ERROR "WITH_GNUTLS, WITH_CYASSL and WITH_TINYDTLS are mutually exclusive and cannot be specified together." )
endif ()


# build tests, but disable running by default
option (BUILD_TESTS "Build tests" ON)
option (RUN_TESTS "Enable tests" OFF)
option (WITH_JSON "Enable JSON support" OFF)
# cmdline: cmake -DBUILD_TESTS=ON -DRUN_TESTS=OFF ..

# build examples
option (BUILD_EXAMPLES "Build examples" ON)


option (ENABLE_GCOV "Enable code coverage check" OFF)
# cmake -DENABLE_GCOV=ON ..

# can't run tests if you don't build them:
if (RUN_TESTS)
  set (BUILD_TESTS ON)
endif ()

if ((CMAKE_CROSSCOMPILING) OR (CMAKE_BUILD_TYPE STREQUAL "Release"))
  set (BUILD_TESTS OFF)
  set (RUN_TESTS OFF)
  set (ENABLE_GCOV OFF)
endif ()

# determine git revision as VERSION
#execute_process (COMMAND git describe --abbrev=4 --dirty --always --tags OUTPUT_VARIABLE VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
file (STRINGS "VERSION" VERSION)

set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
# enable debugging info for all components
  add_compile_options (-g)
endif ()

# build libraries
add_subdirectory (lib)

# build core daemons first, as API tests require them
add_subdirectory (core)
add_subdirectory (daemon)
add_subdirectory (api)
add_subdirectory (tools)

# install systemd files
if (WITH_SYSTEMD)
  install (
    DIRECTORY systemd/
    DESTINATION /
  )
endif ()
