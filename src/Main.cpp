#include <glbinding/gl/bitfield.h>
#define GLFW_INCLUDE_NONE
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "portaudiocpp/PortAudioCpp.hxx"

#include <iostream>
#include <vector>

#include "Shader.h"

using namespace gl;

gl::GLsizei width  = 800;
gl::GLsizei height = 608;

GLFWwindow * initializeWindow( const char * name, gl::GLsizei width, gl::GLsizei height );
void         framebufferSizeCallback( GLFWwindow * window, int width, int height );
void glfwError(int error, const char* msg);
void processInput(GLFWwindow *window);

portaudio::Device *findPulseDevice( portaudio::System &sys );
void listDevices( portaudio::System &sys );

int main( void )
{
    // portaudio stuff
    portaudio::AutoSystem autoSys;
    portaudio::System &sys = portaudio::System::instance();
    listDevices(sys);

    // OpenGL stuff
    glfwSetErrorCallback(glfwError);
    auto window = initializeWindow( "game", width, height );
    if ( window == nullptr )
        return -1;


    static const GLfloat triangleVerts[] = {
        -1.0f, -1.0f, 0.0f,
        -1.0f,  3.0f, 0.0f,
         3.0f, -1.0f, 0.0f,
    };

    GLuint va;
    glGenVertexArrays(1, &va);
    glBindVertexArray(va);

    GLuint shader = gfx::loadShaders("resources/shaders/vert.glsl", "resources/shaders/frag.glsl");

    GLuint vb;
    glGenBuffers(1, &vb);
    glBindBuffer(GL_ARRAY_BUFFER, vb);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVerts), triangleVerts, GL_STATIC_DRAW);

    while ( !glfwWindowShouldClose( window ) ) {
        processInput(window);

        glClearColor( 0, 0, 1, 0 );
        glClear( GL_COLOR_BUFFER_BIT );

        glUseProgram(shader);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vb);
        glVertexAttribPointer(
            0,        // index
            3,        // size
            GL_FLOAT, // type
            GL_FALSE, // normalized
            0,        // stride
            (void*)0  // array buffer offset
        );
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDisableVertexAttribArray(0);

        glfwPollEvents();
        glfwSwapBuffers( window );
    }

    glDeleteBuffers(1, &vb);
    glDeleteVertexArrays(1, &va);
    glDeleteProgram(shader);

    glfwTerminate();

    return 0;
}

GLFWwindow * initializeWindow( const char * name, gl::GLsizei width, gl::GLsizei height )
{
    glfwInit();
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_FLOATING, GLFW_TRUE );

    GLFWwindow * window = glfwCreateWindow( width, height, name, nullptr, nullptr );
    if ( window == nullptr ) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return window;
    }
    glfwMakeContextCurrent( window );

    glbinding::initialize( glfwGetProcAddress );

    glViewport( 0, 0, width, height );
    glfwSetFramebufferSizeCallback( window, framebufferSizeCallback );
    // glfwSetCursorPosCallback( window, mouse_callback );

    return window;
}

void framebufferSizeCallback( GLFWwindow * window, int width, int height )
{
    glViewport( 0, 0, width, height );
}

void glfwError(int error, const char* msg) {
    std::cout << "grrr (" << error << "): " << msg << std::endl;
}

void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

}

portaudio::Device *findPulseDevice( portaudio::System &sys )
{
    // loop through devices found on the system
    for ( auto &&devIter = sys.devicesBegin(); devIter != sys.devicesEnd(); devIter++ ) {
        // find PulseAudio device, called "pulse"
        if ( devIter->name() == std::string( "pulse" ) ) {
            // Dereference iterator into Device, then get the address
            // This device has static lifetime beginning with System initialization
            return &*devIter;
        }
    }

    // if pulse device not found, return nothing
    return nullptr;
}

void listDevices( portaudio::System &sys )
{
    // loop through devices found on the system
    for ( auto &&dev = sys.devicesBegin(); dev != sys.devicesEnd(); dev++ ) {
        // find PulseAudio device, called "pulse"
        std::string inputType = dev->isInputOnlyDevice() 
            ? "input-only"
            : dev->isOutputOnlyDevice() 
            ? "output-only"
            : dev->isFullDuplexDevice()
            ? "full-duplex"
            : "unknown-input-type";
        std::cout << "(" << dev->index() << ") " << dev->name() << "; " << inputType << std::endl;
    }
}

