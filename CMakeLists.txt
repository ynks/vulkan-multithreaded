cmake_minimum_required(VERSION 3.29)
project(VulkanApp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)


# Try to find an installed GLFW first
find_package(glfw3 QUIET)

include(FetchContent)

# GLWF target selection logic
if(NOT glfw3_FOUND)
    message(STATUS "GLFW not found: fetching GLFW from GitHub via FetchContent")

    # Turn off unneeded GLFW targets
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)


    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.3.8
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(glfw)

    if(TARGET glfw)
        set(GLFW_TARGET glfw)
    elseif(TARGET glfw3)
        set(GLFW_TARGET glfw3)
    else()
        message(FATAL_ERROR "GLFW was fetched but no CMake target 'glfw' or 'glfw3' was created")
    endif()
else()
    message(STATUS "Found GLFW via find_package")
    if(TARGET glfw)
        set(GLFW_TARGET glfw)
    elseif(TARGET glfw3)
        set(GLFW_TARGET glfw3)
    else()
        set(GLFW_TARGET glfw)
    endif()
endif()

# --- GLM: prefer find_package, then local external/glm, then FetchContent ---
set(GLM_TARGET "")
# try config package first
find_package(glm CONFIG QUIET)
if (glm_FOUND)
    message(STATUS "Found GLM via find_package")
    if (TARGET glm)
        set(GLM_TARGET glm)
    elseif (TARGET glm::glm)
        set(GLM_TARGET "glm::glm")
    else()
        set(GLM_TARGET glm)
    endif()
else()
    # prefer local clone at external/glm
    if(EXISTS "${CMAKE_SOURCE_DIR}/external/glm/CMakeLists.txt")
        message(STATUS "GLM not found: using local copy at ${CMAKE_SOURCE_DIR}/external/glm via add_subdirectory()")
        add_subdirectory("${CMAKE_SOURCE_DIR}/external/glm" EXCLUDE_FROM_ALL)
        if (TARGET glm)
            set(GLM_TARGET glm)
        elseif (TARGET glm::glm)
            set(GLM_TARGET "glm::glm")
        else()
            message(FATAL_ERROR "Local external/glm added but no target 'glm' or 'glm::glm' was created")
        endif()
    else()
        message(STATUS "GLM not found: fetching GLM from GitHub via FetchContent")
        FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            # No GIT_TAG so FetchContent will clone the repository default branch (avoids tag checkout issues)
            
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
        )
        FetchContent_MakeAvailable(glm)

        if (TARGET glm)
            set(GLM_TARGET glm)
        elseif (TARGET glm::glm)
            set(GLM_TARGET "glm::glm")
        else()
            message(FATAL_ERROR "GLM was fetched but no CMake target 'glm' or 'glm::glm' was created")
        endif()
    endif()
endif()

find_package(Vulkan REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE MODULES "src/*.ixx")

add_executable(vulkan_app ${SOURCES})
target_sources(vulkan_app PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES ${MODULES})
target_include_directories(vulkan_app PRIVATE ${Vulkan_INCLUDE_DIR})
# link libraries discovered (Vulkan, GLFW, GLM)
if(GLFW_TARGET AND GLM_TARGET)
    target_link_libraries(vulkan_app Vulkan::Vulkan ${GLFW_TARGET} ${GLM_TARGET})
elseif(GLFW_TARGET)
    target_link_libraries(vulkan_app Vulkan::Vulkan ${GLFW_TARGET})
elseif(GLM_TARGET)
    target_link_libraries(vulkan_app Vulkan::Vulkan ${GLM_TARGET})
else()
    target_link_libraries(vulkan_app Vulkan::Vulkan)
endif()

if (CLANG)
    target_link_libraries(vulkan_app  c++ c++abi)
    target_compile_options(vulkan_app PRIVATE -stdlib=libc++)

    add_custom_command(TARGET vulkan_app POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_BINARY_DIR}/compile_commands.json
            ${CMAKE_SOURCE_DIR}/compile_commands.json
    )
endif ()


target_compile_definitions(vulkan_app PRIVATE VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1)
