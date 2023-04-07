cmake_minimum_required(VERSION 3.20)
project(visua VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

find_package(glbinding REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

file ( GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp" )
add_executable(visua ${SOURCES})

target_include_directories(visua PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(visua PRIVATE 
    glbinding::glbinding
    glbinding::glbinding-aux
    glfw
    glm::glm
)
