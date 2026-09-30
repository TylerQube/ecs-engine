cmake_minimum_required(VERSION 3.15)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "vcpkg toolchain file")

project(ECSEngine VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)

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
    glfw
    assimp::assimp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGLRenderer
)
