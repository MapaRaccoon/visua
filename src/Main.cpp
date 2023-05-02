#include <glbinding/gl/bitfield.h>
#define GLFW_INCLUDE_NONE
#include "Audio.hpp"
#include "Graphics.hpp"
#include "Simulation.hpp"
#include "Stereo.hpp"
#include <GLFW/glfw3.h>
#include <boost/lockfree/spsc_queue.hpp>
#include <cstdlib>
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
    std::string resourcesPath;
    if ( const char *visuaDir = std::getenv( "VISUA_RESOURCES_PATH" ) )
        resourcesPath = visuaDir;
    else
        resourcesPath = "./resources";

    portaudio::AutoSystem autoSys;
    portaudio::System &sys = portaudio::System::instance();

    std::cout << "Listing devices:" << std::endl;
    sfx::listDevices( sys );

    std::cout << "Searching for PipeWire device..." << std::endl;
    portaudio::Device *pulse = sfx::findDeviceByName( sys, "pulse" );
    if ( !pulse )
        throw std::runtime_error( "pulse device not found" );

    boost::lockfree::spsc_queue<float> rbuf( sfx::FRAMES_PER_BUFFER );
    fftw_complex *fftIn = static_cast<fftw_complex *> (fftw_malloc(sizeof(fftw_complex) * sfx::FRAMES_PER_BUFFER));
    fftw_complex *fftOut = static_cast<fftw_complex *>(fftw_malloc(sizeof(fftw_complex) * sfx::FRAMES_PER_BUFFER));
    fftw_plan fftPlan = fftw_plan_dft_1d(sfx::FRAMES_PER_BUFFER, fftIn, fftOut, FFTW_FORWARD, FFTW_MEASURE);
    sfx::PlaybackToFFT fftOutputPlayback(rbuf, { fftIn, sfx::FRAMES_PER_BUFFER }, { fftOut, sfx::FRAMES_PER_BUFFER }, fftPlan);

    {
        std::cout << "Starting stream" << std::endl;
        auto streamParams = sfx::getInputStreamParameters( *pulse, sfx::FRAMES_PER_BUFFER, sfx::SAMPLE_RATE );
        auto stream = portaudio::MemFunCallbackStream( streamParams, fftOutputPlayback, &sfx::PlaybackToFFT::callback );
        auto streamInputGuard = sfx::StreamGuard( stream );

        // OpenGL stuff
        if ( auto window = gfx::Window::create( "woof", WIDTH, HEIGHT ) ) {
            sim::run( *window, rbuf, resourcesPath );
        } else {
            throw std::runtime_error( "failed to create GLFW window" );
        }
    }

    fftw_destroy_plan(fftPlan);
    fftw_free(fftIn);
    fftw_free(fftOut);

    return 0;
}
