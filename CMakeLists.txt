cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
cmake_policy(VERSION 2.8)
cmake_policy(SET CMP0015 NEW)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(NAME vulkanApp)
set(APP_NAME OnexApp)

project(${NAME})

include_directories(external)
include_directories(external/glm)
include_directories(external/imgui)
include_directories(sascha)

include_directories(../OnexKernel/include)
include_directories(../OnexLang/include)
include_directories(../OnexOS/include)

link_directories(../OnexKernel)
link_directories(../OnexLang)
link_directories(../OnexOS)

OPTION(USE_D2D_WSI "Build the project using Direct to Display swapchain" OFF)
OPTION(USE_WAYLAND_WSI "Build the project using Wayland swapchain" OFF)

# Use FindVulkan module added with CMAKE 3.7
if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
  message(STATUS "Using module to find Vulkan")
  find_package(Vulkan)
endif()

IF(WIN32)
  IF (NOT Vulkan_FOUND)
    find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS ${CMAKE_SOURCE_DIR}/libs/vulkan)
    IF (Vulkan_LIBRARY)
      set(Vulkan_FOUND ON)
      MESSAGE("Using bundled Vulkan library version")
    ENDIF()
  ENDIF()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
ELSE(WIN32)
  IF (NOT Vulkan_FOUND)
    find_library(Vulkan_LIBRARY NAMES vulkan HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
    IF (Vulkan_LIBRARY)
      set(Vulkan_FOUND ON)
      MESSAGE("Using bundled Vulkan library version")
    ENDIF()
  ENDIF()
  find_package(Threads REQUIRED)
IF(USE_D2D_WSI)
  MESSAGE("Using direct to display extension...")
  add_definitions(-D_DIRECT2DISPLAY)
ELSEIF(USE_WAYLAND_WSI)
  find_package(Wayland REQUIRED)
  if (NOT WAYLAND_FOUND)
    message(FATAL_ERROR "Wayland development package not found")
  endif ()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WAYLAND_KHR")
  include_directories(${WAYLAND_INCLUDE_DIR})
ELSE(USE_D2D_WSI)
  find_package(XCB REQUIRED COMPONENTS xcb xkbcommon)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_XCB_KHR")
ENDIF(USE_D2D_WSI)
  # Todo : android?
ENDIF(WIN32)

IF (NOT Vulkan_FOUND)
  message(FATAL_ERROR "Could not find Vulkan library!")
ELSE()
  message(STATUS ${Vulkan_LIBRARY})
ENDIF()

# Set preprocessor defines
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES -Wno-write-strings")

# Clang specific stuff
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch-enum")
endif()

# Debug flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wextra -Wundef")
endif(CMAKE_COMPILER_IS_GNUCXX)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-std=c++11)

function(buildExample SRC)
  file(GLOB SOURCE *.cpp ${SRC}/*.cpp)
  SET(MAIN_CPP ${SRC}/${APP_NAME}.cpp)
  file(GLOB ADD_SOURCE external/imgui/*.cpp)
  SET(SOURCE ${SOURCE} ${ADD_SOURCE})
  if(WIN32)
    add_executable(${APP_NAME} WIN32 ${MAIN_CPP} ${SOURCE})
    target_link_libraries(${APP_NAME} sascha ${Vulkan_LIBRARY} ${WINLIBS})
  else(WIN32)
    add_executable(${APP_NAME} ${MAIN_CPP} ${SOURCE})
    target_link_libraries(${APP_NAME} sascha onex-kernel-xcb onex-lang-x86)
  endif(WIN32)
endfunction(buildExample)

function(buildExamples)
  foreach(SRC ${SOURCES})
    buildExample(${SRC})
  endforeach(SRC)
endfunction(buildExamples)

# Compiler specific stuff
IF(MSVC)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
ENDIF(MSVC)

IF(WIN32)
  # Nothing here (yet)
ELSE(WIN32)
  link_libraries(${XCB_LIBRARIES} ${Vulkan_LIBRARY} ${WAYLAND_CLIENT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
ENDIF(WIN32)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")

add_subdirectory(sascha)

set(SOURCES src)

buildExamples()
