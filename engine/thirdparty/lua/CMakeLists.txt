set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE LUA_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

add_library(liblua STATIC ${LUA_SOURCES})
target_include_directories(liblua PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")