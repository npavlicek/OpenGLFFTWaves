cmake_minimum_required(VERSION 3.21)

project(OpenGLWaves)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(IMGUI_PATH "./imgui" CACHE STRING "Path to imgui source")
set(GLAD_PATH "./glad" CACHE STRING "Path to glad source")

# find glfw3
find_package(glfw3 REQUIRED)

# compile imgui library
file(GLOB IMGUI_SOURCES "${IMGUI_PATH}/*.cpp" "${IMGUI_PATH}/backends/imgui_impl_glfw.cpp" "${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp")
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${IMGUI_PATH})
target_include_directories(imgui PUBLIC ${IMGUI_PATH}/backends)
target_link_libraries(imgui PUBLIC glfw)

# compile glad library
add_library(glad STATIC "${GLAD_PATH}/gl.c")
target_include_directories(glad PUBLIC "./include")

# finally compile the executable
add_executable(OpenGLWaves main.cpp plane.cpp Shader.cpp Spectrum.cpp GLWaves.cpp Skybox.cpp WaveSettings.cpp)

target_include_directories(OpenGLWaves PUBLIC "./include")
target_include_directories(OpenGLWaves PUBLIC ${IMGUI_PATH})
target_include_directories(OpenGLWaves PUBLIC ${IMGUI_PATH}/backends)

target_link_libraries(OpenGLWaves PUBLIC glfw)
target_link_libraries(OpenGLWaves PUBLIC imgui)
target_link_libraries(OpenGLWaves PUBLIC glad)
