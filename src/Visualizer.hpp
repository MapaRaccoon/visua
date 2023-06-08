#pragma once

#include "Graphics.hpp"
#include "Shader.hpp"
#include "Stereo.hpp"
#include <boost/lockfree/spsc_queue.hpp>

namespace vis
{

enum class Command
{
    Continue,
    Quit,
    SwitchToRadial,
    SwitchToLinear
};

struct Visualizer
{
    virtual ~Visualizer() = default;
    virtual Command step() = 0;
    virtual void draw() = 0;
    virtual void doUi() = 0;
};

} // namespace sim
