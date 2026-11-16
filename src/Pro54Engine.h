#pragma once

#include <memory>

//==============================================================================
// The Pro-54 synthesiser engine: a C++ port of the DSP in the Cmajor "Pro54"
// example patch (examples/patches/Pro54/Pro54.cmajor in the cmajor-lang/cmajor
// repository), itself a port of Native Instruments' Pro-53. See README.md for
// the handful of places where this deliberately differs from the Cmajor code.
//
// This file and Pro54Engine.cc have no dependency on JUCE (or anything else
// beyond the standard library), so the engine can be dropped behind any
// plugin API or driven directly from a test program.
//
// This is a port of code from the Cmajor project (https://cmajor.dev), which is
// (C) Cmajor Software Ltd and used here under the terms of the GPLv3, so the
// same terms apply to this file. See README.md.
namespace pro54
{

//==============================================================================
// The synth's parameters, named and ordered as the input endpoints of the
// Cmajor patch are.
enum Param
{
    PolyModFilterEnv, PolyModOscB, PolyModFreqA, PolyModPWA, PolyModFilt,
    OscAFreq, OscASaw, OscAPulse, OscAPW, OscASync,
    MixerOscALevel, MixerOscBLevel, MixerNoiseLevel, ExternalInputLevel,
    FilterCutoff, FilterResonance, FilterEnvAmt, FilterKeyboardTracking,
    FilterAttack, FilterDecay, FilterSustain, FilterRelease,
    DelayTime, DelaySpread, DelayDepth, DelayRate, DelayFeedback, DelayHiCut, DelayLoCut,
    DelayINV, DelayON, DelayWet, DelaySync, DelayMidi,
    LfoMidiSync, LfoFrequency, LfoShapeSaw, LfoShapeTri, LfoShapePulse,
    OscBFreq, OscBFreqFine, OscBShapeSaw, OscBShapeTri, OscBShapePulse,
    OscBPWAmount, OscBSubOsc, OscBKKeyboardTracking,
    WheelModulationLfoNoise, WheelModulationFreqOscA, WheelModulationFreqOscB,
    WheelModulationPWA, WheelModulationPWB, WheelModulationFilter,
    Glide, Unison,
    AmplifierAttack, AmplifierDecay, AmplifierSustain, AmplifierRelease,
    Release, Velocity, Repeat, Drone, FilterHPF, FilterInvertEnv,
    Analog, MasterTune, Volume, ModWheel, PitchBend, FilterVersion, ActiveVoices, TestTone,

    numParams
};

struct ParamInfo
{
    enum Type
    {
        knob,           // 0 to 100
        toggle,         // 0 or 1
        filterVersion,  // 0 = "Old", 0.5 = "New", 1 = "New AA"
        voiceCount      // 1 to 32
    };

    const char* id;     // the name of the Cmajor endpoint
    const char* name;
    Type type;
    float defaultValue;
    int midiController; // the MIDI CC the Pro-53 gives this parameter, or -1

    float getMinValue() const   { return type == voiceCount ? 1.0f : 0.0f; }
    float getMaxValue() const   { return type == knob ? 100.0f : (type == voiceCount ? 32.0f : 1.0f); }
};

extern const ParamInfo paramInfo[numParams];

// The order in which a set of parameter changes is applied. It's the order
// that the Cmajor patch's presets list their values in, which puts the
// parameters that a preset holds first: the six on the end (the hold switch,
// master tune and volume, pitch bend, filter version and the A-440 tone) are
// left alone when a preset is loaded.
constexpr int numPresetParams = 67;
extern const Param paramOrder[numParams];

//==============================================================================
class Engine
{
public:
    Engine();
    ~Engine();

    // Resets all the state and gets ready to render at the given sample rate.
    // The current parameter values are kept, and applied to the fresh state.
    void prepare (double sampleRate);

    // Parameters. Values are in the ranges that ParamInfo describes.
    // setParameters() takes an array of numParams values and passes on the ones
    // that have changed, in the order the Cmajor patch applies a preset in
    // (a few of them aren't independent, so the order can matter).
    void setParameter (int param, float value);
    void setParameters (const float* values);
    float getParameter (int param) const;

    // MIDI, with arguments as they appear in the messages. Channels are ignored.
    // The controllers handled here are the mod wheel (1), sustain (64),
    // sostenuto (66), all-sound-off (120) and all-notes-off (123).
    void noteOn (int note, int velocity);
    void noteOff (int note);
    void setPitchWheel (int value14Bit);
    void setController (int controller, int value);

    // The tempo that the LFO and the delay follow when their "MIDI" sync
    // buttons are on.
    void setTempo (double bpm);

    // The wheel positions currently in effect (0 to 1; 0.5 is no pitch bend),
    // whether they came from MIDI or from the parameters.
    float getPitchBend() const;
    float getModWheel() const;

    // Renders the next block. externalIn is the optional mono signal that the
    // mixer's "ext in" knob feeds into the filter: it may be null, and may be
    // the same buffer as one of the outputs.
    void process (const float* externalIn, float* left, float* right, int numFrames);

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace pro54
