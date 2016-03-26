set (awa_erbium_SOURCES
  er-coap.c
  er-coap-engine.c
  er-coap-transactions.c
#  er-coap-observe.c
  er-coap-separate.c
#  er-coap-res-well-known-core.c
  er-coap-block1.c
#  er-coap-observe-client.c
)

# currently the API and core are a little tangled so we need to define the include
# path here. the API tests depend on core, which depends on the API.
set (API_INCLUDE_DIR ../../../api/include CACHE INTERNAL "API_INCLUDE_DIR")

set (awa_erbium_INCLUDE_DIRS
  ${LIBCOAP_INCLUDE_DIR}
  ${API_INCLUDE_DIR}
)

set (awa_erbium_LIBS
  libxml_static
  libb64_static
)

if (WITH_JSON)
  # LIBJSMN_INCLUDE_DIR is a global, as it's set by an imported target
  list (APPEND awa_erbium_INCLUDE_DIRS
    ${LIBJSMN_INCLUDE_DIR}
  )
  list (APPEND awa_erbium_LIBS
    libjsmn_static
  )
endif ()

if (ENABLE_GCOV)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 --coverage")
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif ()

# Virtual library to avoid building .o files twice:
#add_library(lwm2m_common_object OBJECT ${awa_erbium_SOURCES})
#target_include_directories (lwm2m_common_object PRIVATE ${awa_erbium_INCLUDE_DIRS})
#set_property(TARGET lwm2m_common_object PROPERTY POSITION_INDEPENDENT_CODE ON)

#add_library (awa_erbium_static STATIC $<TARGET_OBJECTS:lwm2m_common_object>)
add_library (awa_erbium_static STATIC ${awa_erbium_SOURCES})
set_target_properties (awa_erbium_static PROPERTIES OUTPUT_NAME "erbiumstatic")
set_target_properties (awa_erbium_static PROPERTIES POSITION_INDEPENDENT_CODE ON)
# TODO: remove this line after sorting out the libcoap_static include path
target_include_directories(awa_erbium_static PUBLIC ${awa_erbium_INCLUDE_DIRS})

# Combine libxml and libcoap into awa_erbium_static:
target_link_libraries (awa_erbium_static ${awa_erbium_LIBS})

if (ENABLE_GCOV)
  target_link_libraries (awa_erbium_static gcov)
endif ()

# Currently disabled as libxml isn't PIC
#add_library (lwm2m_common_shared SHARED $<TARGET_OBJECTS:lwm2m_common_object>)
#set_target_properties (lwm2m_common_shared PROPERTIES OUTPUT_NAME "lwm2mcommon")
#target_link_libraries (lwm2m_common_shared libxml_static ${LIBCOAP_LIBRARY} ${LIBJSMN_LIBRARY})

add_definitions (-DREST_MAX_CHUNK_SIZE=512)

