#include "Shader.hpp"
#include <glbinding/gl/bitfield.h>
#define GLFW_INCLUDE_NONE

#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>

#include <glm/glm.hpp>

#include <boost/lexical_cast.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <variant>
#include <vector>

using namespace gl;

namespace gfx
{

namespace raii
{

GLShader::GLShader( ShaderType shaderType ) : isValid( true )
{
    id = glCreateShader( shaderTypeToEnum( shaderType ) );
}

GLShader::GLShader( GLShader &&other ) : isValid( true )
{
    other.isValid = false;
    std::swap( this->id, other.id );
}

GLShader &GLShader::operator=( GLShader &&other )
{
    other.isValid = false;
    if ( isValid )
        glDeleteShader( id );
    std::swap( this->id, other.id );
    return *this;
}

GLShader::~GLShader()
{
    if ( isValid )
        glDeleteShader( id );
}

GLProgram::GLProgram() : isValid( true )
{
    id = glCreateProgram();
}

GLProgram::GLProgram( GLProgram &&other ) : isValid( true )
{
    other.isValid = false;
    std::swap( this->id, other.id );
}

GLProgram &GLProgram::operator=( GLProgram &&other )
{
    other.isValid = false;
    if ( isValid )
        glDeleteProgram( id );
    std::swap( this->id, other.id );

    this->isValid = true;
    return *this;
}

GLProgram::~GLProgram()
{
    if ( isValid )
        glDeleteProgram( id );
}

} // namespace raii

Shader::Shader( raii::GLShader &&shader ) : internal( std::move( shader ) ) { }
Program::Program( raii::GLProgram &&program ) : internal( std::move( program ) ) { }

std::expected<Shader, ShaderError> Shader::create( ShaderType shaderType, const std::string &code )
{
    raii::GLShader shader( shaderType );

    char const *pSrc = code.c_str();
    glShaderSource( shader.id, 1, &pSrc, NULL );
    glCompileShader( shader.id );

    // check compile result
    GLboolean compileResult = GL_FALSE;
    int logLength;
    glGetShaderiv( shader.id, GL_COMPILE_STATUS, &compileResult );
    glGetShaderiv( shader.id, GL_INFO_LOG_LENGTH, &logLength );
    if ( logLength > 0 ) {
        std::vector<char> errorMessage( logLength + 1 );
        glGetShaderInfoLog( shader.id, logLength, NULL, errorMessage.data() );
        return std::unexpected( ShaderError{ .error = std::string( errorMessage.data() ) } );
    }
    return Shader( std::move( shader ) );
}

std::expected<Shader, ShaderError> Shader::fromFile( ShaderType shaderType, const std::string &filePath )
{
    const std::ifstream in( filePath, std::ios::in );
    if ( in.fail() )
        return std::unexpected( ShaderError{ .error = "file not found: " + filePath } );

    std::stringstream buf;
    buf << in.rdbuf();

    return create( shaderType, buf.str() );
}

std::expected<Program, ShaderError> Program::create( std::vector<Shader> &shaders )
{
    raii::GLProgram program;
    for ( auto &shader : shaders ) {
        glAttachShader( program.id, shader.internal.id );
    }

    glLinkProgram( program.id );

    for ( auto &shader : shaders ) {
        glDetachShader( program.id, shader.internal.id );
    }

    // check link result
    GLboolean linkSuccessful = GL_FALSE;
    int logLength;
    glGetProgramiv( program.id, GL_LINK_STATUS, &linkSuccessful );
    glGetProgramiv( program.id, GL_INFO_LOG_LENGTH, &logLength );
    if ( logLength > 0 && !linkSuccessful ) {
        std::vector<char> errorMessage( logLength + 1 );
        glGetProgramInfoLog( program.id, logLength, NULL, errorMessage.data() );
        return std::unexpected( ShaderError{ .error = std::string( errorMessage.data() ) } );
    }

    return Program( std::move( program ) );
}

void Program::use()
{
    glUseProgram( internal.id );
}

GLenum shaderTypeToEnum( ShaderType shaderType )
{
    switch ( shaderType ) {
    case ShaderType::Vertex:
        return GL_VERTEX_SHADER;
    case ShaderType::Fragment:
        return GL_FRAGMENT_SHADER;
    default:
        throw std::runtime_error( "Unknown shader type: " );
    }
}

} // namespace gfx
