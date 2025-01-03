add_contract(meme.reg meme.reg ${CMAKE_CURRENT_SOURCE_DIR}/src/meme.reg.cpp)

target_include_directories(meme.reg
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../amax.bootdao/include)

set_target_properties(meme.reg
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/meme.reg.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/meme.reg.contracts.md @ONLY )

target_compile_options( meme.reg PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
