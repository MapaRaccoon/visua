#define GLFW_INCLUDE_NONE

#include "Simulation.hpp"
#include "Audio.hpp"
#include "Graphics.hpp"
#include "Shader.hpp"
#include "Stereo.hpp"
#include <boost/lockfree/spsc_queue.hpp>
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>

namespace sim
{

using namespace gl;

void run( gfx::Window &window, boost::lockfree::spsc_queue<Stereo<float>> &rbuf )
{
    std::vector<Stereo<float>> buf( sfx::FRAMES_PER_BUFFER );

    static const GLfloat triangleVerts [] = {
        -1.0f,
        -1.0f,
        0.0f,
        -1.0f,
        3.0f,
        0.0f,
        3.0f,
        -1.0f,
        0.0f,
    };

    GLuint va;
    glGenVertexArrays( 1, &va );
    glBindVertexArray( va );

    GLuint shader = gfx::loadShaders( "resources/shaders/vert.glsl", "resources/shaders/frag.glsl" );

    GLuint vb;
    glGenBuffers( 1, &vb );
    glBindBuffer( GL_ARRAY_BUFFER, vb );
    glBufferData( GL_ARRAY_BUFFER, sizeof( triangleVerts ), triangleVerts, GL_STATIC_DRAW );

    while ( !window.shouldClose() ) {
        if ( window.isKeyDown( GLFW_KEY_Q ) )
            window.setShouldClose( true );
        size_t numRead = rbuf.pop( buf.data(), sfx::FRAMES_PER_BUFFER );
        for ( int i = 0; i < numRead; i++ ) {
            std::cout << buf [ i ] << std::endl;
        }

        glClearColor( 0, 0, 1, 0 );
        glClear( GL_COLOR_BUFFER_BIT );

        glUseProgram( shader );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, vb );
        glVertexAttribPointer(
            0,           // index
            3,           // size
            GL_FLOAT,    // type
            GL_FALSE,    // normalized
            0,           // stride
            ( void * ) 0 // array buffer offset
        );
        glDrawArrays( GL_TRIANGLES, 0, 3 );
        glDisableVertexAttribArray( 0 );

        glfwPollEvents();
        window.swapBuffers();
    }

    std::cout << "GL cleanup..." << std::endl;
    glDeleteBuffers( 1, &vb );
    glDeleteVertexArrays( 1, &va );
    glDeleteProgram( shader );
}

} // namespace sim
