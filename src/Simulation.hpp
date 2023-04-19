#pragma once

#include <boost/lockfree/spsc_queue.hpp>
#include "Stereo.hpp"
#include "Graphics.hpp"

namespace sim
{

void run( gfx::Window &window, boost::lockfree::spsc_queue<Stereo<float>> &rbuf );

}
