#CMAKE
cmake_minimum_required(VERSION 3.14)
include(CMakePrintHelpers)

############################################################

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

############################################################

# project name
project(vka VERSION 1.0
                       DESCRIPTION "vka"
                       LANGUAGES CXX)

############################################################

# general configuration
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${vulkan_framework_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${vulkan_framework_VERSION_MINOR}")

if(UNIX)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g")
else()
endif()

############################################################

# create source groups
FILE(GLOB_RECURSE ALL_SOURCES
"src/*.cpp"
"src/*.h"
"src/*.vert"
"src/*.frag"
"src/*.glsl"
"src/*.comp"
"src/*.geom"
"src/*.rgen"
"src/*.rchit"
"src/*.rahit"
"src/*.rmiss"
)

foreach(FILE ${ALL_SOURCES}) 
    get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")
    string(REPLACE "/" "\\" GROUP "${GROUP}")
    source_group("${GROUP}" FILES "${FILE}")
endforeach()

############################################################

# dependencies:
# vulkan
find_package(Vulkan REQUIRED)
# external folder
add_subdirectory(external)
# imgui
FILE(GLOB SRCS_IMGUI "external/IMGUI/*.cpp" "external/IMGUI/*.h"
"external/IMGUI/backends/imgui_impl_vulkan.h"
"external/IMGUI/backends/imgui_impl_vulkan.cpp"
"external/IMGUI/backends/imgui_impl_glfw.h"
"external/IMGUI/backends/imgui_impl_glfw.cpp")
add_library(imgui STATIC ${SRCS_IMGUI})
target_include_directories(imgui PUBLIC   ./external/IMGUI/ ./external/IMGUI/backends/)
target_include_directories(imgui PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(imgui glfw Vulkan::Vulkan)
# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/GLFW/glfw-3.3.4)
# threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
# boost
#FIND_PACKAGE(Boost)
#IF (Boost_FOUND)
#    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
#    ADD_DEFINITIONS( "-DHAS_BOOST" )
#ENDIF()
#find_package(Boost 1.56 REQUIRED)
#find_package(Boost COMPONENTS system filesystem REQUIRED)

############################################################

# macros
MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
  IF(IS_DIRECTORY ${child})
  string(REPLACE "${curdir}/" "" child "${child}")
  LIST(APPEND dirlist "${child}")
  ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

############################################################

# create and link targets
SET(library_targets "")
SET(executable_targets "")
SET(all_targets "")
SUBDIRLIST(VKA_VERSIONS "${CMAKE_CURRENT_SOURCE_DIR}/src")
FOREACH(vka_version ${VKA_VERSIONS})
  FILE(GLOB_RECURSE current_lib_sources 
  "src/${vka_version}/framework/*.cpp"
  "src/${vka_version}/framework/*.h"
  "src/${vka_version}/framework/*.vert"
  "src/${vka_version}/framework/*.frag"
  "src/${vka_version}/framework/*.glsl"
  "src/${vka_version}/framework/*.comp"
  "src/${vka_version}/framework/*.geom"
  "src/${vka_version}/framework/*.rgen"
  "src/${vka_version}/framework/*.rchit"
  "src/${vka_version}/framework/*.rahit"
  "src/${vka_version}/framework/*.rmiss")

  string(REPLACE "." "_" current_lib "vka_${vka_version}")
  cmake_print_variables(current_lib)
  #cmake_print_variables(${current_sources})
  add_library(${current_lib} STATIC ${current_lib_sources})
  LIST(APPEND library_targets ${current_lib})
  LIST(APPEND all_targets ${current_lib})

  target_compile_definitions(${current_lib} PUBLIC FRAMEWORK_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}/src/${vka_version}/framework")
  target_compile_definitions(${current_lib} PUBLIC RESOURCE_BASE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources")

  # link
  target_include_directories(${current_lib} PUBLIC   ./src/${vka_version}/framework/ )
  target_include_directories(${current_lib} PRIVATE ${Vulkan_INCLUDE_DIRS})
  target_link_libraries(
      ${current_lib} Vulkan::Vulkan glfw imgui stb glm tinyobjloader vma nlohmann_json ktx
      csvwriter ${CMAKE_DL_LIBS} Threads::Threads)
  
  # folder
  set_target_properties (${current_lib} PROPERTIES FOLDER ${vka_version}/framework )

  SUBDIRLIST(APPLICATIONS "${CMAKE_CURRENT_SOURCE_DIR}/src/${vka_version}/applications")
  FOREACH(application ${APPLICATIONS})
    FILE(GLOB_RECURSE current_app_sources 
    "src/${vka_version}/applications/${application}/*.cpp"
    "src/${vka_version}/applications/${application}/*.h"
    "src/${vka_version}/applications/${application}/*.vert"
    "src/${vka_version}/applications/${application}/*.frag"
    "src/${vka_version}/applications/${application}/*.glsl"
    "src/${vka_version}/applications/${application}/*.comp"
    "src/${vka_version}/applications/${application}/*.geom"
    "src/${vka_version}/applications/${application}/*.rgen"
    "src/${vka_version}/applications/${application}/*.rchit"
    "src/${vka_version}/applications/${application}/*.rahit"
    "src/${vka_version}/applications/${application}/*.rmiss")

    string(REPLACE "." "_" current_app "${application}_${vka_version}")
    add_executable(${current_app} ${current_app_sources})
    LIST(APPEND executable_targets ${current_app})
    LIST(APPEND all_targets ${current_app})

    target_compile_definitions(${current_app} PUBLIC SHADER_OUTPUT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/build/shader/${application}")
    target_compile_definitions(${current_app} PUBLIC APP_NAME="${application}")
    target_compile_definitions(${current_app} PUBLIC APP_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}/src/${vka_version}/applications/${application}")
    target_compile_definitions(${current_app} PUBLIC RESOURCE_BASE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources")

    # link
    target_include_directories(${current_app} PUBLIC   ./src/${vka_version}/applications/${application}/ )
    target_include_directories(${current_app} PUBLIC   ./src/${vka_version}/framework/ )
    #target_include_directories(${current_app} PUBLIC   ./src/${vka_version}/applications_common/ )
    target_include_directories(${current_app} PRIVATE ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(${current_app} Vulkan::Vulkan glfw imgui stb glm tinyobjloader vma nlohmann_json ktx
      csvwriter ${CMAKE_DL_LIBS} Threads::Threads)
    target_link_libraries(${current_app} ${current_lib})

    # folder
    set_target_properties (${current_app} PROPERTIES FOLDER ${vka_version}/applications )

  ENDFOREACH()
ENDFOREACH()

############################################################

# target properties
set_target_properties( ${library_targets} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set_target_properties( ${executable_targets} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
#set_target_properties (${library_targets} PROPERTIES FOLDER framework )
#set_target_properties (${executable_targets} PROPERTIES FOLDER applications)

############################################################

# install
if(UNIX)
install(TARGETS ${all_targets} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
else()
install(TARGETS ${all_targets} DESTINATION bin)
endif()


