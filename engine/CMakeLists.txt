set(ENGINE_SOURCE_DIR ${PROJECT_SOURCE_DIR}/engine/src)

set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)

add_subdirectory(${THIRD_PARTY_DIR}/glfw)
add_subdirectory(${THIRD_PARTY_DIR}/lua)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/runtime)

