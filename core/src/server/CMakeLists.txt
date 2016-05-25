
set (awa_serverd_SOURCES
  lwm2m_server_core.c
  lwm2m_server_xml_handlers.c
  lwm2m_server_xml_events.c
  lwm2m_server_xml_registered_entity_tree.c
  lwm2m_object_defs.c
  lwm2m_registration.c
  lwm2m_server.c
  ${CORE_SRC_DIR}/common/lwm2m_xml_interface.c
  ${CORE_SRC_DIR}/common/lwm2m_xml_serdes.c
  ${CORE_SRC_DIR}/common/lwm2m_serdes.c
  ${CORE_SRC_DIR}/common/lwm2m_tlv.c
  ${CORE_SRC_DIR}/common/lwm2m_plaintext.c
  ${CORE_SRC_DIR}/common/lwm2m_prettyprint.c
  ${CORE_SRC_DIR}/common/lwm2m_opaque.c
  ${CORE_SRC_DIR}/common/lwm2m_tree_builder.c
  ${CORE_SRC_DIR}/common/lwm2m_ipc.c

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
  ${CORE_SRC_DIR}/../../api/src/write_mode.c
  #############################################################
)

# fetch the INCLUDE_DIRECTORIES properties of non-linked dependencies:
# (it is not possible to link with an OBJECT library, so these are not automatic)
get_property (LIB_XML_INCLUDE_DIR TARGET libxml_static PROPERTY INCLUDE_DIRECTORIES)
get_property (LIB_B64_INCLUDE_DIR TARGET libb64_static PROPERTY INCLUDE_DIRECTORIES)

set (awa_serverd_INCLUDE_DIRS
  ${LIB_XML_INCLUDE_DIR}
  ${LIB_B64_INCLUDE_DIR}

  ${CORE_SRC_DIR}
  ${CORE_SRC_DIR}/common
  ${CORE_SRC_DIR}/server

  ######################## TODO REMOVE ########################
  # TODO: extract components common to both Core and API
  # FIXME: API_INCLUDE_DIR is not in the cmake cache at the time this is read
  #${API_INCLUDE_DIR}
  #${API_SRC_DIR}
  ${CORE_SRC_DIR}/../../api/src
  ${CORE_SRC_DIR}/../../api/include
  #############################################################
)

if (WITH_JSON)
  list (APPEND awa_serverd_SOURCES
    ${CORE_SRC_DIR}/common/lwm2m_json.c
  )
  # LIBJSMN_INCLUDE_DIR is a global, as it's set by an imported target
  list (APPEND awa_serverd_INCLUDE_DIRS
    ${LIBJSMN_INCLUDE_DIR})
endif ()

add_definitions (-DLWM2M_SERVER)

if (ENABLE_GCOV)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif ()

# Virtual library to avoid building .o files twice:
add_library (awa_serverd_object OBJECT ${awa_serverd_SOURCES})
target_include_directories (awa_serverd_object PRIVATE ${awa_serverd_INCLUDE_DIRS})
set_property (TARGET awa_serverd_object PROPERTY POSITION_INDEPENDENT_CODE ON)

# static library liblwm2mserver.a
add_library (awa_serverd_static STATIC $<TARGET_OBJECTS:awa_serverd_object>)
set_target_properties (awa_serverd_static PROPERTIES OUTPUT_NAME "lwm2mserver")

# shared library liblwm2mserver.so
add_library (awa_serverd_shared SHARED $<TARGET_OBJECTS:awa_serverd_object>)
set_target_properties (awa_serverd_shared PROPERTIES OUTPUT_NAME "lwm2mserver")

add_executable (awa_serverd lwm2m_server.c)
target_include_directories (awa_serverd PRIVATE ${awa_serverd_INCLUDE_DIRS})
target_link_libraries (awa_serverd awa_serverd_static awa_common_static)

if (ENABLE_GCOV)
   target_link_libraries (awa_serverd gcov)
endif ()

install (TARGETS awa_serverd
  RUNTIME DESTINATION bin
)
