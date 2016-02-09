cmake_minimum_required(VERSION 2.8)
set(LIBRARY regular_encoding)
project(${LIBRARY})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
set(PROJECT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/")
set(PROJECT_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(sources
  src/main.cc
)

set(headers
  include/main.h
)

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(3rdparty/gtest)
add_subdirectory(test)

add_library(${CMAKE_PROJECT_NAME} STATIC ${sources} ${headers})
target_link_libraries(${CMAKE_PROJECT_NAME})

