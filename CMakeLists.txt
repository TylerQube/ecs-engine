cmake_minimum_required(VERSION 3.15)

project(ECSEngine VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/dependencies/include
)

# === Set up OpenGL Renderer === #
add_library(OpenGLRenderer
    ${PROJECT_SOURCE_DIR}/Renderer/OpenGLRenderer/OpenGLRenderer.cpp
    ${PROJECT_SOURCE_DIR}/dependencies/src/glad.c
)

target_include_directories(OpenGLRenderer PRIVATE
    ${PROJECT_SOURCE_DIR}/dependencies/include
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(OpenGLRenderer PRIVATE
    ${PROJECT_SOURCE_DIR}/dependencies/lib/libglfw.3.3.dylib
)
# ============================= #

# Find macOS frameworks
find_library(OPENGL_FRAMEWORK OpenGL)
find_library(COCOA_FRAMEWORK Cocoa)
find_library(IOKIT_FRAMEWORK IOKit)
find_library(COREVIDEO_FRAMEWORK CoreVideo)
find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)

target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGLRenderer
    ${OPENGL_FRAMEWORK}
    ${COCOA_FRAMEWORK}
    ${IOKIT_FRAMEWORK}
    ${COREVIDEO_FRAMEWORK}
    ${COREFOUNDATION_FRAMEWORK}
)
