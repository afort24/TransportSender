#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "OSCMessageSenderThread.h"
//
//==============================================================================

TransportSenderV1AudioProcessor::TransportSenderV1AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
                     #if !JucePlugin_IsMidiEffect
                      #if !JucePlugin_IsSynth
                       .withInput("Input", juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                      )
#endif
{
    if (oscSender.connect("127.0.0.1", 8000)) // Ensure the IP and port match Max
    {
        DBG("OSC Sender connected to 127.0.0.1:8000");
        oscConnected = true; // Set the flag to true
        setOscPort(8000); // Store the connected port
    }
    else
    {
        DBG("Error: OSC Sender failed to connect to 127.0.0.1:8000!");
        oscConnected = false;
        setOscPort(0); // Reset port to 0 if connection fails
    }
    
    
    
    // Connect OSC Receiver to listen for transport messages from Ableton
    if (oscReceiver.connect(8002)) // Old: if (oscReceiver.connect(8001)) // Explicitly reference oscReceiver
    {
        DBG("OSC Receiver connected on port 8002.");
        
        oscReceiver.addListener(this, "/play");
        oscReceiver.addListener(this, "/tempo");
        oscReceiver.addListener(this, "/position");
    }
    else
    {
        DBG("Error: OSC Receiver failed to connect!");
    }

    // Create and start the OSC sender thread
    oscThread.reset(new OSCMessageSenderThread(oscSender, oscMessageQueue, oscQueueLock));
    oscThread->startThread();
}

TransportSenderV1AudioProcessor::~TransportSenderV1AudioProcessor()
{
    if (oscThread)
    {
        oscThread->signalThreadShouldExit();
        oscThread->stopThread(100); // Wait up to 100 ms for it to stop
    }
}


//==============================================================================
// Plugin Metadata and Overrides

const juce::String TransportSenderV1AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TransportSenderV1AudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool TransportSenderV1AudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool TransportSenderV1AudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double TransportSenderV1AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TransportSenderV1AudioProcessor::getNumPrograms()
{
    return 1; // Single program
}

int TransportSenderV1AudioProcessor::getCurrentProgram()
{
    return 0; // Default program index
}

void TransportSenderV1AudioProcessor::setCurrentProgram(int index)
{
    // No-op for single program
}

const juce::String TransportSenderV1AudioProcessor::getProgramName(int index)
{
    return {}; // No program names
}

void TransportSenderV1AudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    // No-op for single program
}

//==============================================================================
// Prepare to play
void TransportSenderV1AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Calculate the number of samples corresponding to ~33ms (30 fps)
    samplesPerMessage = sampleRate / 30.0;
    sampleCounter = 0.0;
    // Reconnect OSC Sender in case of issues
    if (!oscSender.connect("127.0.0.1", 8000))
    {
        DBG("Error: OSC Sender failed to connect to 127.0.0.1:8000!");
    }
}

// Release resources
void TransportSenderV1AudioProcessor::releaseResources()
{
    // No cleanup needed for this simple OSC implementation
}

//==============================================================================

// Update transport state and send OSC messages
void TransportSenderV1AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    bool transportChanged = false;
    bool playStateChanged = false;

    if (auto* playHead = getPlayHead())
    {
        if (auto position = playHead->getPosition())
        {
            const auto& posInfo = *position;

            double newPpqPosition = posInfo.getPpqPosition().hasValue() ? *posInfo.getPpqPosition() : 0.0;
            double newBpm = posInfo.getBpm().hasValue() ? *posInfo.getBpm() : 120.0;
            bool newIsPlaying = posInfo.getIsPlaying();

            if (transportState.isPlaying != newIsPlaying)
                playStateChanged = true;

            if (transportState.ppqPosition != newPpqPosition ||
                transportState.bpm != newBpm ||
                playStateChanged)
            {
                transportState.ppqPosition = newPpqPosition;
                transportState.bpm = newBpm;
                transportState.isPlaying = newIsPlaying;
                transportChanged = true;
            }
        }
    }

    // Accumulate the number of processed samples
    sampleCounter += buffer.getNumSamples();

    // If enough samples have passed (e.g. ~33ms worth), then queue up an OSC update
    if (transportState.isPlaying && transportChanged && (sampleCounter >= samplesPerMessage))
    {
        // [We’ll push our OSC transport data to a thread-safe queue here instead of sending immediately]
        OSCTransportMessage msg;
        msg.isPlaying = transportState.isPlaying;
        msg.tempo = static_cast<float>(transportState.bpm);
        msg.position = static_cast<float>(transportState.ppqPosition);

        {
            const juce::ScopedLock lock(oscQueueLock);
            oscMessageQueue.push(msg);
        }

        sampleCounter -= samplesPerMessage; // subtract the interval (or reset to 0)
    }

    // Always update /play immediately if play state changed
    if (playStateChanged)
    {
        // Push play state change as a separate OSC message if needed.
        OSCTransportMessage playMsg;
        playMsg.isPlaying = transportState.isPlaying;
        playMsg.tempo = static_cast<float>(transportState.bpm);
        playMsg.position = static_cast<float>(transportState.ppqPosition);

        {
            const juce::ScopedLock lock(oscQueueLock);
            oscMessageQueue.push(playMsg);
        }
    }

    // Keep plugin alive with inaudible signal
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            channelData[sample] = 0.0001f;
    }
}






//==============================================================================


// Send OSC messages
void TransportSenderV1AudioProcessor::sendOSCMessages()
{
    // Attempt to reconnect if sending fails
    if (!oscSender.send("/play", transportState.isPlaying ? 1 : 0))
    {
        DBG("Failed to send /play OSC message. Attempting to reconnect...");
        if (!oscSender.connect("127.0.0.1", 8000))
        {
            DBG("Reconnection failed.");
            return;
        }
    }

    // Send tempo and position after reconnecting
    if (!oscSender.send("/tempo", static_cast<float>(transportState.bpm)))
    {
        DBG("Failed to send /tempo OSC message");
    }

    if (!oscSender.send("/position", static_cast<float>(transportState.ppqPosition)))
    {
        DBG("Failed to send /position OSC message");
    }
}



void TransportSenderV1AudioProcessor::oscMessageReceived(const juce::OSCMessage& message)
{
    auto address = message.getAddressPattern().toString();
    DBG("Received OSC: " + address);

    if (message.size() > 0)
    {
        if (address == "/tempo" && message[0].isFloat32())
        {
            slaveTransportState.bpm = message[0].getFloat32();
            DBG("Updated Tempo: " + juce::String(slaveTransportState.bpm));
        }
        else if (address == "/position" && message.size() >= 5) // ✅ Ensure at least 5 elements (int | int | int)
        {
            int receivedValues[3] = {0, 0, 0}; // Temporary storage for bar, beat, sub-beat
            int valueIndex = 0; // Track valid int values

            // ✅ Loop through message values and extract only the integers
            for (int i = 0; i < message.size(); ++i)
            {
                if (message[i].isInt32()) // ✅ Only store integer values, ignore "|"
                {
                    if (valueIndex < 3) // Store max 3 integers
                    {
                        receivedValues[valueIndex] = message[i].getInt32();
                        valueIndex++;
                    }
                }
            }

            // ✅ Only update state if we extracted 3 integers
            if (valueIndex == 3)
            {
                slaveTransportState.bar = receivedValues[0];
                slaveTransportState.beat = receivedValues[1];
                slaveTransportState.subBeat = receivedValues[2];

                DBG("Updated Position: " + juce::String(slaveTransportState.bar) + " | "
                    + juce::String(slaveTransportState.beat) + " | "
                    + juce::String(slaveTransportState.subBeat));
            }
            else
            {
                DBG("Error: /position message did not contain 3 valid integers!");
            }
        }
        else if (address == "/play" && message[0].isInt32())
        {
            slaveTransportState.isPlaying = (message[0].getInt32() == 1);
            DBG("Updated Play State: " + juce::String(slaveTransportState.isPlaying ? "true" : "false"));
        }
    }

    // ✅ Ensure UI updates when position changes
    juce::MessageManager::callAsync([this]
    {
        if (auto* editor = dynamic_cast<TransportSenderV1AudioProcessorEditor*>(getActiveEditor()))
        {
            editor->updateTransportLabels();
        }
    });
}













//Set transport state from button - maybe move this to a different section of the code later (near whatever handles playstate data)
void TransportSenderV1AudioProcessor::setPlayingState(bool isPlaying)
{
    if (isPlaying)
        transportState.isPlaying = true;  // Set transport state to playing
    else
        transportState.isPlaying = false; // Set transport state to stopped
}



//==============================================================================
// Other required methods
bool TransportSenderV1AudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* TransportSenderV1AudioProcessor::createEditor()
{
    return new TransportSenderV1AudioProcessorEditor(*this);
}

void TransportSenderV1AudioProcessor::getStateInformation(juce::MemoryBlock& destData) {}

void TransportSenderV1AudioProcessor::setStateInformation(const void* data, int sizeInBytes) {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TransportSenderV1AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}
#endif

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TransportSenderV1AudioProcessor();
}

