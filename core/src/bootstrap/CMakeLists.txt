set (awa_bootstrap_SOURCES
  lwm2m_bootstrap.c
  lwm2m_bootstrap_core.c
  ${CORE_SRC_DIR}/server/lwm2m_object_defs.c
  ${CORE_SRC_DIR}/common/lwm2m_bootstrap_config.c
  ${CORE_SRC_DIR}/common/lwm2m_serdes.c
  ${CORE_SRC_DIR}/common/lwm2m_tlv.c
  ${CORE_SRC_DIR}/common/lwm2m_plaintext.c
  ${CORE_SRC_DIR}/common/lwm2m_prettyprint.c
  ${CORE_SRC_DIR}/common/lwm2m_opaque.c
  ${CORE_SRC_DIR}/common/lwm2m_tree_builder.c
)

# fetch the INCLUDE_DIRECTORIES properties of non-linked dependencies:
# (it is not possible to link with an OBJECT library, so these are not automatic)
get_property (LIB_XML_INCLUDE_DIR TARGET libxml_static PROPERTY INCLUDE_DIRECTORIES)
get_property (LIB_B64_INCLUDE_DIR TARGET libb64_static PROPERTY INCLUDE_DIRECTORIES)

set (awa_bootstrap_INCLUDE_DIRS
  ${LIB_XML_INCLUDE_DIR}
  ${LIB_B64_INCLUDE_DIR}
  
  ${CORE_SRC_DIR}
  ${CORE_SRC_DIR}/common
  ${CORE_SRC_DIR}/bootstrap
  
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
  list (APPEND awa_bootstrap_SOURCES
    ${CORE_SRC_DIR}/common/lwm2m_json.c
  )
  # LIBJSMN_INCLUDE_DIR is a global, as it's set by an imported target
  list (APPEND awa_bootstrap_INCLUDE_DIRS
    ${LIBJSMN_INCLUDE_DIR}
  )
endif ()

add_definitions (-DLWM2M_BOOTSTRAP)

if (ENABLE_GCOV)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0 --coverage")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --coverage")
endif ()

# Virtual library to avoid building .o files twice:
add_library (awa_bootstrap_server_object OBJECT ${awa_bootstrap_SOURCES})
target_include_directories (awa_bootstrap_server_object PRIVATE ${awa_bootstrap_INCLUDE_DIRS})
set_property (TARGET awa_bootstrap_server_object PROPERTY POSITION_INDEPENDENT_CODE ON)

# static library liblwm2mserver.a
add_library (awa_bootstrap_server_static STATIC $<TARGET_OBJECTS:awa_bootstrap_server_object>)
set_target_properties (awa_bootstrap_server_static PROPERTIES OUTPUT_NAME "lwm2mbootstrapserver")

# shared library liblwm2mserver.so
add_library (awa_bootstrap_server_shared SHARED $<TARGET_OBJECTS:awa_bootstrap_server_object>)
set_target_properties (awa_bootstrap_server_shared PROPERTIES OUTPUT_NAME "lwm2mbootstrapserver")
