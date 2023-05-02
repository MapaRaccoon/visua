#include "Audio.hpp"

#include <boost/lockfree/spsc_queue.hpp>
#include <portaudiocpp/PortAudioCpp.hxx>

#include "Stereo.hpp"

#include <cmath>
#include <fftw3.h>

namespace sfx
{

portaudio::Device *findDeviceByName( portaudio::System &sys, const std::string &name )
{
    // loop through devices found on the system
    for ( auto &&devIter = sys.devicesBegin(); devIter != sys.devicesEnd(); devIter++ ) {
        // find PulseAudio device, called "pulse"
        if ( devIter->name() == name ) {
            // Dereference iterator into Device, then get the address
            // This device is owned by the system
            return &*devIter;
        }
    }

    // if pulse device not found, return nothing
    return nullptr;
}

void listDevices( portaudio::System &sys )
{
    // loop through devices found on the system
    for ( auto &&dev = sys.devicesBegin(); dev != sys.devicesEnd(); dev++ ) {
        std::string inputType = dev->isInputOnlyDevice()    ? "input-only"
                                : dev->isOutputOnlyDevice() ? "output-only"
                                : dev->isFullDuplexDevice() ? "full-duplex"
                                                            : "unknown-input-type";
        std::cout << "(" << dev->index() << ") " << dev->name() << "; " << inputType << std::endl;
    }
}

portaudio::StreamParameters getInputStreamParameters(
    portaudio::Device &device,
    unsigned long framesPerBuffer,
    double sampleRate
)
{
    // Set up the parameters required to open a (Callback)Stream:
    portaudio::DirectionSpecificStreamParameters inParams( //
        device,                                            // device
        2,                                                 // numChannels
        portaudio::FLOAT32,                                // format
        true,                                              // interleaved
        device.defaultLowInputLatency(),                   // suggestedLatency
        NULL                                               // hostApiSpecifiStreamInfo
    );

    return portaudio::StreamParameters(                       //
        inParams,                                             // inputParameters
        portaudio::DirectionSpecificStreamParameters::null(), // outpuParameters
        sampleRate,                                           // sampleRate
        framesPerBuffer,                                      // framesPerBuffer
        paClipOff                                             // flags
    );
}

StreamGuard::StreamGuard( portaudio::Stream &stream ) : stream_( stream )
{
    stream_.start();
}
StreamGuard::~StreamGuard()
{
    stream_.stop();
}

PlaybackToFFT::PlaybackToFFT(
    boost::lockfree::spsc_queue<float> &freqOut,
    std::span<fftw_complex> fftIn,
    std::span<fftw_complex> fftOut,
    fftw_plan &fftPlan
)
    : mFreqOut( freqOut )
    , mFftIn( fftIn )
    , mFftOut( fftOut )
    , mFftPlan( fftPlan )
{
}

int PlaybackToFFT::callback(
    const void *inputBuffer,
    void *outputBuffer,
    unsigned long numFrames,
    const PaStreamCallbackTimeInfo *timeInfo,
    PaStreamCallbackFlags statusFlags
)
{
    // assuming all the buffers are of size `numFrames`

    // if output buffer is nonempty, skip
    if ( mFreqOut.read_available() )
        return paContinue;

    const Stereo<float> *in = static_cast<const Stereo<float> *>( inputBuffer );

    // do FFT
    for ( int i = 0; i < numFrames; i++ ) {
        mFftIn [ i ][ 0 ] = 0.5 * ( in [ i ].left + in [ i ].right );
        mFftIn [ i ][ 1 ] = 0;
    }
    fftw_execute( mFftPlan );

    for ( int i = 0; i < numFrames; i++ ) {
        const fftw_complex &val = mFftOut [ i ];
        const float amp = std::sqrt( val [ 0 ] * val [ 0 ] + val [ 1 ] * val [ 1 ] );
        mFreqOut.push( &amp, 1 );
    }

    return paContinue;
}

} // namespace sfx
