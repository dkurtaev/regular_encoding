set(target run_bijective_checker)

set(sources
  run_bijective_checker.cc
)

add_executable(${target} ${sources})
target_link_libraries(${target} ${LIBRARY})
