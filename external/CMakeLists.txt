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