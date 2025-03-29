#include "PluginEditor.h"

// BPM Box
void TransportSenderV1AudioProcessorEditor::paint(juce::Graphics& g)
{
    // Create a more subtle gradient (dark grey to black)
        juce::ColourGradient backgroundGradient(
            juce::Colour::fromRGB(30, 30, 30),  // Darker grey start (previously darkgrey)
            0, 0,
            juce::Colours::black,
            0, static_cast<float>(getHeight()),
            false
        );

        backgroundGradient.addColour(0.8, juce::Colours::black); // Smoother transition

        // Apply the gradient to the background
        g.setGradientFill(backgroundGradient);
        g.fillRect(getLocalBounds());

        float cornerSize = 10.0f;
    
    
    // Outline of Play Button
    g.setColour(juce::Colours::goldenrod);
    g.drawRoundedRectangle(playButton.getBounds().toFloat(), cornerSize, 4.0f);
    
    // Draw the rounded rectangle behind the BPM text
    g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(positionBox.toFloat(), cornerSize);
    g.fillRoundedRectangle(bpmBox.toFloat(), cornerSize);

    
    // Draw the outline of the box (optional)
    g.setColour(juce::Colours::goldenrod);
    g.drawRoundedRectangle(bpmBox.toFloat(), cornerSize, 2.0f);
    g.drawRoundedRectangle(positionBox.toFloat(), cornerSize, 2.0f);
    
    // ---- Add Static Text at Bottom Right ----
    g.setColour(juce::Colours::goldenrod);

    // Corrected font creation using FontOptions (JUCE 8+)
    juce::FontOptions fontOptions;
    fontOptions = fontOptions.withPointHeight(12.0f);  // Set font size

    juce::Font footerFont(fontOptions);  // Create font with options
    footerFont.setTypefaceName("Arial"); // Set typeface name explicitly

    g.setFont(footerFont);

    // UTF-8 Encoded String (First Part with Copyright Symbol)
    const char* utf8Text1 = u8"Alex Fortunato Music ©";
    juce::String footerText1 = juce::String::fromUTF8(utf8Text1, static_cast<int>(std::strlen(utf8Text1)));

    // Second part of string ("2025") Separately
    juce::String footerText2 = " 2025";

    // New Website Label
    juce::String websiteText = "alexfortunatomusic.com   ";

    // Adjust Footer Positioning
    int windowWidth = getWidth();
    int windowHeight = getHeight();

    int footerY = windowHeight - 35; // Move copyright text UP slightly

    // Copyright text (aligned right)
    juce::Rectangle<int> textBounds1(windowWidth - 200, footerY, 150, 15);
    g.drawText(footerText1, textBounds1, juce::Justification::right);

    // Year text ("2025" placed next to copyright text)
    juce::Rectangle<int> textBounds2(windowWidth - 50, footerY, 40, 15);
    g.drawText(footerText2, textBounds2, juce::Justification::left);

    // New Website text (below copyright)
    juce::Rectangle<int> websiteBounds(windowWidth - 200, footerY + 18, 190, 15);
    g.drawText(websiteText, websiteBounds, juce::Justification::right);

}

//==============================================================================
void TransportSenderV1AudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &playButton)
    {
        static bool isPlaying = false;
        isPlaying = !isPlaying;

        if (isPlaying)
            playButton.setButtonText(juce::CharPointer_UTF8("▶"));
        else
            playButton.setButtonText(juce::CharPointer_UTF8("▶"));

        playButton.repaint(); // Ensure the UI updates
    }
}


void TransportSenderV1AudioProcessorEditor::togglePlayState()
{
    

    // Toggle play state
    bool isPlaying = audioProcessor.getTransportState().isPlaying; // Check Current State
    audioProcessor.setPlayingState(!isPlaying); // Toggle it
    
    repaint(); // Refresh UI to update play button color
}

//==============================================================================Editor's Constructor:

TransportSenderV1AudioProcessorEditor::TransportSenderV1AudioProcessorEditor(TransportSenderV1AudioProcessor& p)
    : juce::AudioProcessorEditor(&p), audioProcessor(p), processorRef(p)


{

    playButton.addListener(this);
    addAndMakeVisible(playButton);
    playButton.addAndMakeVisible(isPlayingLabel); // Add label inside button
    playButton.onClick = [this] { togglePlayState(); };
    // addAndMakeVisible(transportDisplay);
    addAndMakeVisible(isPlayingLabel);
    addAndMakeVisible(bpmLabel);
    // Initialize BPM text label
    addAndMakeVisible(bpmTextLabel);
    bpmTextLabel.setText("BPM", juce::dontSendNotification);
    bpmTextLabel.setJustificationType(juce::Justification::centred);
    bpmTextLabel.setFont(juce::Font(14.0f, juce::Font::plain)); // Remove bold
    bpmTextLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    addAndMakeVisible(positionLabel);
    
    // Initialize and style OSC Status Label - chatgpt
        addAndMakeVisible(oscStatusLabel);
        oscStatusLabel.setText("OSC Status: Disconnected", juce::dontSendNotification);
        oscStatusLabel.setJustificationType(juce::Justification::centredLeft);
        oscStatusLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    
    // Text label above position label
    addAndMakeVisible(positionTextLabel);
    positionTextLabel.setText("Position", juce::dontSendNotification);
    positionTextLabel.setJustificationType(juce::Justification::centred);
    positionTextLabel.setFont(juce::Font(14.0f, juce::Font::plain));
    positionTextLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    
    // Initialize plugin title label
    addAndMakeVisible(pluginTitle);
    pluginTitle.setText("ReWire Solution VST3 - (Host) - Prototype V1", juce::dontSendNotification);
    pluginTitle.setJustificationType(juce::Justification::left);
    pluginTitle.setFont(juce::Font(20.0f, juce::Font::plain));
    pluginTitle.setColour(juce::Label::textColourId, juce::Colours::goldenrod);
    
    



    setSize(500, 300);
    setResizable(true, true); // Allow resizing
   // getConstrainer()->setFixedAspectRatio(2.0); // Enforce an aspect ratio (e.g., width:height = 2:1)
    startTimer(100); // Updates labels every 100ms
    
    //Ableton Data label (STAGNANT)
    abletonDataLabel.setText("Ableton Data:", juce::dontSendNotification);
    abletonDataLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    abletonDataLabel.setJustificationType(juce::Justification::centredLeft);
    abletonDataLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
        
    addAndMakeVisible(abletonDataLabel);
    
    
    tempoLabel.setText("Tempo: ---", juce::dontSendNotification);
    locationLabel.setText("Location: ---", juce::dontSendNotification);
    playStateLabel.setText("Play State: ---", juce::dontSendNotification);
    
    // Styling & adding to the UI
     for (auto* label : { &tempoLabel, &locationLabel, &playStateLabel })
     {
         label->setFont(juce::Font(15.0f, juce::Font::bold));
         label->setJustificationType(juce::Justification::left);
         label->setColour(juce::Label::textColourId, juce::Colours::white);
         addAndMakeVisible(label);
     }
    
    updateTransportLabels();
}





TransportSenderV1AudioProcessorEditor::~TransportSenderV1AudioProcessorEditor()
{
    playButton.setLookAndFeel(nullptr); // Remove LookAndFeel before destruction
    stopTimer();
    
    
}



void TransportSenderV1AudioProcessorEditor::resized()
{
    // Position UI labels in bottom-left
    int margin = 10;
    int labelHeight = 20;

    // ✅ New "Ableton Data:" label positioned above tempo, location, and play state labels
    abletonDataLabel.setBounds(margin, getHeight() - 90, getWidth() - margin * 2, labelHeight);

    tempoLabel.setBounds(margin, getHeight() - 70, getWidth() - margin * 2, labelHeight);
    locationLabel.setBounds(margin, getHeight() - 50, getWidth() - margin * 2, labelHeight);
    playStateLabel.setBounds(margin, getHeight() - 30, getWidth() - margin * 2, labelHeight);

    int windowWidth = getWidth();
    int windowHeight = getHeight();

    int boxWidth = windowWidth / 8;   // Width of each label box
    int boxHeight = 40;               // Fixed height
    int padding = 10;                 // Space between labels

    // Position play button (directly left of position box)
    playButton.setBounds(
        windowWidth / 2 - (boxWidth + padding / 2) - 2 * (boxWidth + padding),
        windowHeight / 4,
        boxWidth - 1,
        boxHeight - 1
    );


    
    
    

    pluginTitle.setBounds(10, 10, getWidth() - 20, 30);


    
    

    
    // **Move isPlayingLabel ABOVE the play button**
    isPlayingLabel.setBounds(
        playButton.getX(),         // Align with play button's left edge
        playButton.getY() - 25,    // Move it 25 pixels above the play button
        playButton.getWidth(),     // Keep the same width as the button
        20                         // Set label height
    );
    
    
    // Position the Position box and label (left of BPM)
    positionBox.setBounds(
        windowWidth / 2 - (boxWidth + padding / 2) - boxWidth - padding, // Shift left to place it before BPM
        windowHeight / 4,
        boxWidth,
        boxHeight
    );
    positionLabel.setBounds(positionBox); // Position inside position box
    
    // Position locationLabel (this is just a text label) directly above the position box
    positionTextLabel.setBounds(
        positionBox.getX(),           // Align with positionBox
        positionBox.getY() - 25,      // Move it 25 pixels above
        positionBox.getWidth(),       // Match positionBox width
        20                            // Label height
    );

    // Position the BPM box and label (right of Position)
    bpmBox.setBounds(
        positionBox.getRight() + padding,  // Place it to the right of Position box
        positionBox.getY(),
        boxWidth,
        boxHeight
    );
    bpmLabel.setBounds(bpmBox); // Position BPM label inside BPM box
    
    // Position bpmTextLabel directly above the BPM box
    bpmTextLabel.setBounds(
        bpmBox.getX(),           // Align with bpmBox
        bpmBox.getY() - 25,      // Move it 25 pixels above
        bpmBox.getWidth(),       // Match bpmBox width
        20                       // Label height
    );


    // Other UI elements
    auto area = getLocalBounds().reduced(10);
   // isPlayingLabel.setBounds(area.removeFromTop(20)); // This is handled by an earlier function
    // Position oscStatusLabel below the BPM and Position labels
    oscStatusLabel.setBounds(
        playButton.getX(),                 // Align left with the play button
        positionBox.getBottom() + 10,      // Position it just below the bottom of the labels
        bpmBox.getRight() - playButton.getX(), // Stretch to the right end of the last box
        20                                 // Set height
    );
 

    repaint();
}

// Function to update labels based on received OSC messages
void TransportSenderV1AudioProcessorEditor::updateTransportLabels()
{
    juce::MessageManager::callAsync([this]
    {
        const auto& state = processorRef.getSlaveTransportState();

        tempoLabel.setText("Tempo: " + juce::String(state.bpm), juce::dontSendNotification);

        // ✅ Ensure position updates properly
        juce::String locationText = "Location: " + juce::String(state.bar) + " | "
                                    + juce::String(state.beat) + " | "
                                    + juce::String(state.subBeat);
        locationLabel.setText(locationText, juce::dontSendNotification);
        locationLabel.repaint(); // ✅ Force UI refresh

        juce::String playText = state.isPlaying ? "Playing" : "Stopped";
        playStateLabel.setText("Play State: " + playText, juce::dontSendNotification);
    });
}








void TransportSenderV1AudioProcessorEditor::timerCallback()
{
    updateLabels();
    // updateOscMessageLabel(); // Incoming messages
}

void TransportSenderV1AudioProcessorEditor::updateLabels()
{
    auto state = audioProcessor.getTransportState();
    
    //  set playButton text to "▶"
    playButton.setButtonText(juce::CharPointer_UTF8("▶"));
    // Increase Font Size
    //playButton.setFont(juce::Font(30.0f, juce::Font::bold)); // Adjust size as needed
    playButton.setLookAndFeel(&customLookAndFeel);
    
    // Change playButton text color instead of button color
    playButton.setColour(juce::TextButton::textColourOffId, state.isPlaying ? juce::Colours::chartreuse : juce::Colours::red);
    // Change playButton Background Color
    playButton.setColour(juce::TextButton::buttonColourId, state.isPlaying ? juce::Colours::darkgrey : juce::Colours::black);

    
    // Update transport state labels
    isPlayingLabel.setText(state.isPlaying ? "Playing" : "Paused", juce::dontSendNotification);
    isPlayingLabel.setJustificationType(juce::Justification::centred);

    
    
    bpmLabel.setText(juce::String(state.bpm, 2), juce::dontSendNotification);
    bpmLabel.setJustificationType(juce::Justification::centred);

    
    int bar = static_cast<int>(state.ppqPosition / state.timeSigNumerator) + 1;
    int beat = static_cast<int>(state.ppqPosition) % state.timeSigNumerator + 1;
    positionLabel.setText(juce::String(bar) + " | " + juce::String(beat) + " | " +
                          juce::String((static_cast<int>(state.ppqPosition * 4) % 4) + 1),
                          juce::dontSendNotification);
    positionLabel.setJustificationType(juce::Justification::centred);
    

    
    
    
    


    // Update OSC Status with port
    if (audioProcessor.isOscConnected())
    {
        int port = audioProcessor.getOscPort();
        oscStatusLabel.setText("OSC Status: Connected to Port " + juce::String(port), juce::dontSendNotification);
        oscStatusLabel.setColour(juce::Label::textColourId, juce::Colours::chartreuse);
    }
    else
    {
        oscStatusLabel.setText("OSC Status: Disconnected", juce::dontSendNotification);
        oscStatusLabel.setColour(juce::Label::textColourId, juce::Colours::red);
    }
}





