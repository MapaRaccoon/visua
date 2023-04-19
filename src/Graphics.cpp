#define GLFW_INCLUDE_NONE

#include "Graphics.hpp"
#include "Shader.hpp"
#include <GLFW/glfw3.h>
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <iostream>
#include <string>

namespace gfx
{

using namespace gl;

Window::Window( std::string name, gl::GLsizei width, gl::GLsizei height )
    : name_( name )
    , width_( width )
    , height_( height )
    , isMovedFrom_( false )
{
}

Window::Window( Window &&other ) : isMovedFrom_( false )
{
    if ( other.isMovedFrom_ )
        return;

    std::swap( other.name_, this->name_ );
    std::swap( other.width_, this->width_ );
    std::swap( other.height_, this->height_ );
    std::swap( other.glfwWindow_, this->glfwWindow_ );

    other.isMovedFrom_ = true;
}

Window &Window::operator=( Window &&other )
{
    if ( other.isMovedFrom_ )
        return *this;

    std::swap( other.name_, this->name_ );
    std::swap( other.width_, this->width_ );
    std::swap( other.height_, this->height_ );
    std::swap( other.glfwWindow_, this->glfwWindow_ );

    other.isMovedFrom_ = true;

    return *this;
}

std::optional<Window> Window::create( std::string name, gl::GLsizei width, gl::GLsizei height )
{
    if (!glfwInit()) return {};
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_FLOATING, GLFW_TRUE );

    glfwSetErrorCallback( glfwError );
    auto glfwWindow = glfwCreateWindow( width, height, name.c_str(), nullptr, nullptr );
    if ( !glfwWindow ) {
        glfwTerminate();
        return {};
    }
    Window window( name, width, height );
    window.glfwWindow_ = glfwWindow;

    glfwMakeContextCurrent( glfwWindow );
    glbinding::initialize( glfwGetProcAddress );

    glViewport( 0, 0, width, height );
    glfwSetFramebufferSizeCallback( glfwWindow, framebufferSizeCallback );

    return window;
}

Window::~Window()
{
    if ( !isMovedFrom_ )
        glfwTerminate();
}

bool Window::shouldClose()
{
    return glfwWindowShouldClose( glfwWindow_ );
}

void Window::swapBuffers()
{
    glfwSwapBuffers( glfwWindow_ );
}

int Window::getKey( int key )
{
    return glfwGetKey( glfwWindow_, key );
}

void framebufferSizeCallback( GLFWwindow *window, int width, int height )
{
    gl::glViewport( 0, 0, width, height );
}

void glfwError( int error, char const *msg )
{
    std::cerr << "GLFW error (" << error << "): " << msg << std::endl;
}

void Window::setShouldClose( bool shouldClose )
{
    glfwSetWindowShouldClose( glfwWindow_, shouldClose );
}

bool Window::isKeyDown( int key )
{
    return getKey( key ) == GLFW_PRESS;
}

}; // namespace gfx
