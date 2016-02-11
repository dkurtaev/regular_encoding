set(target test_bijective_checker)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty")

set(sources
  test_bijective_checker.cc
  test_main.cc
)

add_executable(${target} ${sources})
target_link_libraries(${target} gtest ${CMAKE_THREAD_LIBS_INIT} pthread ${LIBRARY})
