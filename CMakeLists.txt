cmake_minimum_required(VERSION 2.8)
set(LIBRARY regular_encoding)
project(${LIBRARY})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
set(PROJECT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/")
set(PROJECT_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(sources
  src/alphabetic_encoder.cc
  src/bijective_checker.cc
  src/code_generator.cc
  src/code_tree.cc
  src/code_tree_node.cc
  src/simple_suffix_tree.cc
  src/state_machine.cc
  src/structures.cc
  src/unbijective_code_generator.cc
)

set(headers
  include/alphabetic_encoder.h
  include/bijective_checker.h
  include/code_generator.h
  include/code_tree.h
  include/code_tree_node.h
  include/simple_suffix_tree.h
  include/state_machine.h
  include/structures.h
  include/unbijective_code_generator.h
)

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(3rdparty/gtest)
add_subdirectory(test)
add_subdirectory(tools)

add_library(${CMAKE_PROJECT_NAME} STATIC ${sources} ${headers})
target_link_libraries(${CMAKE_PROJECT_NAME})

