#define GLFW_INCLUDE_NONE

#include "RadialSpectrogram.hpp"
#include "Audio.hpp"
#include "Graphics.hpp"
#include "Shader.hpp"
#include "Stereo.hpp"
#include <boost/lockfree/spsc_queue.hpp>
#include <cmath>
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace vis
{

using namespace gl;

RadialSpectrogram::RadialSpectrogram( std::string resourcesPath, boost::lockfree::spsc_queue<float> &rbuf )
    : rbuf( rbuf )
    , params(
          { .wiggleOffset = 4.0,
            .wiggleAmplitude = 0.2,
            .wigglesPerRevolution = 16,
            .wigglePhase = 0,
            .normExponent = 2.0 / 3 }
      )
    , program( makeShaderProgram( resourcesPath ) )
    , buf(sfx::FRAMES_PER_BUFFER)
    , texData(sfx::FRAMES_PER_BUFFER)
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
    GLuint tex;
    glGenTextures( 1, &tex );
    glBindTexture( GL_TEXTURE_1D, tex );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // uniform
    uniforms.paramsWiggleOffsetLoc = glGetUniformLocation( program, "params.wiggleOffset" );
    uniforms.paramsWiggleAmplitudeLoc = glGetUniformLocation( program, "params.wiggleAmplitude" );
    uniforms.paramsWigglesPerRevolutionLoc = glGetUniformLocation( program, "params.wigglesPerRevolution" );
    uniforms.paramsWigglePhaseLoc = glGetUniformLocation( program, "params.wigglePhase" );
    uniforms.paramsNormExponentLoc = glGetUniformLocation( program, "params.normExponent" );
}

Command RadialSpectrogram::step()
{
    // wait for full buffer
    while ( rbuf.write_available() )
        ;

    // populate texture data from sound
    size_t numRead = rbuf.pop( buf.data(), sfx::FRAMES_PER_BUFFER );
    for ( int i = 0; i < numRead; i++ ) {
        texData [ i ] = std::abs( buf [ i ] );
    }

    return Command::Continue;
}

void RadialSpectrogram::updateUniforms()
{
    glUniform1f( uniforms.paramsWiggleOffsetLoc, params.wiggleOffset );
    glUniform1f( uniforms.paramsWiggleAmplitudeLoc, params.wiggleAmplitude );
    glUniform1f( uniforms.paramsWigglesPerRevolutionLoc, params.wigglesPerRevolution );
    glUniform1f( uniforms.paramsWigglePhaseLoc, params.wigglePhase );
    glUniform1f( uniforms.paramsNormExponentLoc, params.normExponent );
}

void RadialSpectrogram::doUi()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin( "Options" );
    ImGui::SliderFloat( "Wiggle offset", &params.wiggleOffset, 2.0f, 10.0f );
    ImGui::SliderFloat( "Wiggle amplitude", &params.wiggleAmplitude, 0.0f, 0.5f );
    ImGui::SliderFloat( "Wiggles per revolution", &params.wigglesPerRevolution, 1, 64 );
    ImGui::SliderFloat( "Wiggle phase", &params.wigglePhase, 0.0f, 6.28f );
    ImGui::SliderFloat( "Star Shape", &params.normExponent, 0.1f, 2.0f );
    ImGui::End();

    updateUniforms();
}

void RadialSpectrogram::draw()
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

RadialSpectrogram::~RadialSpectrogram()
{
    glDeleteBuffers( 1, &vb );
    glDeleteVertexArrays( 1, &va );
}

gfx::Program RadialSpectrogram::makeShaderProgram( std::string resourcesPath )
{
    auto vertexShader = gfx::Shader::fromFile( gfx::ShaderType::Vertex, resourcesPath + "/shaders/identity.vert" );
    if ( !vertexShader )
        throw std::runtime_error( "error creating vertex shader: " + vertexShader.error().error );

    auto fragmentShader =
        gfx::Shader::fromFile( gfx::ShaderType::Fragment, resourcesPath + "/shaders/radial_spectrogram.frag" );
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
