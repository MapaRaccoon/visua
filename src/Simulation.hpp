#pragma once

#include "Graphics.hpp"
#include "Shader.hpp"
#include "Stereo.hpp"
#include <boost/lockfree/spsc_queue.hpp>

namespace sim
{

struct Parameters
{
    float wiggleOffset;
    float wiggleScale;
    float wigglesPerRevolution;
    float normExponent;
};

class IParameterProvider
{
    virtual Parameters getParameters() = 0;
    virtual ~IParameterProvider();
};

void run(
    gfx::Window &window,
    boost::lockfree::spsc_queue<float> &rbuf,
    std::string resourcesPath,
    IParameterProvider &parameterProvider
);
gfx::Program makeShaderProgram( std::string resourcesPath );

} // namespace sim
