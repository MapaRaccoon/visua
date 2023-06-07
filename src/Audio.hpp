#pragma once

#include "Stereo.hpp"
#include <boost/lockfree/spsc_queue.hpp>
#include <fftw3.h>
#include <portaudiocpp/PortAudioCpp.hxx>
#include <span>

namespace sfx
{

const double SAMPLE_RATE = 44100.0;
const int FRAMES_PER_BUFFER = 2048;

class PlaybackToFFT
{
  public:
    PlaybackToFFT(
        boost::lockfree::spsc_queue<float> &freqOut,
        std::span<fftw_complex> fftIn,
        std::span<fftw_complex> fftOut,
        fftw_plan &fftPlan
    );

    int callback(
        const void *inputBuffer,
        void *outputBuffer,
        unsigned long numFrames,
        const PaStreamCallbackTimeInfo *timeInfo,
        PaStreamCallbackFlags statusFlags
    );

  private:
    boost::lockfree::spsc_queue<float> &mFreqOut;
    std::span<fftw_complex> mFftIn;
    std::span<fftw_complex> mFftOut;
    fftw_plan &mFftPlan;
};

portaudio::Device *findDeviceByName( portaudio::System &sys, const std::string &name );
void listDevices( portaudio::System &sys );
portaudio::StreamParameters getInputStreamParameters(
    portaudio::Device &device,
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
