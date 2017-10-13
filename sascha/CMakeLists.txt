file(GLOB SASCHA_SRC *.cpp)
file(GLOB SASCHA_HEADERS *.hpp)

if(WIN32)
    add_library(sascha STATIC ${SASCHA_SRC})
    target_link_libraries(sascha ${Vulkan_LIBRARY} ${WINLIBS})
 else(WIN32)
    add_library(sascha STATIC ${SASCHA_SRC})
    target_link_libraries(sascha)
endif(WIN32)

