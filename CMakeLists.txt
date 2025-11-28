cmake_minimum_required(VERSION 3.29)
project(VulkanApp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE MODULES "src/*.ixx")

add_executable(vulkan_app ${SOURCES})
target_sources(vulkan_app PUBLIC FILE_SET cxx_modules TYPE CXX_MODULES FILES ${MODULES})
target_include_directories(vulkan_app PRIVATE ${Vulkan_INCLUDE_DIR})
target_link_libraries(vulkan_app Vulkan::Vulkan glfw c++ c++abi)
target_compile_options(vulkan_app PRIVATE -stdlib=libc++)
target_compile_definitions(vulkan_app PRIVATE VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1)

add_custom_command(TARGET vulkan_app POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}/compile_commands.json
)

