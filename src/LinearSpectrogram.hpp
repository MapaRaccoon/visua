#pragma once

#include "Graphics.hpp"
#include "Shader.hpp"
#include "Stereo.hpp"
#include <boost/lockfree/spsc_queue.hpp>

namespace vis::linear_spectrogram
{

void run(
    gfx::Window &window,
    boost::lockfree::spsc_queue<float> &rbuf,
    std::string resourcesPath
);

gfx::Program makeShaderProgram( std::string resourcesPath );

} // namespace sim
