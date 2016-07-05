set (API_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "API_TEST_DIR")

set (test_api_runner_SOURCES
  support/main/main.cc
  support/main/main_cmdline.c
  support/support.cc
  support/process.cc
  support/daemon.cc
  support/static_api_support.cc
  support/log.cc

  common/test_log.cc
  common/test_error.cc
  common/test_lwm2m_error.cc
  common/test_ipc.cc
  common/test_utils.cc
  common/test_iterator.cc
  common/test_path.cc
  common/test_string_iterator.cc
  common/test_path_iterator.cc
  common/test_path_result.cc
  common/test_array_types.cc
  common/test_objects_tree.cc
  common/test_queue.cc
  common/test_list.cc
  common/test_map.cc
  common/test_xmltree.cc
  common/test_operation_common.cc
  common/test_response_common.cc
  common/test_define_common.cc
  common/test_session_common.cc
  common/test_subscribe_observe_common.cc
  common/test_define_operation.cc
  common/test_value.cc
  common/test_bootstrap.cc
  common/test_daemon.cc
  common/test_client_server_interaction.cc
  
  client/test_client.cc
  client/test_client_session.cc
  client/test_client_define_operation.cc
  client/test_client_define_defaults.cc
  client/test_set_operation.cc
  client/test_get_operation.cc
  client/test_client_delete_operation.cc
  client/test_subscribe_operation.cc
  client/test_client_iterator.cc
  client/test_client_objdefs_from_file.cc

  server/test_server_session.cc
  server/test_server_define_operation.cc
  server/test_server_define_defaults.cc
  server/test_list_clients_operation.cc
  server/test_write_operation.cc
  server/test_set_write_common.cc
  server/test_read_operation.cc
  server/test_write_attributes_operation.cc
  server/test_changeset.cc 
  server/test_server_delete_operation.cc
  server/test_execute_operation.cc
  server/test_server_events.cc
  server/test_server_response.cc
  server/test_server.cc
  server/test_observe_operation.cc
  server/test_server_notification.cc
  server/test_server_objdefs_from_file.cc
)

set (test_static_api_runner_SOURCES
  support/main/main.cc
  support/main/main_cmdline.c
  support/support.cc
  support/process.cc
  support/daemon.cc
  support/static_api_support.cc
  support/log.cc

  static-client/test_static_api.cc
  static-client/test_static_api_with_pointer.cc
  static-client/test_static_api_handlers.cc
  static-client/test_static_api_get_resource_instance_pointer.cc
  static-client/test_static_api_create_delete.cc
)


set (test_api_runner_INCLUDE_DIRS
  ${GTEST_INCLUDE_DIR}
  ${API_INCLUDE_DIR}
  ${API_SRC_DIR}
  ${CORE_SRC_DIR}
  ${CORE_SRC_DIR}/common
  ${CORE_SRC_DIR}/client
  ${XML_INCLUDE_DIR}
  ${API_TEST_DIR}
)

set (test_static_api_runner_INCLUDE_DIRS
  ${GTEST_INCLUDE_DIR}
  ${API_INCLUDE_DIR}
  ${API_SRC_DIR}
  ${CORE_SRC_DIR}
  ${CORE_SRC_DIR}/common
  ${XML_INCLUDE_DIR}
  ${API_TEST_DIR}
)

set (test_api_runner_LIBRARIES
  gtest
  pthread
  awa_static
  awa_common_static
  Awa_static
  libb64_static
)

set (test_static_api_runner_LIBRARIES
  gtest
  pthread
  awa_static
  awa_common_static
  Awa_static
  libb64_static
)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -g -std=c++11")
if (ENABLE_GCOV)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif ()

add_definitions (-D__STDC_FORMAT_MACROS)
add_definitions (-DVERSION="${VERSION}")

# Disable format-extra-args for this source file:
set_source_files_properties (common/test_utils.cc PROPERTIES COMPILE_FLAGS -Wno-format-extra-args)

set_source_files_properties (main/main_cmdline.c PROPERTIES COMPILE_FLAGS -Wno-all)

add_executable (test_api_runner ${test_api_runner_SOURCES})
target_include_directories (test_api_runner PRIVATE ${test_api_runner_INCLUDE_DIRS})
target_link_libraries (test_api_runner ${test_api_runner_LIBRARIES})

add_executable (test_static_api_runner ${test_static_api_runner_SOURCES})
target_include_directories (test_static_api_runner PRIVATE ${test_static_api_runner_INCLUDE_DIRS})
target_link_libraries (test_static_api_runner ${test_static_api_runner_LIBRARIES})

if (ENABLE_GCOV)
  target_link_libraries (test_api_runner gcov)
endif ()

# Testing
add_custom_command (
  OUTPUT test_api_runner_out.xml
  COMMAND test_api_runner --gtest_output=xml:test_api_runner_out.xml || true
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  VERBATIM
)

add_custom_command (
  OUTPUT test_static_api_runner_out.xml
  COMMAND test_static_api_runner --gtest_output=xml:test_static_api_runner_out.xml || true
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  VERBATIM
)

if (RUN_TESTS)
  # always run test_api_runner
  add_custom_target (
    test_api_runner_TARGET ALL
    DEPENDS test_api_runner_out.xml
  )
  
  add_custom_target (
    test_static_api_runner_TARGET ALL
    DEPENDS test_static_api_runner_out.xml
  )
endif ()



