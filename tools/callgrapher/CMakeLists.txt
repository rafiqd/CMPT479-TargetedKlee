include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(callgrapher
  main.cpp
)

llvm_map_components_to_libnames(REQ_LLVM_LIBRARIES
  asmparser core linker bitreader irreader ipo analysis support
)

target_link_libraries(callgrapher callgraphs ${REQ_LLVM_LIBRARIES})

# Platform dependencies.
if( WIN32 )
  find_library(SHLWAPI_LIBRARY shlwapi)
  target_link_libraries(callgrapher
    ${SHLWAPI_LIBRARY}
  )
else()
  find_package(Threads REQUIRED)
  find_package(Curses REQUIRED)
  target_link_libraries(callgrapher
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_DL_LIBS}
    ${CURSES_LIBRARIES}
  )
endif()

set_target_properties(callgrapher
                      PROPERTIES
                      LINKER_LANGUAGE CXX
                      PREFIX "")

install(TARGETS callgrapher
  RUNTIME DESTINATION bin
)

