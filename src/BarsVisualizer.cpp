#define GLFW_INCLUDE_NONE

#include "BarsVisualizer.hpp"
#include "Audio.hpp"
#include "Graphics.hpp"
#include "Shader.hpp"
#include "Stereo.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <boost/lockfree/spsc_queue.hpp>
#include <cmath>
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>

namespace vis
{

using namespace gl;

BarsVisualizer::BarsVisualizer( std::string resourcesPath, boost::lockfree::spsc_queue<float> &rbuf )
    : rbuf( rbuf )
    , program( makeShaderProgram( resourcesPath ) )
    , buf( sfx::FRAMES_PER_BUFFER )
    , texData( sfx::FRAMES_PER_BUFFER )
{
    // clang-format off
    static const GLfloat triangleVerts [] = {
        -1.0f, -1.0f, 0.0f,
        -1.0f, 3.0f, 0.0f,
        3.0f, -1.0f, 0.0f,
    };
    // clang-format on

    glGenVertexArrays( 1, &va );
    glBindVertexArray( va );

    glGenBuffers( 1, &vb );
    glBindBuffer( GL_ARRAY_BUFFER, vb );
    glBufferData( GL_ARRAY_BUFFER, sizeof( triangleVerts ), triangleVerts, GL_STATIC_DRAW );

    // create texture
    glGenTextures( 1, &tex );
    glBindTexture( GL_TEXTURE_1D, tex );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
}

BarsVisualizer::~BarsVisualizer()
{
    // TODO: make these RAII
    glDeleteBuffers( 1, &vb );
    glDeleteVertexArrays( 1, &va );
    glDeleteTextures( 1, &tex );
}

Command BarsVisualizer::step()
{
    // wait for full buffer
    if ( rbuf.write_available() ) {
        return Command::Continue;
    }

    // populate texture data from sound
    size_t numRead = rbuf.pop( buf.data(), sfx::FRAMES_PER_BUFFER );
    for ( int i = 0; i < numRead; i++ ) {
        texData [ i ] = std::abs( buf [ i ] );
    }

    return Command::Continue;
}

void BarsVisualizer::draw()
{
    glTexImage1D( GL_TEXTURE_1D, 0, GL_RED, sfx::FRAMES_PER_BUFFER, 0, GL_RED, GL_FLOAT, texData.data() );
    glGenerateMipmap( GL_TEXTURE_1D );

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
}

void BarsVisualizer::doUi() { }

// TODO: move this out into common function for both visualizers
gfx::Program BarsVisualizer::makeShaderProgram( std::string resourcesPath )
{
    auto vertexShader = gfx::Shader::fromFile( gfx::ShaderType::Vertex, resourcesPath + "/shaders/identity.vert" );
    if ( !vertexShader )
        throw std::runtime_error( "error creating vertex shader: " + vertexShader.error().error );

    auto fragmentShader =
        gfx::Shader::fromFile( gfx::ShaderType::Fragment, resourcesPath + "/shaders/linear_spectrogram.frag" );
    if ( !fragmentShader )
        throw std::runtime_error( "error creating fragment shader: " + fragmentShader.error().error );

    std::vector<gfx::Shader> shaders;
    shaders.push_back( std::move( *vertexShader ) );
    shaders.push_back( std::move( *fragmentShader ) );

    auto program = gfx::Program::create( shaders );
    if ( !program )
        throw std::runtime_error( "error creating shader program: " + program.error().error );

    return std::move( *program );
}

} // namespace vis
