add_library(tinyobjloader INTERFACE)
target_include_directories(tinyobjloader INTERFACE TINY_OBJ_LOADER/tinyobjloader)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE STB)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE GLM)

add_library(vma INTERFACE)
target_include_directories(vma INTERFACE VULKAN_MEMORY_ALLOCATOR/VulkanMemoryAllocator/include)

add_library(csvwriter INTERFACE)
target_include_directories(csvwriter INTERFACE CSV_WRITER/include)
