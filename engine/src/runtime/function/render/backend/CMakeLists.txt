file(GLOB_RECURSE FUNC_SRC "*.cpp")
file(GLOB_RECURSE HEADERS "*.h")


find_package(Vulkan REQUIRED)

add_library(vulkanBase STATIC VulkanBase.cpp ${FUNC_SRC} ${HEADERS})
source_group("Vulkan headers" FILES ${HEADERS})

target_include_directories(vulkanBase PUBLIC ${THIRD_PARTY_DIR})
target_include_directories(vulkanBase PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(vulkanBase PUBLIC ${THIRD_PARTY_DIR}/vulkan/include)
target_link_libraries(vulkanBase ${Vulkan_LIBRARY} glfw)