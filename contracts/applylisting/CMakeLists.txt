add_contract(applylisting applylisting ${CMAKE_CURRENT_SOURCE_DIR}/src/applylisting.cpp)

target_include_directories(applylisting
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../amax.bootdao/include)

set_target_properties(applylisting
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/applylisting.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/applylisting.contracts.md @ONLY )

target_compile_options( applylisting PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
