set (test_daemon_runner_SOURCES
  main.cc

  test_xml.cc
  
  ${DAEMON_SRC_DIR}/client/lwm2m_client_xml_handlers.c
  ${DAEMON_SRC_DIR}/common/lwm2m_xml_interface.c
  ${DAEMON_SRC_DIR}/common/lwm2m_xml_serdes.c
  ${DAEMON_SRC_DIR}/common/lwm2m_ipc.c
  ${DAEMON_SRC_DIR}/common/ipc_session.c
  ${DAEMON_SRC_DIR}/common/xml.c
  ${DAEMON_SRC_DIR}/common/objdefs.c
  
    ######################## TODO REMOVE ########################
  # TODO: extract components common to both Core and API
  # FIXME: API_SRC_DIR is not in the cmake cache at the time this is read
  #${API_SRC_DIR}/path.c
  #${API_SRC_DIR}/objects_tree.c
  #${API_SRC_DIR}/log.c
  #${API_SRC_DIR}/error.c
  #${API_SRC_DIR}/utils.c
  ${CORE_SRC_DIR}/../../api/src/path.c
  ${CORE_SRC_DIR}/../../api/src/objects_tree.c
  ${CORE_SRC_DIR}/../../api/src/log.c
  ${CORE_SRC_DIR}/../../api/src/error.c
  ${CORE_SRC_DIR}/../../api/src/lwm2m_error.c
  ${CORE_SRC_DIR}/../../api/src/utils.c
  #############################################################
)

set (test_daemon_runner_INCLUDE_DIRS
  ${GTEST_INCLUDE_DIR}
  ${CORE_SRC_DIR}
  ${CORE_SRC_DIR}/common
  ${CORE_SRC_DIR}/client
  ${DAEMON_SRC_DIR}
  ${DAEMON_SRC_DIR}/common
)

set (test_daemon_runner_LIBRARIES
  gtest
  pthread
  awa_static
  awa_common_static
  libxml_static
  libb64_static
)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -g -std=c++11")
if (ENABLE_GCOV)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif ()

add_definitions (-DLWM2M_CLIENT -D__STDC_FORMAT_MACROS)

add_executable (test_daemon_runner ${test_daemon_runner_SOURCES})
target_include_directories (test_daemon_runner PRIVATE ${test_daemon_runner_INCLUDE_DIRS})
target_link_libraries (test_daemon_runner ${test_daemon_runner_LIBRARIES})

if (ENABLE_GCOV)
  target_link_libraries (test_daemon_runner gcov)
endif ()

# Testing
add_custom_command (
  OUTPUT test_daemon_runner_out.xml
  COMMAND test_daemon_runner --gtest_output=xml:test_daemon_runner_out.xml
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  VERBATIM
)

if (RUN_TESTS)
  # always run test_daemon_runner
  add_custom_target (
    test_daemon_runner_TARGET ALL
    DEPENDS test_daemon_runner_out.xml
  )
endif ()
