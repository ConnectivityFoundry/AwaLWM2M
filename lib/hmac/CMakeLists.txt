add_library (libhmac_static hmac.c)
set_property (TARGET libhmac_static PROPERTY POSITION_INDEPENDENT_CODE ON)
set_target_properties (libhmac_static PROPERTIES OUTPUT_NAME "hmac")
target_include_directories (libhmac_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
