include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty")

set(main test_main.cc)

set(tests
  test_code_generator.cc
  test_bijective_checker.cc
)

foreach(test ${tests})
  get_filename_component(target ${test} NAME_WE)
  add_executable(${target} ${test} ${main})
  target_link_libraries(${target} gtest ${CMAKE_THREAD_LIBS_INIT} pthread ${LIBRARY})
endforeach(test)

