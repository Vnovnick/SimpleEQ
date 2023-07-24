/*
  ==============================================================================

	This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class SimpleEQAudioProcessor : public juce::AudioProcessor
#if JucePlugin_Enable_ARA
	, public juce::AudioProcessorARAExtension
#endif
{
public:
	//==============================================================================
	SimpleEQAudioProcessor();
	~SimpleEQAudioProcessor() override;

	//==============================================================================
	void prepareToPlay(double sampleRate, int samplesPerBlock) override;
	void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

	void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
	void setCurrentProgram(int index) override;
	const juce::String getProgramName(int index) override;
	void changeProgramName(int index, const juce::String& newName) override;

	//==============================================================================
	void getStateInformation(juce::MemoryBlock& destData) override;
	void setStateInformation(const void* data, int sizeInBytes) override;

	static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
	juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Parameters", createParameterLayout() };

private:
	// alias - filter type has response of 12db per octave 
	using Filter = juce::dsp::IIR::Filter<float>;

	// https://docs.juce.com/master/classdsp_1_1ProcessorChain.html 
	using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

	// will represent the whole mono input path - (Mono Chain: Lowcut -> Parametric -> HighCut)
	// vid reference link: https://youtu.be/i_Iq4_Kd7Rc?t=2088
	using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

	// need both left and right chains for Stereo (each chain representing different channels)
	MonoChain leftChain, rightChain;

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleEQAudioProcessor)
};
