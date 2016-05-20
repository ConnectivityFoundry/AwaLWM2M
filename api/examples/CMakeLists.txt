set (Examples
  client-get-example
  client-get-contains-path-example
  client-get-array-example
  client-set-example
  client-set-create-example
  client-set-array-example
  client-defineset-example
  client-delete-example
  #client-delete-array-range-example # TODO enable when delete array range is supported.
  client-subscribe-to-change-example
  client-subscribe-to-execute-example
  client-definition-iterator-example
  
  server-list-clients-example
  server-read-example
  server-write-example
  server-write-create-example
  server-write-create-unspecified-instance-example
  server-delete-object-instance-example
  server-observe-example
  server-defineset-example
  server-execute-example
  server-execute-arguments-example
  server-event-example

  # TODO: set-array example uses single values and not an array. Add an example for that too and 
  # rename this set-array to set-array-individual-instances-example
  
  # TODO: get-response-iterator-example
  # TODO: subscribe iterate through changeset example
)

foreach (example ${Examples})
   add_executable (${example} ${example}.c)
  target_include_directories (${example} PRIVATE ${API_INCLUDE_DIR})
  target_link_libraries (${example} Awa_static)
  target_link_libraries (${example} libxml_static)

  if (ENABLE_GCOV)
    target_link_libraries (${example} gcov)
  endif ()
endforeach (example ${Examples})

add_subdirectory (tutorials)
