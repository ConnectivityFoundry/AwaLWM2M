set (awa_clientd_SOURCES
  lwm2m_registration.c
  lwm2m_bootstrap.c
  lwm2m_connectivity_object.c
  lwm2m_security_object.c
  lwm2m_device_object.c
  lwm2m_location_object.c
  lwm2m_firmware_object.c
  lwm2m_acl_object.c
  lwm2m_server_object.c
  lwm2m_client_xml_handlers.c
  lwm2m_client_core.c
  lwm2m_client.c
  ${CORE_SRC_DIR}/common/lwm2m_xml_interface.c
  ${CORE_SRC_DIR}/common/lwm2m_xml_serdes.c
  ${CORE_SRC_DIR}/common/lwm2m_bootstrap_config.c
  ${CORE_SRC_DIR}/common/lwm2m_serdes.c
  ${CORE_SRC_DIR}/common/lwm2m_tlv.c
  ${CORE_SRC_DIR}/common/lwm2m_plaintext.c
  ${CORE_SRC_DIR}/common/lwm2m_prettyprint.c
  ${CORE_SRC_DIR}/common/lwm2m_json.c
  ${CORE_SRC_DIR}/common/lwm2m_opaque.c
  ${CORE_SRC_DIR}/common/lwm2m_tree_builder.c
  ${CORE_SRC_DIR}/common/lwm2m_observers.c
  ${CORE_SRC_DIR}/common/lwm2m_ipc.c
  lwm2m_object_tree.c   
  
  
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

# fetch the INCLUDE_DIRECTORIES properties of non-linked dependencies:
# (it is not possible to link with an OBJECT library, so these are not automatic)
get_property (LIB_XML_INCLUDE_DIR TARGET libxml_static PROPERTY INCLUDE_DIRECTORIES)
get_property (LIB_B64_INCLUDE_DIR TARGET libb64_static PROPERTY INCLUDE_DIRECTORIES)
get_property (LIB_JSMN_INCLUDE_DIR TARGET libjsmn_static PROPERTY INCLUDE_DIRECTORIES)
get_property (LIB_HMAC_INCLUDE_DIR TARGET libhmac_static PROPERTY INCLUDE_DIRECTORIES)

set (awa_clientd_INCLUDE_DIRS
  ${LIB_XML_INCLUDE_DIR}
  ${LIB_B64_INCLUDE_DIR}
  ${LIB_HMAC_INCLUDE_DIR}
  ${LIBJSMN_INCLUDE_DIR}

  ${CORE_SRC_DIR}
  ${CORE_SRC_DIR}/common
  ${CORE_SRC_DIR}/client

  ######################## TODO REMOVE ########################
  # TODO: extract components common to both Core and API
  # FIXME: API_INCLUDE_DIR is not in the cmake cache at the time this is read  
  #${API_INCLUDE_DIR}
  #${API_SRC_DIR}
  ${CORE_SRC_DIR}/../../api/src
  ${CORE_SRC_DIR}/../../api/include
  #############################################################
)

add_definitions (-DLWM2M_CLIENT)

if (ENABLE_GCOV)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif ()


# Virtual library to avoid building .o files twice:
add_library (awa_clientd_object OBJECT ${awa_clientd_SOURCES})
target_include_directories (awa_clientd_object PRIVATE ${awa_clientd_INCLUDE_DIRS})
set_property (TARGET awa_clientd_object PROPERTY POSITION_INDEPENDENT_CODE ON)

# static library liblwm2mclient.a
add_library (awa_clientd_static STATIC $<TARGET_OBJECTS:awa_clientd_object>)
set_target_properties (awa_clientd_static PROPERTIES OUTPUT_NAME "lwm2mclient")

# shared library liblwm2mclient.so
add_library (awa_clientd_shared SHARED $<TARGET_OBJECTS:awa_clientd_object>)
set_target_properties (awa_clientd_shared PROPERTIES OUTPUT_NAME "lwm2mclient")

add_executable (awa_clientd lwm2m_client.c)
target_include_directories (awa_clientd PRIVATE ${awa_clientd_INCLUDE_DIRS})
target_link_libraries (awa_clientd awa_clientd_static lwm2m_common_static libb64_static libhmac_static)

if (ENABLE_GCOV)
  target_link_libraries (awa_clientd gcov)
endif ()

install (TARGETS awa_clientd
    RUNTIME DESTINATION /bin
)
