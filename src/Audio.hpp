#pragma once

#include "Stereo.hpp"
#include "portaudiocpp/PortAudioCpp.hxx"
#include <boost/lockfree/spsc_queue.hpp>

namespace sfx
{

const double SAMPLE_RATE = 44100.0;
const int FRAMES_PER_BUFFER = 1024;

int audioCallback(
    const void *inputBuffer,
    void *outputBuffer,
    unsigned long numFrames,
    const PaStreamCallbackTimeInfo *timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData
);

portaudio::Device *findPulseDevice( portaudio::System &sys );
void listDevices( portaudio::System &sys );
portaudio::FunCallbackStream createInputStream(
    portaudio::Device &device,
    boost::lockfree::spsc_queue<Stereo<float>> &buffer,
    unsigned long framesPerBuffer,
    double sampleRate
);

class StreamGuard
{
  public:
    StreamGuard( portaudio::Stream & );
    ~StreamGuard();
    StreamGuard( const StreamGuard & ) = delete;
    StreamGuard operator=( const StreamGuard & ) = delete;
    StreamGuard( const StreamGuard && ) = delete;
    StreamGuard operator=( const StreamGuard && ) = delete;

  private:
    portaudio::Stream &stream_;
};

} // namespace sfx
