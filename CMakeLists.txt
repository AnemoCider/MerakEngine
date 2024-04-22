cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

project(game_engine)

set(RUNTIME_TARGET "Runtime")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_SUPPRESS_REGENERATION true)

set(RESOURCE_DIR ${PROJECT_SOURCE_DIR}/resources)

set(SHADER_DIR "${RESOURCE_DIR}/shaders") 

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_MVK")
    set(GLSL_COMPILER "${SHADER_DIR}/glslc.exe")
elseif(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_MACOS_KHR")
    set(GLSL_COMPILER "${SHADER_DIR}/glslc_macos")
endif()

file(GLOB SHADER_SOURCES "${SHADER_DIR}/*.vert" "${SHADER_DIR}/*.frag" )

add_subdirectory(${PROJECT_SOURCE_DIR}/engine)

set(SHADER_SPIRV_FILES "")

# Iterate over all shader sources
foreach(SHADER ${SHADER_SOURCES})
    # Get the filename without extension
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    # Define the output file
    set(SHADER_SPIRV "${SHADER_DIR}/${SHADER_NAME}.spv")
    list(APPEND SHADER_SPIRV_FILES ${SHADER_SPIRV})
    # Add a custom command that compiles the shader
    add_custom_command(
        OUTPUT ${SHADER_SPIRV}
        COMMAND ${GLSL_COMPILER} -o ${SHADER_SPIRV} ${SHADER}
        DEPENDS ${SHADER}
        COMMENT "Compiling ${SHADER_NAME} to SPIR-V"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endforeach()

add_custom_target(CompileShaders
    DEPENDS ${SHADER_SPIRV_FILES}
    COMMENT "Compiling shaders..."
)

# Add the shaders directory as a dependency to ensure the shaders are compiled
set_source_files_properties(${SHADER_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)