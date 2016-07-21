set (test_core_runner_SOURCES
  main.cc

  test_lwm2m_core.cc
  test_object_store_interface.cc
  test_template.cc
  test_tlv.cc
  test_definition_registry.cc
  test_plaintext.cc
  test_prettyprint.cc
  test_lwm2m_types.cc

  test_lwm2m_tree.cc
  test_lwm2m_tree_builder.cc
  unit_support.cc
  test_object_tree.cc
  test_xml.cc
  
  lwm2m_device_object.c
)

set (test_core_runner_INCLUDE_DIRS
  ${GTEST_INCLUDE_DIR}
  ${CORE_SRC_DIR}
  ${CORE_SRC_DIR}/common
  ${CORE_SRC_DIR}/client
)

set (test_core_runner_LIBRARIES
  gtest
  pthread
  awa_static
  awa_common_static
)

if (WITH_JSON)
  list (APPEND test_core_runner_SOURCES
    test_json.cc
  )
endif ()

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -g -std=c++11")
if (ENABLE_GCOV)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif ()

add_definitions (-DLWM2M_CLIENT -D__STDC_FORMAT_MACROS)

add_executable (test_core_runner ${test_core_runner_SOURCES})
target_include_directories (test_core_runner PRIVATE ${test_core_runner_INCLUDE_DIRS})
target_link_libraries (test_core_runner ${test_core_runner_LIBRARIES})

if (ENABLE_GCOV)
  target_link_libraries (test_core_runner gcov)
endif ()

# Testing
add_custom_command (
  OUTPUT test_core_runner_out.xml
  COMMAND test_core_runner --gtest_output=xml:test_core_runner_out.xml
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  VERBATIM
)

if (RUN_TESTS)
  # always run test_core_runner
  add_custom_target (
    test_core_runner_TARGET ALL
    DEPENDS test_core_runner_out.xml
  )
endif ()
