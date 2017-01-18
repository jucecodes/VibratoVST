/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
VibratoAudioProcessor::VibratoAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    addParameter (rate = new AudioParameterFloat ("rate", // parameterID,
                                                  "Rate (Hz)", // parameter name
                                                  0.0f,   // minimum value
                                                  50.0f,   // maximum value
                                                  1.0f));    // default value
    addParameter (depth = new AudioParameterFloat ("depth","Depth",0.0f,50.0f,0.5f));
    addParameter (mix = new AudioParameterFloat ("mix","Dry/Wet",0.0f,1.0f,0.5f));
    for (int i = 0; i<2; i++){
        for (int j = 0; j<192001; j++){
            vBuffer[i][j] = 0.0f;
        }
    }
}

VibratoAudioProcessor::~VibratoAudioProcessor()
{
}

//==============================================================================
const String VibratoAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VibratoAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VibratoAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double VibratoAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VibratoAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VibratoAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VibratoAudioProcessor::setCurrentProgram (int index)
{
}

const String VibratoAudioProcessor::getProgramName (int index)
{
    return String();
}

void VibratoAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void VibratoAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void VibratoAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VibratoAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void VibratoAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    
    int numSamples = buffer.getNumSamples();
    float depthCopy = *depth;
    float delay = *depth;
    float rateCopy = *rate;
    float wet = *mix;
    float dry = 1-wet;
    float M = 0;
    if (getSampleRate() > 0) {
        M = rateCopy/getSampleRate();
    } else {
        M = rateCopy/44100.0f;
    }
    
    //AudioSampleBuffer bufferCopy = buffer;
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
        
         
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {               
            int w = writeIndex[channel];
            float* channelData = buffer.getWritePointer (channel); 
            for (int i = 0; i < numSamples; i++)
            {
                vBuffer[channel][w] = channelData[i];
                float modfreq = std::sin(2*M_PI*M*w);
                int tap = int(1+delay+depthCopy*modfreq);
                int rindex = int(w - tap);
                if (rindex < 0)
                    rindex = rindex + 192001;
                    
                channelData[i] = channelData[i]*dry + wet*vBuffer[channel][rindex];
                
                w++;
                if (w > 192000)
                    w =  1;
            
            }
            writeIndex[channel] = w;
    }
    
}

//==============================================================================
bool VibratoAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* VibratoAudioProcessor::createEditor()
{
    return new VibratoAudioProcessorEditor (*this);
}

//==============================================================================
void VibratoAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    MemoryOutputStream (destData, true).writeFloat(*rate);
    MemoryOutputStream (destData, true).writeFloat(*depth);
    MemoryOutputStream (destData, true).writeFloat(*mix);
}

void VibratoAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    *rate = MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false).readFloat();
    *depth = MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false).readFloat();
    *mix = MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false).readFloat();
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VibratoAudioProcessor();
}
