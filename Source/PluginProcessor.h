#pragma once

#include <JuceHeader.h>
#include <juce_osc/juce_osc.h>
#include <juce_audio_processors/juce_audio_processors.h>


class TransportSenderV1AudioProcessor :public juce::AudioProcessor, public juce::OSCReceiver, public juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::MessageLoopCallback>
    
{
public:
    //==============================================================================
    TransportSenderV1AudioProcessor();
    ~TransportSenderV1AudioProcessor() override;
    
    //new:
    double lastOscSendTime = 0;
    const double oscSendIntervalMs = 33.0; // 30 fps equivalent
    //endnew
    
    void oscMessageReceived(const juce::OSCMessage& message) override;

    
    void updateOscMessageLabel();
    
    
    void setPlayingState(bool isPlaying);
    
    struct TransportState
       {
           bool isPlaying = false;
           double bpm = 120.0;
           double ppqPosition = 0.0;
           int timeSigNumerator = 4;
           int timeSigDenominator = 4;
       };

       TransportState getTransportState() const { return transportState; }
    //==============================================================================
    
    struct SlaveTransportState
    {
        bool isPlaying = false;
        double bpm = 120.0;
        
        // Store bar, beat, sub-beat separately
        int bar = 1;
        int beat = 1;
        int subBeat = 1;
    };

    const SlaveTransportState& getSlaveTransportState() const { return slaveTransportState; }
    
    
    //==============================================================================
    // METHOD TO SET AND GET THE PORT #
    juce::OSCSender& getOscSender() { return oscSender; } // Getter function to expose the oscSender

    bool isOscConnected() const { return oscConnected; } // expose getter function
    void setOscPort(int port) { oscPort = port; } // METHOD TO SET AND GET THE PORT # chagtpt
    int getOscPort() const { return oscPort; } // METHOD TO SET AND GET THE PORT # chagtpt
    
    juce::String getLastOscMessage() const
    {
        DBG("Fetching Last OSC Message: " + lastReceivedOSCMessage);
        return lastReceivedOSCMessage;
    }

    

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==============================================================================



private:
    
    // Slave Transport State
    SlaveTransportState slaveTransportState;
    
    juce::String lastReceivedOSCMessage; // Stores the latest OSC message
   //     void updateOscMessageLabel(); // Moved this to public
    
    int oscPort = 0; // Store the connected OSC port #
    bool oscConnected = false; // Tracks whether OSC is connected
   
    
    void sendOSCMessages();
    
    juce::OSCSender oscSender;
    juce::OSCReceiver oscReceiver;  // Listens for transport updates from Ableton
    

    
    TransportState transportState;

    void updateTransportState();
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TransportSenderV1AudioProcessor)
};
