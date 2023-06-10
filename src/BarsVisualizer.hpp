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

class BarsVisualizer : public Visualizer
{
  public:
    BarsVisualizer( std::string resourcesPath, boost::lockfree::spsc_queue<float> &rbuf );
    ~BarsVisualizer();
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
    GLuint tex;
};

gfx::Program makeShaderProgram( std::string resourcesPath );

} // namespace vis
