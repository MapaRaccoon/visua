#pragma once

#include <glbinding/gl/gl.h>
#include <expected>
#include "tl/expected.hpp"

namespace gfx
{

using namespace gl;

enum class ShaderType
{
    Vertex,
    Fragment
};

namespace raii
{
struct GLShader
{
    GLShader( ShaderType );
    ~GLShader();
    GLShader( GLShader const & ) = delete;
    GLShader &operator=( GLShader const & ) = delete;
    GLShader( GLShader && );
    GLShader &operator=( GLShader && );

    GLuint id;
    bool isValid;
};

struct GLProgram
{
    GLProgram( );
    ~GLProgram();
    GLProgram( GLProgram const & ) = delete;
    GLProgram &operator=( GLProgram const & ) = delete;
    GLProgram( GLProgram && );
    GLProgram &operator=( GLProgram && );

    GLuint id;
    bool isValid;
};
} // namespace raii

struct ShaderError
{
    std::string error;
};

GLenum shaderTypeToEnum( ShaderType shaderType );

class Shader
{
  public:
    static tl::expected<Shader, ShaderError> create( ShaderType shaderType, const std::string &code );
    static tl::expected<Shader, ShaderError> fromFile( ShaderType shaderType, const std::string &filePath );
    raii::GLShader internal;

  private:
    Shader( raii::GLShader &&shader );
};

class Program
{
  public:
    static tl::expected<Program, ShaderError> create( std::vector<Shader> &shaders );
    raii::GLProgram internal;
    void use();
    operator GLuint() const;

  private:
    Program( raii::GLProgram &&program );
};

gl::GLuint loadShaders( std::string vertex_file_path, std::string fragment_file_path );

} // namespace gfx
