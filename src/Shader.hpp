#pragma once

#include <glbinding/gl/gl.h>

namespace gfx {

gl::GLuint loadShaders(std::string vertex_file_path, std::string fragment_file_path);

} // namespace gfx
