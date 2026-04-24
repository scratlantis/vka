set(TARGET_NAME boxer)

if(WIN32)
  add_library(${TARGET_NAME} STATIC IMPORTED GLOBAL)
  target_include_directories(${TARGET_NAME} INTERFACE "include")
  set_target_properties(${TARGET_NAME} PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/lib/win/boxer.lib")
else()
  add_library(${TARGET_NAME} STATIC "src/boxer_linux.cpp")
  target_include_directories(${TARGET_NAME} PUBLIC "include")

  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

  target_include_directories(${TARGET_NAME} PRIVATE ${GTK3_INCLUDE_DIRS})
  target_link_libraries(${TARGET_NAME} PRIVATE ${GTK3_LIBRARIES})
  target_compile_options(${TARGET_NAME} PRIVATE ${GTK3_CFLAGS_OTHER})
endif()
