set(CMAKE_CXX_STANDARD 20)

# add_executable(${RUNTIME_TARGET} engine.cpp "function/global/global_context.cpp" "function/input/input_system.cpp" "function/physics/physics_manager.cpp" "function/framework/event/event_manager.h")

file(GLOB_RECURSE ENGINE_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/core/**/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/function/framework/**/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/function/global/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/function/input/*.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/function/physics/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/function/render/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/resource/*.cpp
)

source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    PREFIX "Engine Sources"
    FILES ${ENGINE_SOURCES}
)

add_executable(${RUNTIME_TARGET} ${ENGINE_SOURCES} engine.cpp main.cpp)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/function/render/backend)

target_link_libraries(${RUNTIME_TARGET} vulkanBase)
target_link_libraries(${RUNTIME_TARGET} glfw)
target_link_libraries(${RUNTIME_TARGET} liblua)

target_include_directories(${RUNTIME_TARGET} PUBLIC ${THIRD_PARTY_DIR})
target_include_directories(${RUNTIME_TARGET} PUBLIC ${ENGINE_SOURCE_DIR})

# add_custom_command(TARGET ${RUNTIME_TARGET} PRE_BUILD
#    COMMAND ${CMAKE_COMMAND} -E echo "Copying ${RESOURCE_DIR} to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/resources"
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#    ${RESOURCE_DIR}
#    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/resources
#)

