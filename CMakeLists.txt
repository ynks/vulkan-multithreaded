cmake_minimum_required(VERSION 3.29)

project(VulkanApp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add option to enable/disable C++ 20 module
option(ENABLE_CPP20_MODULE "Enable C++ 20 module support for Vulkan" ON)

# Enable C++ module dependency scanning only if C++ 20 module is enabled
if(ENABLE_CPP20_MODULE)
  set(CMAKE_CXX_SCAN_FOR_MODULES ON)
endif()

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

# Find Slang compiler
find_program(SLANG_COMPILER slangc
  HINTS
    ENV SLANG_DIR
    ENV VULKAN_SDK
  PATH_SUFFIXES bin
  DOC "Slang shader compiler"
)

if(NOT SLANG_COMPILER)
  message(FATAL_ERROR "slangc compiler not found. Please install Slang or set SLANG_DIR environment variable.")
endif()

message(STATUS "Found Slang compiler: ${SLANG_COMPILER}")

# set up Vulkan C++ module only if enabled
if(ENABLE_CPP20_MODULE)
  add_library(VulkanCppModule)
  add_library(Vulkan::cppm ALIAS VulkanCppModule)

  target_compile_definitions(VulkanCppModule
          PUBLIC VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
  )
  target_include_directories(VulkanCppModule
          PUBLIC
          "${Vulkan_INCLUDE_DIR}"
  )
  target_link_libraries(VulkanCppModule
          PUBLIC
          Vulkan::Vulkan
  )

  set_target_properties(VulkanCppModule PROPERTIES CXX_STANDARD 23)

  # Add MSVC-specific compiler options for proper C++ module support
  if(MSVC)
    target_compile_options(VulkanCppModule PRIVATE
      /std:c++latest
      /permissive-
      /Zc:__cplusplus
      /EHsc
      /Zc:preprocessor
      /translateInclude
    )
  endif()

  # Add GCC/Clang-specific options to use libc++ for C++ module support
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(VulkanCppModule PRIVATE
      -nostdinc++
      -isystem /usr/include/c++/v1
    )
    target_link_libraries(VulkanCppModule PUBLIC c++ c++abi)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(VulkanCppModule PRIVATE
      -stdlib=libc++
    )
    target_link_libraries(VulkanCppModule PUBLIC c++ c++abi)
  endif()

  target_sources(VulkanCppModule
          PUBLIC
          FILE_SET cxx_modules TYPE CXX_MODULES
          BASE_DIRS
          "${Vulkan_INCLUDE_DIR}"
          FILES
          "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm"
  )

  target_sources(VulkanCppModule
          PRIVATE
          "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm"
  )
else()
  # Create a dummy interface library when C++ 20 module is disabled
  add_library(VulkanCppModule INTERFACE)
  add_library(Vulkan::cppm ALIAS VulkanCppModule)
  target_link_libraries(VulkanCppModule INTERFACE Vulkan::Vulkan)
  target_compile_definitions(VulkanCppModule
          INTERFACE VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
  )
endif()

# Main executable
add_executable(vulkan_app
	src/main.cpp
	src/instance.cpp
	src/device.cpp
	src/window.cpp
	src/swapchain.cpp
)
target_sources(vulkan_app
  PUBLIC
  FILE_SET cxx_modules TYPE CXX_MODULES
  FILES
  src/instance.ixx
  src/device.ixx
	src/window.ixx
	src/swapchain.ixx
)
set_target_properties(vulkan_app PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(vulkan_app Vulkan::cppm glfw)

# Add compile definition if C++ 20 module is enabled
if(ENABLE_CPP20_MODULE)
  target_compile_definitions(vulkan_app PRIVATE USE_CPP20_MODULES=1)
  
  # Use libc++ for GCC/Clang when modules are enabled
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(vulkan_app PRIVATE
      -nostdinc++
      -isystem /usr/include/c++/v1
    )
    target_link_libraries(vulkan_app c++ c++abi)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(vulkan_app PRIVATE -stdlib=libc++)
    target_link_libraries(vulkan_app c++ c++abi)
  endif()
endif()

# Copy compile_commands.json to project root
add_custom_command(TARGET vulkan_app POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}/compile_commands.json
  COMMENT "Copying compile_commands.json to project root"
)
