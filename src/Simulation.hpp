#pragma once

#include "Graphics.hpp"
#include "Stereo.hpp"
#include <boost/lockfree/spsc_queue.hpp>

namespace sim
{

void run( gfx::Window &window, boost::lockfree::spsc_queue<Stereo<float>> &rbuf );

}
