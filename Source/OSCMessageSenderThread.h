#pragma once

#include <queue>
#include <JuceHeader.h>

// Define the OSCTransportMessage struct here since it doesn't exist in a separate file.
struct OSCTransportMessage
{
    bool isPlaying;
    float tempo;
    float position;
};

/**
 * @class OSCMessageSenderThread
 * @brief A background thread that continuously checks a queue for transport data
 *        and sends corresponding OSC messages.
 */
class OSCMessageSenderThread : public juce::Thread
{
public:
    OSCMessageSenderThread(juce::OSCSender& sender,
                           std::queue<OSCTransportMessage>& messageQueue,
                           juce::CriticalSection& queueLock)
        : juce::Thread("OSC Message Sender Thread"),
          oscSender(sender),
          oscMessageQueue(messageQueue),
          oscQueueLock(queueLock)
    {
    }

    void run() override
    {
        while (!threadShouldExit())
        {
            OSCTransportMessage msg;
            bool messageAvailable = false;

            {
                const juce::ScopedLock lock(oscQueueLock);
                if (!oscMessageQueue.empty())
                {
                    msg = oscMessageQueue.front();
                    oscMessageQueue.pop();
                    messageAvailable = true;
                }
            }

            if (messageAvailable)
            {
                if (!oscSender.send("/play", msg.isPlaying ? 1 : 0))
                    DBG("Failed to send /play message");

                if (!oscSender.send("/tempo", msg.tempo))
                    DBG("Failed to send /tempo message");

                if (!oscSender.send("/position", msg.position))
                    DBG("Failed to send /position message");
            }
            else
            {
                wait(5);
            }
        }
    }

private:
    juce::OSCSender& oscSender;
    std::queue<OSCTransportMessage>& oscMessageQueue;
    juce::CriticalSection& oscQueueLock;
};
