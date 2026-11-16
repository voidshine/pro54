#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "Pro54Engine.h"
#include "Pro54Presets.h"

#include <array>
#include <atomic>

//==============================================================================
// One of the synth's parameters. It holds its value as the engine wants it
// (see pro54::ParamInfo) rather than leaving that to be worked out from the
// host's 0 to 1 version, so that a value from a preset or from saved state
// reaches the engine exactly as it was given.
class Pro54Parameter final : public juce::RangedAudioParameter
{
public:
    explicit Pro54Parameter (const pro54::ParamInfo&);

    float get() const noexcept      { return value.load (std::memory_order_relaxed); }
    void set (float newValue);

    //==============================================================================
    float getValue() const override;
    void setValue (float newValue) override;
    float getDefaultValue() const override;
    int getNumSteps() const override;
    bool isDiscrete() const override;
    bool isBoolean() const override;
    juce::String getText (float normalisedValue, int maximumLength) const override;
    float getValueForText (const juce::String& text) const override;
    const juce::NormalisableRange<float>& getNormalisableRange() const override;

private:
    const pro54::ParamInfo& info;
    const juce::NormalisableRange<float> range;
    std::atomic<float> value;
};

//==============================================================================
// The plugin: a pro54::Engine with host parameters for its knobs and switches,
// the factory presets as its programs, and the Pro-53's MIDI controller
// assignments.
class Pro54AudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    Pro54AudioProcessor();
    ~Pro54AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

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
    // The programs are the factory presets. Choosing one sets the parameters
    // that it holds; nothing is ever stored back into one (the original's
    // "record" button isn't implemented), so to keep a sound of your own, save
    // it with the host - its state is the parameters' values.
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // For the editor:

    Pro54Parameter& getParam (pro54::Param param) const     { return *parameters[(size_t) param]; }

    // Does what setCurrentProgram() does, and lets the host know about it.
    void selectProgram (int index);

    // Gets the on-screen keyboard's notes into the engine, and lets it show
    // the ones that are being played over MIDI.
    juce::MidiKeyboardState keyboardState;

    // Where the wheels are (0 to 1), which MIDI can change without the
    // corresponding parameters knowing anything about it.
    float getPitchWheelPosition() const     { return pitchWheelPosition.load (std::memory_order_relaxed); }
    float getModWheelPosition() const       { return modWheelPosition.load (std::memory_order_relaxed); }

    // Goes up whenever some MIDI arrives.
    juce::uint32 getMidiActivityCount() const   { return midiActivityCount.load (std::memory_order_relaxed); }

    // The size that the editor was last given, so that it can reopen the same.
    int editorWidth = 1143;

private:
    //==============================================================================
    void updateEngineParameters();
    void handleMidiMessage (const juce::MidiMessage&);

    pro54::Engine engine;

    std::array<Pro54Parameter*, pro54::numParams> parameters {};
    std::array<Pro54Parameter*, 128> controllerParameters {}; // indexed by MIDI CC number

    std::atomic<int> currentProgram { 0 };
    int bankSelect = 0;

    std::atomic<float> pitchWheelPosition { 0.5f }, modWheelPosition { 0.0f };
    std::atomic<juce::uint32> midiActivityCount { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pro54AudioProcessor)
};
