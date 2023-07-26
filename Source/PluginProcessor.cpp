/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEQAudioProcessor::SimpleEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
		// #if checks for code and then will remove the code that doesn't match the condition on compile time instead of on run time
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	)
#endif
{
}

SimpleEQAudioProcessor::~SimpleEQAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleEQAudioProcessor::getName() const
{
	return JucePlugin_Name;
}

bool SimpleEQAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool SimpleEQAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool SimpleEQAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double SimpleEQAudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int SimpleEQAudioProcessor::getNumPrograms()
{
	return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
	// so this should be at least 1, even if you're not really implementing programs.
}

int SimpleEQAudioProcessor::getCurrentProgram()
{
	return 0;
}

void SimpleEQAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String SimpleEQAudioProcessor::getProgramName(int index)
{
	return {};
}

void SimpleEQAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleEQAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	// Use this method as the place to do any pre-playback
	// initialisation that you need..
	// https://docs.juce.com/master/structdsp_1_1ProcessSpec.html
	// timestamp: https://youtu.be/i_Iq4_Kd7Rc?t=2144
	juce::dsp::ProcessSpec spec;

	spec.maximumBlockSize = samplesPerBlock;
	spec.numChannels = 1;
	spec.sampleRate = sampleRate;
	leftChain.prepare(spec);
	rightChain.prepare(spec);
}

void SimpleEQAudioProcessor::releaseResources()
{
	// When playback stops, you can use this as an opportunity to free up any
	// spare memory, etc.
}
// checks if below is defined 
#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleEQAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused(layouts);
	return true;
#else
	// This is the place where you check if the layout is supported.
	// In this template code we only support mono or stereo.
	// Some plugin hosts, such as certain GarageBand versions, will only
	// load plugins that support stereo bus layouts.
	if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
		&& layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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
// AudioBuffer: https://docs.juce.com/master/classAudioBuffer.html (A multi-channel buffer containing floating point audio samples)
void SimpleEQAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
	auto totalNumInputChannels = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();

	// In case we have more outputs than inputs, this code clears any output
	// channels that didn't contain input data, (because these aren't
	// guaranteed to be empty - they may contain garbage).
	// This is here to avoid people getting screaming feedback
	// when they first compile a plugin, but obviously you don't need to keep
	// this code if your algorithm always overwrites all the output channels.
	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear(i, 0, buffer.getNumSamples());

	// Process chain requires processing context to be passed to it in order for the audio to run through the links in the chain.
	// To make a processing context, we need to suplly an AudioBlock instance.
	// processBlock function called by the host, and it is given a buffer that can have any number of channels.
	// so we need to extract the left and right channels from the buffer (typically 0 and 1 respectively)
	// youtube timestamp: https://youtu.be/i_Iq4_Kd7Rc?t=2183

	// AudioBlock<float> - Minimal and lightweight data-structure which contains a list of pointers to channels containing some kind of sample data.
	// https://docs.juce.com/master/classdsp_1_1AudioBlock.html
	juce::dsp::AudioBlock<float> block(buffer);
	auto leftBlock = block.getSingleChannelBlock(0);
	auto rightBlock = block.getSingleChannelBlock(1);


	juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
	juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

	leftChain.process(leftContext);
	rightChain.process(rightContext);



}

//==============================================================================
bool SimpleEQAudioProcessor::hasEditor() const
{
	return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleEQAudioProcessor::createEditor()
{
	//return new SimpleEQAudioProcessorEditor(*this);
	return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SimpleEQAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	// You should use this method to store your parameters in the memory block.
	// You could do that either as raw data, or use the XML or ValueTree classes
	// as intermediaries to make it easy to save and load complex data.
}

void SimpleEQAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	// You should use this method to restore your parameters from this memory block,
	// whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout SimpleEQAudioProcessor::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;

	// low cut frequency 
	layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq", "LowCut Freq", juce::NormalisableRange<float>(20.f, 200000.f, 1.f, 1.f), 20.f));

	// high cut frequency 
	layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq", "HighCut Freq", juce::NormalisableRange<float>(20.f, 200000.f, 1.f, 1.f), 20000.f));

	// peak frequency 
	layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Freq", "Peak Freq", juce::NormalisableRange<float>(20.f, 200000.f, 1.f, 1.f), 750.f));

	// gain knob - in db
	layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Gain", "Peak Gain", juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.0f));

	// quality control - controls how wide or narrow the q is (think of the wide vs narrow control in pro tools eq that make widen or shrink the angle of the curve - i think)
	layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Quality", "Peak Quality", juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f), 1.f));

	juce::StringArray cutOptionsArray;
	for (int i = 0; i < 4; ++i)
	{
		juce::String option;
		option << (12 + i * 12);
		option << " db/Oct";
		cutOptionsArray.add(option);
	}

	// low cut filter control
	layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", cutOptionsArray, 0));
	// high cut filter control
	layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope", cutOptionsArray, 0));



	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new SimpleEQAudioProcessor();

}
