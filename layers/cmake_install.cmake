# Install script for directory: /home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/michaelb/Dev/Vulkan/1.1.126.0/x86_64")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "RelWithDebInfo")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/cast_utils.h"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/hash_util.h"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/hash_vk_types.h"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/vk_format_utils.h"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/vk_format_utils.cpp"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/vk_layer_config.h"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/vk_layer_config.cpp"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/vk_layer_data.h"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/vk_layer_extension_utils.h"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/vk_layer_extension_utils.cpp"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/vk_layer_logging.h"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/vk_layer_utils.h"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/vk_layer_utils.cpp"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/vk_loader_platform.h"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/generated/vk_validation_error_messages.h"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/generated/vk_layer_dispatch_table.h"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/generated/vk_dispatch_table_helper.h"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/generated/vk_safe_struct.h"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/generated/vk_safe_struct.cpp"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/generated/vk_enum_string_helper.h"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/generated/vk_object_types.h"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/generated/vk_extension_helper.h"
    "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/layers/generated/vk_typemap_helper.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/etc/vulkan/explicit_layer.d" TYPE FILE FILES "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/build/layers/staging-json/VkLayer_khronos_validation.json")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/etc/vulkan/explicit_layer.d" TYPE FILE FILES "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/build/layers/staging-json/VkLayer_standard_validation.json")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/home/michaelb/Dev/Vulkan/1.1.126.0/x86_64/lib/libVkLayer_khronos_validation.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/michaelb/Dev/Vulkan/1.1.126.0/x86_64/lib/libVkLayer_khronos_validation.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/michaelb/Dev/Vulkan/1.1.126.0/x86_64/lib/libVkLayer_khronos_validation.so"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/michaelb/Dev/Vulkan/1.1.126.0/x86_64/lib/libVkLayer_khronos_validation.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/michaelb/Dev/Vulkan/1.1.126.0/x86_64/lib" TYPE SHARED_LIBRARY FILES "/home/michaelb/Dev/Vulkan/1.1.126.0/source/Vulkan-ValidationLayers/build/layers/libVkLayer_khronos_validation.so")
  if(EXISTS "$ENV{DESTDIR}/home/michaelb/Dev/Vulkan/1.1.126.0/x86_64/lib/libVkLayer_khronos_validation.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/michaelb/Dev/Vulkan/1.1.126.0/x86_64/lib/libVkLayer_khronos_validation.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/michaelb/Dev/Vulkan/1.1.126.0/x86_64/lib/libVkLayer_khronos_validation.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

