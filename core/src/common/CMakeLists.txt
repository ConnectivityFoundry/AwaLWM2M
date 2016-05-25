set (awa_common_SOURCES
  lwm2m_list.c
  coap_abstraction_libcoap.c
  lwm2m_debug.c
  lwm2m_util.c
  lwm2m_object_store.c
  lwm2m_attributes.c
  lwm2m_definition.c
  xml.c
  lwm2m_tree_node.c
  lwm2m_endpoints.c
  lwm2m_result.c
  lwm2m_types.c
  ipc_session.c
  lwm2m_events.c
)

# currently the API and core are a little tangled so we need to define the include
# path here. the API tests depend on core, which depends on the API.
set (API_INCLUDE_DIR ../../../api/include CACHE INTERNAL "API_INCLUDE_DIR")

set (awa_common_INCLUDE_DIRS
  ${LIBCOAP_INCLUDE_DIR}
  ${API_INCLUDE_DIR}
)

set (awa_common_LIBS
  libxml_static
  libcoap_static
  libb64_static
)

if (WITH_JSON)
  # LIBJSMN_INCLUDE_DIR is a global, as it's set by an imported target
  list (APPEND awa_common_INCLUDE_DIRS
    ${LIBJSMN_INCLUDE_DIR}
  )
  list (APPEND awa_common_LIBS
    libjsmn_static
  )
endif ()

if (ENABLE_GCOV)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif ()

# Virtual library to avoid building .o files twice:
#add_library(lwm2m_common_object OBJECT ${awa_common_SOURCES})
#target_include_directories (lwm2m_common_object PRIVATE ${awa_common_INCLUDE_DIRS})
#set_property(TARGET lwm2m_common_object PROPERTY POSITION_INDEPENDENT_CODE ON)

#add_library (awa_common_static STATIC $<TARGET_OBJECTS:lwm2m_common_object>)
add_library (awa_common_static STATIC ${awa_common_SOURCES})
set_target_properties (awa_common_static PROPERTIES OUTPUT_NAME "lwm2mcommon")
set_target_properties (awa_common_static PROPERTIES POSITION_INDEPENDENT_CODE ON)
# TODO: remove this line after sorting out the libcoap_static include path
target_include_directories(awa_common_static PUBLIC ${awa_common_INCLUDE_DIRS})

# Combine libxml and libcoap into awa_common_static:
target_link_libraries (awa_common_static ${awa_common_LIBS})

if (ENABLE_GCOV)
  target_link_libraries (awa_common_static gcov)
endif ()

# Currently disabled as libxml isn't PIC
#add_library (lwm2m_common_shared SHARED $<TARGET_OBJECTS:lwm2m_common_object>)
#set_target_properties (lwm2m_common_shared PROPERTIES OUTPUT_NAME "lwm2mcommon")
#target_link_libraries (lwm2m_common_shared libxml_static ${LIBCOAP_LIBRARY} ${LIBJSMN_LIBRARY})

