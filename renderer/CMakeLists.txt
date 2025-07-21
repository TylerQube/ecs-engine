file(GLOB RENDERER_SRC CONFIGURE_DEPENDS *.cpp *.h)

add_library(renderer STATIC ${RENDERER_SRC})

target_include_directories(renderer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
