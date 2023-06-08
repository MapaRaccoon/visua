#pragma once

#include "Audio.hpp"
#include "Graphics.hpp"
#include "Shader.hpp"
#include "Stereo.hpp"
#include "Visualizer.hpp"
#include <boost/lockfree/spsc_queue.hpp>
#include <glbinding/gl/gl.h>

namespace vis
{

using namespace gl;

class RadialSpectrogram : public Visualizer
{
  public:
    RadialSpectrogram( std::string resourcesPath, boost::lockfree::spsc_queue<float> &rbuf );
    ~RadialSpectrogram();
    Command step() override;
    void draw() override;
    void doUi() override;

  private:
    void updateUniforms();
    gfx::Program makeShaderProgram( std::string resourcesPath );

    struct
    {
        float wiggleOffset;
        float wiggleAmplitude;
        float wigglesPerRevolution;
        float wigglePhase;
        float normExponent;
    } params;

    struct
    {
        GLuint paramsWiggleOffsetLoc;
        GLuint paramsWiggleAmplitudeLoc;
        GLuint paramsWigglesPerRevolutionLoc;
        GLuint paramsWigglePhaseLoc;
        GLuint paramsNormExponentLoc;
    } uniforms;

    boost::lockfree::spsc_queue<float> &rbuf;
    gfx::Program program;

    std::vector<float> buf;
    std::vector<float> texData;

    GLuint va;
    GLuint vb;
};

void run( gfx::Window &window, boost::lockfree::spsc_queue<float> &rbuf, std::string resourcesPath );

} // namespace vis
