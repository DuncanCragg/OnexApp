
project(OnexApp)

cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

set(TOP_DIR ../..)
set(ONEX_KERNEL ../../../OnexKernel)
set(ONEX_LANG ../../../OnexLang)
set(EXTERNAL_DIR ../../external)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Werror -Wno-incompatible-pointer-types-discards-qualifiers -DVK_USE_PLATFORM_ANDROID_KHR -DVK_NO_PROTOTYPES")

file(GLOB SOURCE_FILES "${ONEX_KERNEL}/src/onl/unix/channel-serial.c" "${ONEX_KERNEL}/src/onl/unix/mem.c" "${ONEX_KERNEL}/src/onl/unix/persistence.c" "${ONEX_KERNEL}/src/onl/unix/properties.c" "${ONEX_KERNEL}/src/onl/unix/random.c" "${ONEX_KERNEL}/src/onl/unix/serial.c" "${ONEX_KERNEL}/src/onl/unix/time.c" "${ONEX_KERNEL}/src/lib/lib.c" "${ONEX_KERNEL}/src/lib/list.c" "${ONEX_KERNEL}/src/lib/value.c" "${ONEX_KERNEL}/src/onn/*.c" "${ONEX_KERNEL}/src/onp/*.c" "${ONEX_LANG}/src/*.c" "${EXTERNAL_DIR}/imgui/*.cpp" "${TOP_DIR}/sascha/*.cpp" "${TOP_DIR}/src/*.cpp")

add_library(native-lib SHARED ${SOURCE_FILES})

add_library(native-app-glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

#add_subdirectory(../base ${CMAKE_SOURCE_DIR}/../base)

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

include_directories(${EXTERNAL_DIR}/imgui)
include_directories(${EXTERNAL_DIR}/glm)
include_directories(${ONEX_KERNEL}/include)
include_directories(${ONEX_KERNEL}/src)
include_directories(${ONEX_KERNEL}/src/onn)
include_directories(${ONEX_KERNEL}/src/onp)
include_directories(${ONEX_LANG}/include)
include_directories(${ANDROID_NDK}/sources/android/native_app_glue)

target_link_libraries(
    native-lib
    native-app-glue
    android
    log
    z
)
