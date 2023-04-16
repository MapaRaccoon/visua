#include <glbinding/gl/bitfield.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <glm/glm.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include "portaudiocpp/PortAudioCpp.hxx"
#include "Stereo.hpp"

#include <iostream>
#include <vector>

#include "Shader.h"

using namespace gl;

const gl::GLsizei WIDTH  = 800;
const gl::GLsizei HEIGHT = 608;

const double SAMPLE_RATE       = 44100.0;
const int    FRAMES_PER_BUFFER = 1024;

GLFWwindow *initializeWindow( const char *name, gl::GLsizei width, gl::GLsizei height );
void        framebufferSizeCallback( GLFWwindow *window, int width, int height );
void        glfwError( int error, const char *msg );
void        processInput( GLFWwindow *window );
int         audioCallback( const void *inputBuffer, void *outputBuffer, unsigned long numFrames,
                           const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData );

portaudio::Device *findPulseDevice( portaudio::System &sys );
void               listDevices( portaudio::System &sys );

int main( void )
{
    // portaudio stuff
    portaudio::AutoSystem autoSys;
    portaudio::System    &sys = portaudio::System::instance();
    listDevices( sys );
    std::cout << "Searching for PulseAudio device..." << std::endl;
    portaudio::Device *pulse = findPulseDevice( sys );
    if ( !pulse ) {
        std::cerr << "Pulse device not found" << std::endl;
        return 1;
    }

    // Set up the parameters required to open a (Callback)Stream:
    portaudio::DirectionSpecificStreamParameters inParams( //
        *pulse,                                            // device
        2,                                                 // numChannels
        portaudio::FLOAT32,                                // format
        true,                                              // interleaved
        pulse->defaultLowInputLatency(),                   // suggestedLatency
        NULL                                               // hostApiSpecifiStreamInfo
    );

    portaudio::StreamParameters params(                       //
        inParams,                                             // inputParameters
        portaudio::DirectionSpecificStreamParameters::null(), // outpuParameters
        SAMPLE_RATE,                                          // sampleRate
        FRAMES_PER_BUFFER,                                    // framesPerBuffer
        paClipOff                                             // flags
    );

    std::cout << "Initializing ring buffer..." << std::endl;
    std::vector<Stereo<float>> buf(FRAMES_PER_BUFFER);
    boost::lockfree::spsc_queue<Stereo<float>> rbuf {FRAMES_PER_BUFFER};

    std::cout << "Opening stereo input stream..." << std::endl;
    portaudio::FunCallbackStream stream( params, &audioCallback, &rbuf );

    std::cout << "Starting stream" << std::endl;

    stream.start();

    // OpenGL stuff
    glfwSetErrorCallback( glfwError );
    auto window = initializeWindow( "game", WIDTH, HEIGHT );
    if ( window == nullptr )
        return -1;

    static const GLfloat triangleVerts [] = {
        -1.0f, -1.0f, 0.0f, -1.0f, 3.0f, 0.0f, 3.0f, -1.0f, 0.0f,
    };

    GLuint va;
    glGenVertexArrays( 1, &va );
    glBindVertexArray( va );

    GLuint shader = gfx::loadShaders( "resources/shaders/vert.glsl", "resources/shaders/frag.glsl" );

    GLuint vb;
    glGenBuffers( 1, &vb );
    glBindBuffer( GL_ARRAY_BUFFER, vb );
    glBufferData( GL_ARRAY_BUFFER, sizeof( triangleVerts ), triangleVerts, GL_STATIC_DRAW );

    while ( !glfwWindowShouldClose( window ) ) {
        processInput( window );
        size_t numRead = rbuf.pop(buf.data(), FRAMES_PER_BUFFER);
        for ( int i = 0; i < numRead; i++ ) {
            std::cout << buf[i] << std::endl;
        }

        glClearColor( 0, 0, 1, 0 );
        glClear( GL_COLOR_BUFFER_BIT );

        glUseProgram( shader );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, vb );
        glVertexAttribPointer( 0,           // index
                               3,           // size
                               GL_FLOAT,    // type
                               GL_FALSE,    // normalized
                               0,           // stride
                               ( void * ) 0 // array buffer offset
        );
        glDrawArrays( GL_TRIANGLES, 0, 3 );
        glDisableVertexAttribArray( 0 );

        glfwPollEvents();
        glfwSwapBuffers( window );
    }

    std::cout << "GL cleanup..." << std::endl;
    glDeleteBuffers( 1, &vb );
    glDeleteVertexArrays( 1, &va );
    glDeleteProgram( shader );

    glfwTerminate();

    std::cout << "Portaudio cleanup..." << std::endl;
    stream.stop();
    stream.close();
    sys.terminate();

    return 0;
}

GLFWwindow *initializeWindow( const char *name, gl::GLsizei width, gl::GLsizei height )
{
    glfwInit();
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_FLOATING, GLFW_TRUE );

    GLFWwindow *window = glfwCreateWindow( width, height, name, nullptr, nullptr );
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

void framebufferSizeCallback( GLFWwindow *window, int width, int height )
{
    glViewport( 0, 0, width, height );
}

void glfwError( int error, const char *msg )
{
    std::cout << "grrr (" << error << "): " << msg << std::endl;
}

void processInput( GLFWwindow *window )
{
    if ( glfwGetKey( window, GLFW_KEY_Q ) == GLFW_PRESS ) {
        glfwSetWindowShouldClose( window, true );
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
        std::string inputType = dev->isInputOnlyDevice()    ? "input-only"
                                : dev->isOutputOnlyDevice() ? "output-only"
                                : dev->isFullDuplexDevice() ? "full-duplex"
                                                            : "unknown-input-type";
        std::cout << "(" << dev->index() << ") " << dev->name() << "; " << inputType << std::endl;
    }
}

int audioCallback( const void *inputBuffer, void *outputBuffer, unsigned long numFrames,
                    const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData )
{
    auto rbuf = static_cast<boost::lockfree::spsc_queue<Stereo<float>> *>(userData);
    const Stereo<float> *in = static_cast< const Stereo<float> *>( inputBuffer );
    rbuf->push(in, numFrames);

    return paContinue;
}
