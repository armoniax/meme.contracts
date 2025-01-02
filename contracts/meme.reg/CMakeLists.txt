add_contract(meme.regbp meme.regbp ${CMAKE_CURRENT_SOURCE_DIR}/src/meme.regbp.cpp)

target_include_directories(meme.regbp
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/../amax.bootdao/include)

set_target_properties(meme.regbp
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/ricardian/meme.regbp.contracts.md.in ${CMAKE_CURRENT_BINARY_DIR}/ricardian/meme.regbp.contracts.md @ONLY )

target_compile_options( meme.regbp PUBLIC -R${CMAKE_CURRENT_SOURCE_DIR}/ricardian -R${CMAKE_CURRENT_BINARY_DIR}/ricardian )
