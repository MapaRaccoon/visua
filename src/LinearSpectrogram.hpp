#pragma once

#include "Graphics.hpp"
#include "Shader.hpp"
#include "Stereo.hpp"
#include "Visualizer.hpp"
#include <boost/lockfree/spsc_queue.hpp>
#include <glbinding/gl/gl.h>

namespace vis
{

using namespace gl;

class LinearSpectrogram : public Visualizer
{
  public:
    LinearSpectrogram( std::string resourcesPath, boost::lockfree::spsc_queue<float> &rbuf );
    ~LinearSpectrogram();
    Command step() override;
    void draw() override;
    void doUi() override;

  private:
    gfx::Program makeShaderProgram( std::string resourcesPath );

    boost::lockfree::spsc_queue<float> &rbuf;
    gfx::Program program;

    std::vector<float> buf;
    std::vector<float> texData;

    GLuint va;
    GLuint vb;
};

gfx::Program makeShaderProgram( std::string resourcesPath );

} // namespace vis
