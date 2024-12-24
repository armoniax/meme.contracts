add_contract(meme.token meme.token ${CMAKE_CURRENT_SOURCE_DIR}/src/meme.token.cpp)

target_include_directories(meme.token
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(meme.token
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/meme.token.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/meme.token.contracts.md @ONLY )

target_compile_options( meme.token PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
