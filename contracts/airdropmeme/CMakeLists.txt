add_contract(airdropmeme airdropmeme ${CMAKE_CURRENT_SOURCE_DIR}/src/airdropmeme.cpp)

target_include_directories(airdropmeme
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../amax.bootdao/include)

set_target_properties(airdropmeme
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/airdropmeme.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/airdropmeme.contracts.md @ONLY )

target_compile_options( airdropmeme PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
