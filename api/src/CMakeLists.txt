set (API_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "API_SRC_DIR")

set (Awa_SOURCES
  log.c
  error.c
  lwm2m_error.c
  ipc.c
  utils.c
  changeset.c
  client_subscribe_response.c
  response_common.c
  server_response.c
  iterator.c
  string_iterator.c
  path_iterator.c
  client_iterator.c
  registered_entity_iterator.c
  path.c
  path_result.c
  arrays.c
  queue.c
  list.c
  value.c

  objects_tree.c
  define_common.c

  integer_array.c
  float_array.c
  string_array.c
  opaque_array.c
  time_array.c
  boolean_array.c
  objectlink_array.c
  map.c
  
  session_common.c
  server_session.c
  client_session.c
  
  operation_common.c
  server_operation.c
  set_write_common.c
  define_operation.c
  client_define_operation.c
  server_define_operation.c
  set_operation.c
  get_operation.c
  client_delete_operation.c
  server_delete_operation.c
  get_response.c
  client_subscribe.c
  client_notification.c
  server_notification.c
  observe_operation.c
  list_clients_operation.c
  write_operation.c
  write_mode.c
  read_operation.c
  subscribe_observe_common.c
  write_attributes_operation.c
  execute_operation.c
  server_events.c

  unsupported.c

  ${CORE_SRC_DIR}/common/xml.c
  ${CORE_SRC_DIR}/common/lwm2m_definition.c
  ${CORE_SRC_DIR}/common/lwm2m_list.c
  ${CORE_SRC_DIR}/common/lwm2m_types.c
  ${CORE_SRC_DIR}/common/lwm2m_result.c
  ${CORE_SRC_DIR}/common/lwm2m_debug.c
  ${CORE_SRC_DIR}/common/lwm2m_tree_node.c
  ${CORE_SRC_DIR}/common/lwm2m_xml_serdes.c
)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Werror -D_GNU_SOURCE")

if (ENABLE_GCOV)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif ()

# Virtual library to avoid building .o files twice:

# fetch the INCLUDE_DIRECTORIES properties of non-linked dependencies:
# (it is not possible to link with an OBJECT library, so these are not automatic)
get_property (LIB_XML_INCLUDE_DIR TARGET libxml_static PROPERTY INCLUDE_DIRECTORIES)
get_property (LIB_B64_INCLUDE_DIR TARGET libb64_static PROPERTY INCLUDE_DIRECTORIES)

add_library (Awa_object OBJECT ${Awa_SOURCES})
target_include_directories (Awa_object PUBLIC ${API_INCLUDE_DIR})
target_include_directories (Awa_object PRIVATE ${CORE_SRC_DIR})
target_include_directories (Awa_object PRIVATE ${CORE_SRC_DIR}/common)
target_include_directories (Awa_object PRIVATE ${CORE_SRC_DIR}/client)
target_include_directories (Awa_object PRIVATE ${LIB_XML_INCLUDE_DIR})
target_include_directories (Awa_object PRIVATE ${LIB_B64_INCLUDE_DIR})
set_property (TARGET Awa_object PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library (Awa_static STATIC $<TARGET_OBJECTS:Awa_object>)
set_target_properties (Awa_static PROPERTIES OUTPUT_NAME "awa")
target_link_libraries (Awa_static libxml_static libb64_static libhmac_static)

add_library (Awa_shared SHARED $<TARGET_OBJECTS:Awa_object>)
set_target_properties (Awa_shared PROPERTIES OUTPUT_NAME "awa")
target_link_libraries (Awa_shared libxml_static libb64_static libhmac_static)

install (TARGETS Awa_shared
   LIBRARY DESTINATION lib
)

