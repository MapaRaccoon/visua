cmake_minimum_required(VERSION 3.20)
project(visua VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_BUILD_TYPE Debug)

find_package(glbinding REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(portaudio REQUIRED)
find_package(portaudiocpp REQUIRED)
find_package(ALSA REQUIRED)
find_package(Boost REQUIRED)
find_package(FFTW REQUIRED)

set(PORTAUDIO_COMMON_PATH "${CMAKE_SOURCE_DIR}/external/portaudio/common")
file ( GLOB SOURCES 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${PORTAUDIO_COMMON_PATH}/*.c"
    "${CMAKE_SOURCE_DIR}/external/imgui/*.cpp"
    "${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.cpp"
    "${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_glfw.cpp"
)
add_executable(visua ${SOURCES})

target_include_directories(visua PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/include
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/imgui/backends
    ${PORTAUDIO_INCLUDE_DIRS}
    ${PORTAUDIO_CPP_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${FFTW_INCLUDE_DIRS}
)

target_link_libraries(visua PRIVATE 
    glbinding::glbinding
    glbinding::glbinding-aux
    glfw
    glm::glm
    ${PORTAUDIO_LIBRARIES}
    ${PORTAUDIO_CPP_LIBRARIES}
    ALSA::ALSA
    ${FFTW_LIBRARIES}
)

install( TARGETS visua DESTINATION bin )

add_custom_command(TARGET visua POST_BUILD
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/post-build.sh
)
