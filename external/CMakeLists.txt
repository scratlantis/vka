add_library(tinyobjloader INTERFACE)
target_include_directories(tinyobjloader INTERFACE TINY_OBJ_LOADER/tinyobjloader)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE STB)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE GLM)

add_library(ktx INTERFACE)
target_include_directories(ktx INTERFACE KTX/include
                                        KTX/other_include/KHR)

add_library(vma INTERFACE)
target_include_directories(vma INTERFACE VULKAN_MEMORY_ALLOCATOR/include)

add_library(csvwriter INTERFACE)
target_include_directories(ktx INTERFACE CSV_WRITER/include)

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE NLOMANN_JSON/include)

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(GLFW/glfw-3.3.4)

# imgui
FILE(GLOB SRCS_IMGUI
"IMGUI/*.cpp"
"IMGUI/*.h"
"IMGUI/backends/imgui_impl_vulkan.h"
"IMGUI/backends/imgui_impl_vulkan.cpp"
"IMGUI/backends/imgui_impl_glfw.h"
"IMGUI/backends/imgui_impl_glfw.cpp")
add_library(imgui STATIC ${SRCS_IMGUI})
target_include_directories(imgui PUBLIC IMGUI IMGUI/backends)
target_include_directories(imgui PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(imgui glfw Vulkan::Vulkan)

# implot
FILE(GLOB SRCS_IMPLOT
"IMPLOT/*.cpp"
"IMPLOT/*.h")
add_library(implot STATIC ${SRCS_IMPLOT})
target_include_directories(implot PUBLIC  IMPLOT )
target_link_libraries(implot imgui)

FILE(GLOB SRCS_IMFILE
"IMFILE/*.cpp"
"IMFILE/*.h")
add_library(imfile STATIC ${SRCS_IMFILE})
target_include_directories(imfile PUBLIC IMFILE)
target_link_libraries(imfile imgui stb)

FILE(GLOB SRCS_TINY_EXR
"TINY_EXR/tinyexr-release/tinyexr.cc"
"TINY_EXR/tinyexr-release/tinyexr.h")
add_library(tinyexr STATIC ${SRCS_TINY_EXR})
target_include_directories(tinyexr PUBLIC TINY_EXR/tinyexr-release)
target_compile_definitions(tinyexr PUBLIC TINYEXR_USE_STB_ZLIB)
target_compile_definitions(tinyexr PUBLIC TINYEXR_USE_MINIZ=0)

