#define GLFW_INCLUDE_NONE

#include "Simulation.hpp"
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

namespace sim
{

using namespace gl;

Parameters ConstParameterProvider::getParameters()
{
    return Parameters {
        .wiggleOffset = 4.0,
        .wiggleAmplitude = 0.2,
        .wigglesPerRevolution = 16,
        .wigglePhase = 0,
        .normExponent = 2.0/3
    };
}

void setupImgui()
{
}

void run(
    gfx::Window &window,
    boost::lockfree::spsc_queue<float> &rbuf,
    std::string resourcesPath,
    IParameterProvider &parameterProvider
)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window.glfwWindow, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    auto program = makeShaderProgram( resourcesPath );

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
    glGenTextures( 1, &tex );
    glBindTexture( GL_TEXTURE_1D, tex );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // uniform
    GLuint paramsWiggleOffsetLoc = glGetUniformLocation(program, "params.wiggleOffset");
    GLuint paramsWiggleAmplitudeLoc = glGetUniformLocation(program, "params.wiggleAmplitude");
    GLuint paramsWigglesPerRevolutionLoc = glGetUniformLocation(program, "params.wigglesPerRevolution");
    GLuint paramsWigglePhaseLoc = glGetUniformLocation(program, "params.wigglePhase");
    GLuint paramsNormExponentLoc = glGetUniformLocation(program, "params.normExponent");

    std::vector<float> buf( sfx::FRAMES_PER_BUFFER );
    std::vector<float> texData( sfx::FRAMES_PER_BUFFER );
    Parameters params = parameterProvider.getParameters();
    while ( !window.shouldClose() ) {
        if ( window.isKeyDown( GLFW_KEY_Q ) )
            window.setShouldClose( true );

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Options");
        ImGui::SliderFloat("Wiggle offset", &params.wiggleOffset, 2.0f, 10.0f);
        ImGui::SliderFloat("Wiggle amplitude", &params.wiggleAmplitude, 0.0f, 0.5f);
        ImGui::SliderFloat("Wiggles per revolution", &params.wigglesPerRevolution, 1, 64);
        ImGui::SliderFloat("Wiggle phase", &params.wigglePhase, 0.0f, 6.28f);
        ImGui::SliderFloat("Star Shape", &params.normExponent, 0.1f, 2.0f);
        ImGui::End();

        glUniform1f(paramsWiggleOffsetLoc, params.wiggleOffset);
        glUniform1f(paramsWiggleAmplitudeLoc, params.wiggleAmplitude);
        glUniform1f(paramsWigglesPerRevolutionLoc, params.wigglesPerRevolution);
        glUniform1f(paramsWigglePhaseLoc, params.wigglePhase);
        glUniform1f(paramsNormExponentLoc, params.normExponent);

        // wait for full buffer
        while ( rbuf.write_available() )
            ;

        // populate texture data from sound
        size_t numRead = rbuf.pop( buf.data(), sfx::FRAMES_PER_BUFFER );
        for ( int i = 0; i < numRead; i++ ) {
            texData [ i ] = std::abs( buf [ i ] );
        }
        glTexImage1D( GL_TEXTURE_1D, 0, GL_RED, numRead, 0, GL_RED, GL_FLOAT, texData.data() );
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


        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwPollEvents();
        window.swapBuffers();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    std::cout << "GL cleanup..." << std::endl;
    glDeleteBuffers( 1, &vb );
    glDeleteVertexArrays( 1, &va );
}

gfx::Program makeShaderProgram( std::string resourcesPath )
{
    auto vertexShader = gfx::Shader::fromFile( gfx::ShaderType::Vertex, resourcesPath + "/shaders/vert.glsl" );
    if ( !vertexShader )
        throw std::runtime_error( "error creating vertex shader: " + vertexShader.error().error );

    auto fragmentShader = gfx::Shader::fromFile( gfx::ShaderType::Fragment, resourcesPath + "/shaders/frag.glsl" );
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

} // namespace sim
