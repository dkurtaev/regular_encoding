set(tools
  check_bijectivity.cc
)

foreach(tool ${tools})
  get_filename_component(target ${tool} NAME_WE)
  add_executable(${target} ${tool})
  target_link_libraries(${target} ${LIBRARY})
endforeach(tool)

