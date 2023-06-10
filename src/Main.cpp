#include <glbinding/gl/bitfield.h>
#define GLFW_INCLUDE_NONE
#include "Audio.hpp"
#include "Graphics.hpp"
#include "StarVisualizer.hpp"
#include "BarsVisualizer.hpp"
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
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

const gl::GLsizei WIDTH = 800;
const gl::GLsizei HEIGHT = 608;

// TODO: fix the segfault on quit

void run(gfx::Window &window, boost::lockfree::spsc_queue<float> &rbuf, std::string resourcesPath)
{
    // init gui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL( window.glfwWindow, true );
    ImGui_ImplOpenGL3_Init( "#version 330" );

    // run visualizer
    vis::Command next;
    vis::VisualizerType visualizerType = vis::VisualizerType::Star;
    std::unique_ptr<vis::Visualizer> visualizer = std::make_unique<vis::StarVisualizer>(resourcesPath, rbuf);
    while ( !window.shouldClose() ) {
        // handle key input
        if ( window.isKeyDown( GLFW_KEY_Q ) )
            window.setShouldClose( true );

        if (window.isKeyDown( GLFW_KEY_1 ) && visualizerType != vis::VisualizerType::Star) {
            visualizerType = vis::VisualizerType::Star;
            visualizer = std::make_unique<vis::StarVisualizer>(resourcesPath, rbuf);
        }

        if (window.isKeyDown( GLFW_KEY_2 ) && visualizerType != vis::VisualizerType::Bars) {
            visualizerType = vis::VisualizerType::Bars;
            visualizer = std::make_unique<vis::BarsVisualizer>(resourcesPath, rbuf);
        }

        // step visualizer
        next = visualizer->step();
        if (next == vis::Command::Quit) {
            window.setShouldClose( true );
        }

        // make UI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        visualizer->doUi();

        // draw visualizer
        visualizer->draw();

        // draw UI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

        glfwPollEvents();
        window.swapBuffers();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

int main( void )
{
    std::string resourcesPath;
    if ( const char *visuaDir = std::getenv( "VISUA_RESOURCES_PATH" ) )
        resourcesPath = visuaDir;
    else
        resourcesPath = "./resources";

    // TODO: why does this spam stdout so much
    portaudio::AutoSystem autoSys;
    portaudio::System &sys = portaudio::System::instance();

    std::cout << "Listing devices:" << std::endl;
    sfx::listDevices( sys );

    // TODO: cli to choose device
    std::cout << "Searching for PipeWire device..." << std::endl;
    portaudio::Device *pulse = sfx::findDeviceByName( sys, "pulse" );
    if ( !pulse )
        throw std::runtime_error( "pulse device not found" );

    boost::lockfree::spsc_queue<float> rbuf( sfx::FRAMES_PER_BUFFER );
    fftw_complex *fftIn = static_cast<fftw_complex *>( fftw_malloc( sizeof( fftw_complex ) * sfx::FRAMES_PER_BUFFER ) );
    fftw_complex *fftOut =
        static_cast<fftw_complex *>( fftw_malloc( sizeof( fftw_complex ) * sfx::FRAMES_PER_BUFFER ) );
    fftw_plan fftPlan = fftw_plan_dft_1d( sfx::FRAMES_PER_BUFFER, fftIn, fftOut, FFTW_FORWARD, FFTW_MEASURE );
    sfx::PlaybackToFFT
        fftOutputPlayback( rbuf, { fftIn, sfx::FRAMES_PER_BUFFER }, { fftOut, sfx::FRAMES_PER_BUFFER }, fftPlan );

    // start stream which collects frequency spectrum data into rbuf  
    std::cout << "Starting stream" << std::endl;
    auto streamParams = sfx::getInputStreamParameters( *pulse, sfx::FRAMES_PER_BUFFER, sfx::SAMPLE_RATE );
    auto stream = portaudio::MemFunCallbackStream( streamParams, fftOutputPlayback, &sfx::PlaybackToFFT::callback );
    auto streamInputGuard = sfx::StreamGuard( stream );

    // run visualizers
    if ( auto window = gfx::Window::create( "woof", WIDTH, HEIGHT ) ) {
        run(*window, rbuf, resourcesPath);
    } else {
        throw std::runtime_error( "failed to create GLFW window" );
    }

    fftw_destroy_plan( fftPlan );
    fftw_free( fftIn );
    fftw_free( fftOut );

    return 0;
}
