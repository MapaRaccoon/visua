#define GLFW_INCLUDE_NONE

#include "Simulation.hpp"
#include "Audio.hpp"
#include "Graphics.hpp"
#include "Shader.hpp"
#include "Stereo.hpp"
#include <boost/lockfree/spsc_queue.hpp>
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <cmath>

namespace sim
{

using namespace gl;

void run( gfx::Window &window, boost::lockfree::spsc_queue<Stereo<float>> &rbuf )
{
    auto program = makeShaderProgram();

    // clang-format off
    static const GLfloat triangleVerts [] = {
        -1.0f, -1.0f, 0.0f,
        -1.0f, 3.0f, 0.0f,
        3.0f, -1.0f, 0.0f,
    };
    // clang-format on

    GLuint va;
    glGenVertexArrays( 1, &va );
    glBindVertexArray( va );

    GLuint vb;
    glGenBuffers( 1, &vb );
    glBindBuffer( GL_ARRAY_BUFFER, vb );
    glBufferData( GL_ARRAY_BUFFER, sizeof( triangleVerts ), triangleVerts, GL_STATIC_DRAW );

    // create texture
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_1D, tex);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    std::vector<Stereo<float>> buf( sfx::FRAMES_PER_BUFFER );
    std::vector<float> texData( sfx::FRAMES_PER_BUFFER );
    while ( !window.shouldClose() ) {
        if ( window.isKeyDown( GLFW_KEY_Q ) )
            window.setShouldClose( true );

        // wait for full buffer
        while (rbuf.write_available());

        // populate texture data from sound
        size_t numRead = rbuf.pop( buf.data(), sfx::FRAMES_PER_BUFFER );
        for (int i = 0; i < numRead; i++) {
            texData[i] = std::abs(0.5 * (buf[i].left + buf[i].right));
            std::cout << texData[i] << std::endl;
        }
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RED, numRead, 0, GL_RED, GL_FLOAT, texData.data());
        glGenerateMipmap(GL_TEXTURE_1D);

        glClearColor( 0, 0, 1, 0 );
        glClear( GL_COLOR_BUFFER_BIT );

        program.use();

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
}

gfx::Program makeShaderProgram()
{
    auto vertexShader = gfx::Shader::fromFile( gfx::ShaderType::Vertex, "resources/shaders/vert.glsl" );
    if ( !vertexShader )
        throw std::runtime_error( "error creating vertex shader: " + vertexShader.error().error );

    auto fragmentShader = gfx::Shader::fromFile( gfx::ShaderType::Fragment, "resources/shaders/frag.glsl" );
    if ( !fragmentShader )
        throw std::runtime_error( "error creating fragment shader: " + fragmentShader.error().error );

    std::vector<gfx::Shader> shaders;
    shaders.push_back( std::move( *vertexShader ) );
    shaders.push_back( std::move( *fragmentShader ) );

    auto program = gfx::Program::create( shaders );
    if ( !program )
        throw std::runtime_error( "error creating shader program: " + program.error().error );

    return std::move(*program);
}

} // namespace sim
