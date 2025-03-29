#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


//==============================================================================



//==============================================================================
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override
    {
       // return juce::Font(30.0f, juce::Font::bold);
        return juce::Font(20.0f, juce::Font::bold);
    }
};
//==============================================================================
class TransportSenderV1AudioProcessorEditor  :
    public juce::AudioProcessorEditor,
    public juce::Timer,
    public juce::Button::Listener
{
    
public:
    TransportSenderV1AudioProcessorEditor (TransportSenderV1AudioProcessor&);
    ~TransportSenderV1AudioProcessorEditor() override;

    //==============================================================================
    void buttonClicked(juce::Button*) override;
    void togglePlayState();
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void updateLabels();
    void updateOscMessageLabel();
    void updateTransportLabels();

private:
    
    //OSC from Ableton Labels (display only)
    TransportSenderV1AudioProcessor& processorRef;
    juce::Label tempoLabel;
    juce::Label locationLabel;
    juce::Label playStateLabel;
    
    CustomLookAndFeel customLookAndFeel;
    
    
    
    juce::TextButton playButton {"Playing"}; // Triangle play
    
    juce::Label oscStatusLabel; // Displays OSC connection status
    
    // juce::Label oscMessageLabel; // Label to show received OSC messages
    
    TransportSenderV1AudioProcessor& audioProcessor;

    juce::Rectangle<int> bpmBox; // BPM Box variable
    juce::Label bpmTextLabel; // Text Label above the BPM box

    juce::Rectangle<int> positionBox; // Position Label Box variable
    juce::Label positionTextLabel; // Text label above position Label
    juce::Label isPlayingLabel;
    juce::Label bpmLabel;
    juce::Label positionLabel;
    
    
    // Stagnant labels
    juce::Label websiteLabel; // Label for "alexfortunatomusic.com"
    juce::Label pluginTitle; // Title at the top-left of the plugin window
    juce::Label abletonDataLabel;
    

    // TransportDisplay transportDisplay; // Embedding Play Button UI

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TransportSenderV1AudioProcessorEditor)
};

