#include <glbinding/gl/bitfield.h>
#define GLFW_INCLUDE_NONE
#include "Audio.hpp"
#include "Graphics.hpp"
#include "Simulation.hpp"
#include "Stereo.hpp"
#include <GLFW/glfw3.h>
#include <boost/lockfree/spsc_queue.hpp>
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <glm/glm.hpp>
#include <iostream>
#include <portaudiocpp/PortAudioCpp.hxx>
#include <vector>

const gl::GLsizei WIDTH = 800;
const gl::GLsizei HEIGHT = 608;

int main( void )
{
    portaudio::AutoSystem autoSys;
    portaudio::System &sys = portaudio::System::instance();

    std::cout << "Searching for PulseAudio device..." << std::endl;
    portaudio::Device *pulse = sfx::findPulseDevice( sys );
    if ( !pulse ) {
        std::cerr << "Pulse device not found" << std::endl;
        return 1;
    }

    boost::lockfree::spsc_queue<Stereo<float>> rbuf( sfx::FRAMES_PER_BUFFER );

    std::cout << "Starting stream" << std::endl;
    auto stream = sfx::createInputStream( *pulse, rbuf, sfx::FRAMES_PER_BUFFER, sfx::SAMPLE_RATE );
    auto streamInputGuard = sfx::StreamGuard( stream );

    // OpenGL stuff
    if ( auto window = gfx::Window::create( "woof", WIDTH, HEIGHT ) ) {
        sim::run( *window, rbuf );
    }

    return 0;
}
