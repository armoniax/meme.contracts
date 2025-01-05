add_contract(applynewmeme applynewmeme ${CMAKE_CURRENT_SOURCE_DIR}/src/applynewmeme.cpp)

target_include_directories(applynewmeme
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../amax.bootdao/include)

set_target_properties(applynewmeme
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/applynewmeme.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/applynewmeme.contracts.md @ONLY )

target_compile_options( applynewmeme PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
