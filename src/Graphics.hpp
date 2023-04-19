#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glbinding/gl/gl.h>
#include <optional>
#include <string>

namespace gfx
{

class Window
{
  public:
    Window( std::string name, gl::GLsizei width, gl::GLsizei height );
    Window( Window const & ) = delete;
    Window &operator=( Window const & ) = delete;
    Window( Window && );
    Window &operator=( Window && );
    ~Window();

    static std::optional<Window> create( std::string name, gl::GLsizei width, gl::GLsizei height );

    int getKey( int key );
    bool isKeyDown( int key );
    bool shouldClose();
    void swapBuffers();
    void setShouldClose( bool );

  private:
    std::string name_;
    gl::GLsizei width_;
    gl::GLsizei height_;
    GLFWwindow *glfwWindow_;
    bool isMovedFrom_;
};

void framebufferSizeCallback( GLFWwindow *window, int width, int height );
void glfwError( int error, char const *msg );

} // namespace gfx
