set (Tutorials
  client-tutorial1
  client-tutorial2
  server-tutorial
  static-client-tutorial1
  static-client-tutorial2
  static-client-tutorial3
)

foreach (tutorial ${Tutorials})

  add_executable (${tutorial} ${tutorial}.c)
  target_include_directories (${tutorial} PRIVATE ${API_INCLUDE_DIR})
  target_link_libraries (${tutorial} Awa_static)
  target_link_libraries (${tutorial} awa_static)
  target_link_libraries (${tutorial} libxml_static)

  if (ENABLE_GCOV)
    target_link_libraries (${tutorial} gcov)
  endif ()
endforeach (tutorial ${Tutorials})
