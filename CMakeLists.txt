cmake_minimum_required(VERSION 3.20)
project(visua VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

find_package(glbinding REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(portaudio REQUIRED)
find_package(portaudiocpp REQUIRED)
find_package(ALSA REQUIRED)
find_package(Boost REQUIRED)

set(PORTAUDIO_COMMON_PATH "${CMAKE_SOURCE_DIR}/external/portaudio/common")
file ( GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp" "${PORTAUDIO_COMMON_PATH}/*.c" )
message("woof")
message("${SOURCES}")
message("woof")
add_executable(visua ${SOURCES})

target_include_directories(visua PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${PORTAUDIO_INCLUDE_DIRS}
    ${PORTAUDIO_CPP_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(visua PRIVATE 
    glbinding::glbinding
    glbinding::glbinding-aux
    glfw
    glm::glm
    ${PORTAUDIO_LIBRARIES}
    ${PORTAUDIO_CPP_LIBRARIES}
    ALSA::ALSA
)

install( TARGETS visua DESTINATION bin )
