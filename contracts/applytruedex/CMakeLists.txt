add_contract(applytruedex applytruedex ${CMAKE_CURRENT_SOURCE_DIR}/src/applytruedex.cpp)

target_include_directories(applytruedex
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../amax.bootdao/include)

set_target_properties(applytruedex
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/applytruedex.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/applytruedex.contracts.md @ONLY )

target_compile_options( applytruedex PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
