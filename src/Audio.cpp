#include "Audio.hpp"

#include <boost/lockfree/spsc_queue.hpp>
#include <portaudiocpp/PortAudioCpp.hxx>

#include "Stereo.hpp"

namespace sfx
{

portaudio::Device *findPulseDevice( portaudio::System &sys )
{
    // loop through devices found on the system
    for ( auto &&devIter = sys.devicesBegin(); devIter != sys.devicesEnd(); devIter++ ) {
        // find PulseAudio device, called "pulse"
        if ( devIter->name() == std::string( "pipewire" ) ) {
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

int audioCallback(
    const void *inputBuffer,
    void *outputBuffer,
    unsigned long numFrames,
    const PaStreamCallbackTimeInfo *timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData
)
{
    auto rbuf = static_cast<boost::lockfree::spsc_queue<Stereo<float>> *>( userData );
    const Stereo<float> *in = static_cast<const Stereo<float> *>( inputBuffer );
    if (!rbuf->read_available())
      rbuf->push( in, numFrames );

    return paContinue;
}

portaudio::FunCallbackStream createInputStream(
    portaudio::Device &device,
    boost::lockfree::spsc_queue<Stereo<float>> &buffer,
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

    portaudio::StreamParameters params(                       //
        inParams,                                             // inputParameters
        portaudio::DirectionSpecificStreamParameters::null(), // outpuParameters
        sampleRate,                                           // sampleRate
        framesPerBuffer,                                      // framesPerBuffer
        paClipOff                                             // flags
    );

    return portaudio::FunCallbackStream( params, &sfx::audioCallback, &buffer );
}

StreamGuard::StreamGuard( portaudio::Stream &stream ) : stream_( stream )
{
    stream_.start();
}
StreamGuard::~StreamGuard()
{
    stream_.stop();
}

} // namespace sfx
