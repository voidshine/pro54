// The Pro-54 synthesiser engine: see Pro54Engine.h.
//
// This is a port of code from the Cmajor project (https://cmajor.dev), which is
// (C) Cmajor Software Ltd and used here under the terms of the GPLv3, so the
// same terms apply to this file. See README.md.

#include "Pro54Engine.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iterator>

namespace pro54
{

//==============================================================================
const ParamInfo paramInfo[numParams] =
{
    { "PolyModFilterEnv",        "PolyMod Source Filt Env",  ParamInfo::knob,          37.7953f,  20 },
    { "PolyModOscB",             "PolyMod Source Osc B",     ParamInfo::knob,          50.3937f,  21 },
    { "PolyModFreqA",            "PolyMod Dest Freq A",      ParamInfo::toggle,        1.0f,      22 },
    { "PolyModPWA",              "PolyMod Dest PWidth A",    ParamInfo::toggle,        0.0f,      23 },
    { "PolyModFilt",             "PolyMod Dest Filter",      ParamInfo::toggle,        0.0f,      24 },
    { "OscAFreq",                "Oscillator A Frequency",   ParamInfo::knob,          63.7795f,  40 },
    { "OscASaw",                 "Oscillator A Sawtooth",    ParamInfo::toggle,        1.0f,      41 },
    { "OscAPulse",               "Oscillator A Pulse",       ParamInfo::toggle,        0.0f,      42 },
    { "OscAPW",                  "Oscillator A PulseWidth",  ParamInfo::knob,          19.685f,   43 },
    { "OscASync",                "Oscillator A Sync",        ParamInfo::toggle,        1.0f,      44 },
    { "MixerOscALevel",          "Mixer Oscillator A",       ParamInfo::knob,          61.4173f,  45 },
    { "MixerOscBLevel",          "Mixer Oscillator B",       ParamInfo::knob,          100.0f,    46 },
    { "MixerNoiseLevel",         "Mixer Noise",              ParamInfo::knob,          0.0f,      47 },
    { "ExternalInputLevel",      "Mixer External Input",     ParamInfo::knob,          0.0f,      48 },
    { "FilterCutoff",            "Filter Cutoff",            ParamInfo::knob,          29.9213f,  70 },
    { "FilterResonance",         "Filter Resonance",         ParamInfo::knob,          29.1339f,  71 },
    { "FilterEnvAmt",            "Filter Envelope Amount",   ParamInfo::knob,          37.0079f,  72 },
    { "FilterKeyboardTracking",  "Filter Keyboard Follow",   ParamInfo::knob,          100.0f,    73 },
    { "FilterAttack",            "Filter Attack",            ParamInfo::knob,          0.0f,      75 },
    { "FilterDecay",             "Filter Decay",             ParamInfo::knob,          68.5039f,  76 },
    { "FilterSustain",           "Filter Sustain",           ParamInfo::knob,          0.0f,      77 },
    { "FilterRelease",           "Filter Release",           ParamInfo::knob,          65.3543f,  78 },
    { "DelayTime",               "Delay Effect Time",        ParamInfo::knob,          68.5039f,  105 },
    { "DelaySpread",             "Delay Effect Spread",      ParamInfo::knob,          41.7323f,  106 },
    { "DelayDepth",              "Delay Effect Depth",       ParamInfo::knob,          0.0f,      107 },
    { "DelayRate",               "Delay Effect Rate",        ParamInfo::knob,          0.0f,      108 },
    { "DelayFeedback",           "Delay Effect Feedback",    ParamInfo::knob,          26.7717f,  110 },

    // The Cmajor patch has the IDs of these two the wrong way round: "DelayHiCut"
    // sets the high-pass filter in the delay's feedback path, and is the knob
    // labelled "lo cut" on the panel. The IDs are left as they are because the
    // presets use them, but the names say what the parameters actually do.
    { "DelayHiCut",              "Delay Effect Low Cut",     ParamInfo::knob,          30.7087f,  111 },
    { "DelayLoCut",              "Delay Effect High Cut",    ParamInfo::knob,          70.0787f,  112 },

    { "DelayINV",                "Delay Effect Invert",      ParamInfo::toggle,        0.0f,      113 },
    { "DelayON",                 "Delay Effect On",          ParamInfo::toggle,        1.0f,      115 },
    { "DelayWet",                "Delay Effect Wet",         ParamInfo::knob,          20.4724f,  116 },
    { "DelaySync",               "Delay Effect Sync",        ParamInfo::toggle,        0.0f,      117 },
    { "DelayMidi",               "Delay Effect MIDI Sync",   ParamInfo::toggle,        0.0f,      118 },
    { "LfoMidiSync",             "LFO MIDI Sync",            ParamInfo::toggle,        0.0f,      25 },
    { "LfoFrequency",            "LFO Frequency",            ParamInfo::knob,          73.2283f,  26 },
    { "LfoShapeSaw",             "LFO Sawtooth",             ParamInfo::toggle,        0.0f,      27 },
    { "LfoShapeTri",             "LFO Triangle",             ParamInfo::toggle,        0.0f,      28 },
    { "LfoShapePulse",           "LFO Pulse",                ParamInfo::toggle,        1.0f,      29 },
    { "OscBFreq",                "Oscillator B Frequency",   ParamInfo::knob,          25.1968f,  50 },
    { "OscBFreqFine",            "Oscillator B Freq Fine",   ParamInfo::knob,          0.0f,      51 },
    { "OscBShapeSaw",            "Oscillator B Sawtooth",    ParamInfo::toggle,        0.0f,      52 },
    { "OscBShapeTri",            "Oscillator B Triangle",    ParamInfo::toggle,        1.0f,      53 },
    { "OscBShapePulse",          "Oscillator B Pulse",       ParamInfo::toggle,        0.0f,      54 },
    { "OscBPWAmount",            "Oscillator B PulseWidth",  ParamInfo::knob,          11.0236f,  55 },
    { "OscBSubOsc",              "Oscillator B Low Freq",    ParamInfo::toggle,        0.0f,      56 },
    { "OscBKKeyboardTracking",   "Oscillator B Key Follow",  ParamInfo::toggle,        1.0f,      57 },
    { "WheelModulationLfoNoise", "WheelMod LFO-Noise Mix",   ParamInfo::knob,          0.0f,      34 },
    { "WheelModulationFreqOscA", "WheelMod Dest Freq A",     ParamInfo::toggle,        1.0f,      35 },
    { "WheelModulationFreqOscB", "WheelMod Dest Freq B",     ParamInfo::toggle,        0.0f,      36 },
    { "WheelModulationPWA",      "WheelMod Dest PWidth A",   ParamInfo::toggle,        0.0f,      37 },
    { "WheelModulationPWB",      "WheelMod Dest PWidth B",   ParamInfo::toggle,        0.0f,      38 },
    { "WheelModulationFilter",   "WheelMod Dest Filter",     ParamInfo::toggle,        0.0f,      39 },
    { "Glide",                   "Glide Time",               ParamInfo::knob,          0.0f,      5 },
    { "Unison",                  "Unisono Mode",             ParamInfo::toggle,        0.0f,      59 },
    { "AmplifierAttack",         "Amplifier Attack",         ParamInfo::knob,          0.0f,      80 },
    { "AmplifierDecay",          "Amplifier Decay",          ParamInfo::knob,          61.4173f,  81 },
    { "AmplifierSustain",        "Amplifier Sustain",        ParamInfo::knob,          71.6535f,  82 },
    { "AmplifierRelease",        "Amplifier Release",        ParamInfo::knob,          37.7953f,  83 },
    { "Release",                 "Release on/off",           ParamInfo::toggle,        1.0f,      85 },
    { "Velocity",                "Velocity on/off",          ParamInfo::toggle,        1.0f,      86 },
    { "Repeat",                  "LFO Envelope Trigger",     ParamInfo::toggle,        0.0f,      30 },
    { "Drone",                   "Amplifier Hold",           ParamInfo::toggle,        0.0f,      84 },
    { "FilterHPF",               "Filter HPF-Mode",          ParamInfo::toggle,        0.0f,      74 },
    { "FilterInvertEnv",         "Filter Envelope Invert",   ParamInfo::toggle,        0.0f,      79 },
    { "Analog",                  "Analog",                   ParamInfo::knob,          33.0709f,  87 },
    { "MasterTune",              "Master Tune",              ParamInfo::knob,          50.0f,     88 },
    { "Volume",                  "Master Volume",            ParamInfo::knob,          100.0f,    7 },
    { "ModWheel",                "Modulation Wheel",         ParamInfo::knob,          0.0f,      -1 },
    { "PitchBend",               "Pitch Bend",               ParamInfo::knob,          50.0f,     -1 },
    { "FilterVersion",           "Filter Version",           ParamInfo::filterVersion, 0.0f,      -1 },
    { "ActiveVoices",            "Active Voices",            ParamInfo::voiceCount,    5.0f,      -1 },
    { "TestTone",                "Test Tone",                ParamInfo::toggle,        0.0f,      89 }
};

const Param paramOrder[numParams] =
{
    ActiveVoices,
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
    Release, Velocity,
    Analog, ModWheel, FilterHPF, FilterInvertEnv, Repeat,

    Drone, MasterTune, Volume, PitchBend, FilterVersion, TestTone
};

//==============================================================================
namespace
{
    constexpr int   numVoices         = 32;       // the most that "active voices" can be set to
    constexpr int   controlPeriod     = 1;        // frames between runs of the control-rate code
    constexpr int   delayMask         = 0x1ffff;
    constexpr int   delayLength       = delayMask + 1;
    constexpr float envelopeAOffset   = -0.02f;   // the amp envelope aims below zero, to be sure of getting there
    constexpr float A440Pulse         = 1.66f;
    constexpr float inputVolume       = 25.0f;
    constexpr float masterVolume      = 0.224f;
    constexpr int   noteOffMarker     = -300;
    constexpr int   knobSmoothingTime = 20;

    // The "analog" knob gives every voice its own slight detuning, using numbers
    // from a 32-bit shift register. The Cmajor code keeps that register in a
    // signed int and steps it with ">>", which in Cmajor is an arithmetic shift:
    // once the top bit gets set it stays set, and within a few voices every
    // number comes out as +1 - so from about the sixth voice on, the voices all
    // get the same offsets rather than ones of their own. The shift is treated
    // as unsigned here, which is what the same generator does for the noise
    // source, and clearly what was meant. Set this to true for results that
    // match the Cmajor patch exactly.
    constexpr bool cmajorCompatibleAnalogRandomness = false;

    // Voice indexes are a wrap<32> in the Cmajor code: this does the same job
    // wherever one is made from a number that could, in principle, be out of range.
    constexpr int wrapVoice (int v)             { return v & (numVoices - 1); }

    //==============================================================================
    // These behave exactly as the Cmajor intrinsics of the same names do.
    inline float min (float a, float b)         { return a < b ? a : b; }
    inline float max (float a, float b)         { return a > b ? a : b; }
    inline float clamp (float v, float lo, float hi)    { return v > hi ? hi : (v < lo ? lo : v); }

    inline float sign (float v)                 { return v >= 0 ? 1.0f : -1.0f; }
    inline float undenormalise (float f)        { return -1e-12 < f && f < 1e-12 ? 0.0f : f; }

    // Brings a phase that has had something subtracted from it back into -1 to 1
    inline float getPhase (float ph)
    {
        if (ph >= 1.0f)   return ph - 2.0f;
        if (ph <= -1.0f)  return ph + 2.0f;
        return ph;
    }

    // 0..1.50845 range only!
    inline float fasttanf (float x)
    {
        const float a = ((-0.0896638f * x + 0.0388452f) * x + 1.00005f) * x;
        const float b = (-0.430871f * x + 0.0404318f) * x + 1.0f;
        return a / b;
    }

    inline float fasttanhf (float x)
    {
        if (std::abs (x) > 4.9717869f)
            return x > 0 ? 1.0f : -1.0f;

        const float x2 = x * x;
        const float a = (((x2 + 378) * x2 + 17325) * x2 + 135135) * x;
        const float b = ((28 * x2 + 3150) * x2 + 62370) * x2 + 135135;
        return a / b;
    }

    // An anti-aliased tanh: the average of tanh between the previous input and
    // this one, found from its integral, log (cosh (x)).
    inline float tanhaa (float x, float& lastX, float& lastF0)
    {
        const float x2 = lastX;
        lastX = x;
        const float dx = x - x2;

        if (std::abs (dx) < 0.001)
        {
            x = std::tanh ((x + x2) * 0.5f);
            lastF0 = -2.0f;
        }
        else
        {
            if (lastF0 < -1.0f)
                lastF0 = std::abs (x2) < 12 ? std::log (std::cosh (x2)) : x2 - 0.69314718f;

            const float F0 = std::abs (x) < 12 ? std::log (std::cosh (x)) : x - 0.69314718f;
            x = (F0 - lastF0) / dx;
            lastF0 = F0;
        }

        return x;
    }

    //==============================================================================
    // The oscillators are anti-aliased by smoothing each step in their waveforms
    // with a correction that's spread over two samples: the first part goes on
    // the sample in which the step falls, and the second is carried over to the
    // next one. x says where in the sample the step is: how far the phase had
    // got past it, as a fraction of the phase increment.
    inline float stepCorrection1 (float x)      { return 2.0f - x * x; }
    inline float stepCorrection2 (float x)      { const float x1 = x - 1.0f; return x1 * x1; }

    // An oscillator's phase running off the top and wrapping round, which is a step down.
    struct PhaseWrap
    {
        float reciprocalF;              // 1 / the phase increment
        float correction1, correction2;
    };

    // Oscillator A being reset to the start of its cycle by oscillator B wrapping.
    struct SyncReset
    {
        float distance;                 // how far A gets in what's left of the sample after the reset
        float phaseChange;              // the jump that the reset makes in its phase
        float correction1, correction2;
    };

    // Returns a pulse wave's value at the given phase, with the corrections for
    // its edges. A pulse is the sign of the distance between the phase and the
    // pulse width, so it steps up where the phase passes the width, and steps
    // down along with the phase when that wraps.
    inline float renderPulse (float phase, float pulseWidth, float f, const PhaseWrap* wrap, float& carry)
    {
        const float ph2 = phase - pulseWidth;
        const float ph3 = getPhase (ph2);
        float y = sign (ph2) + carry;

        if (wrap != nullptr)
            y += wrap->correction1;

        if (ph3 * (ph3 - f) < 0.0f) // the step up is in this sample
        {
            const float x3 = wrap != nullptr ? std::abs (ph3) * wrap->reciprocalF
                                             : std::abs (ph3) / f;
            y -= stepCorrection1 (x3);
            carry = wrap != nullptr ? wrap->correction2 - stepCorrection2 (x3)
                                    : -stepCorrection2 (x3);
        }
        else
        {
            carry = wrap != nullptr ? wrap->correction2 : 0.0f;
        }

        return y;
    }

    // The same, for a sample in which the oscillator gets reset by a sync. That
    // can move the phase to the other side of the pulse width (another step), and
    // a step up that was due later in the sample may now come sooner, or not at all.
    inline float renderSyncedPulse (float phaseBeforeSync, float phaseAfterSync, float pulseWidth, float f,
                                    const PhaseWrap* wrap, const SyncReset& sync, float& carry)
    {
        const float ph2 = phaseBeforeSync - pulseWidth;
        const float ph3 = getPhase (ph2);
        const float ph4 = ph2 + sync.phaseChange;

        float upAfterSync1 = 0.0f, upAfterSync2 = 0.0f;

        if (ph4 * (ph4 + sync.phaseChange - f) < 0.0f)
        {
            const float x4 = wrap != nullptr ? std::abs (ph4) * wrap->reciprocalF
                                             : std::abs (ph4) / f;
            upAfterSync1 = stepCorrection1 (x4);
            upAfterSync2 = stepCorrection2 (x4);
        }

        float syncStep1 = 0.0f, syncStep2 = 0.0f;

        if ((ph2 - sync.distance) > 0.0f)
        {
            syncStep1 = sync.correction1;
            syncStep2 = sync.correction2;
        }

        float y = sign (phaseAfterSync - pulseWidth) + carry;

        if (wrap != nullptr)
            y += wrap->correction1;

        if ((ph3 - sync.distance) * (ph3 - f) < 0.0f)
        {
            const float x3 = wrap != nullptr ? std::abs (ph3) * wrap->reciprocalF
                                             : std::abs (ph3) / f;
            y -= stepCorrection1 (x3);
            carry = wrap != nullptr ? wrap->correction2 - stepCorrection2 (x3)
                                    : -stepCorrection2 (x3);
            carry -= upAfterSync2;
        }
        else
        {
            carry = wrap != nullptr ? wrap->correction2 - upAfterSync2
                                    : -upAfterSync2;
        }

        carry += syncStep2;
        return y - upAfterSync1 + syncStep1;
    }
}

//==============================================================================
struct Engine::Impl
{
    Impl()
    {
        for (int i = 0; i < numParams; ++i)
            parameters[i] = paramInfo[i].defaultValue;

        initExponentLUT();
        prepare (44100.0);
    }

    //==============================================================================
    // The state. Its layout and names follow the Cmajor source, itself "a fairly
    // direct port" of the original, to keep the two easy to compare.

    struct MonoSignals
    {
        float controlCount;
        int   knobSmoothingCounter;
        float glide;
        float glideFinal;
        float glideOut;
        float glideInc;

        float lfoPhase;
        float lfoFreq;
        float lfoSawA;
        float lfoTriangleA;
        float lfoPulseA;
        bool  lfoPulseSign;
        float lfoShA;
        float lfoSh;
        float lfoConst;

        uint32_t noiseMem;
        float noiseFilterFreq;
        float noiseFilterLPF;

        float lfoNoiseMix;
        float modulationWheel;

        float envelopeA_D_FAC;
        float envelopeA_R_FAC;
        float envelopeA_Rel;
        float envelopeA_Att;
        float envelopeA_Sus;

        float envelopeF_D_FAC;
        float envelopeF_R_FAC;
        float envelopeF_Rel;
        float envelopeF_Att;
        float envelopeF_Sus;

        float oscillatorB_PulseA;
        float oscillatorB_SawA;
        bool  oscillatorB_TriA;
        float oscillatorB_FM_WM;
        float oscillatorB_PW;
        float oscillatorB_PW_Inc;
        float oscillatorB_PW_WM;
        float oscillatorB_LOF;
        float oscillatorB_Keyboard;
        float oscillatorB_Freq;
        float oscillatorB_Pitch;
        float oscillatorB_Fine;

        float polymodOscillatorB;
        float polymodEnvelopeF;

        float oscillatorA_SawA;
        float oscillatorA_FM_WM;
        float oscillatorA_FM_PM;
        float oscillatorA_PW;
        float oscillatorA_PW_Inc;
        float oscillatorA_PW_WM;
        float oscillatorA_PW_PM;
        bool  oscillatorA_PulseActive;
        float oscillatorA_SawActive;
        float oscillatorA_Freq;
        bool  oscillatorSyncActive;

        float mixerOscillatorA;
        float mixerOscillatorA_Inc;
        float mixerOscillatorB;
        float mixerOscillatorB_Inc;
        float mixerNoise;
        float mixerExternal;

        float filter_WM;
        float filter_PM;
        float filter_Env;
        float filter_EnvInc;
        bool  filter_HPF;
        float filter_D;
        float filter_FMAX;
        float filter_Keyboard;
        float filter_KeyboardInc;
        float filter_Cutoff;
        float filter_Event;
        float filter_EventInc;
        float filter_Res;
        float filterN_D;
        float filterN_FMAX;
        float filter1HPF;
        float filter_K;

        int   smootherCount;
        float volumeSmooth;
        float volumeInc;
        bool  delayActive;
        float delayDrySmooth;
        float delayDryInc;
        float delayWetSmooth;
        float delayWetInc;
        float delayFeedbackSmooth;
        float delayFeedbackInc;
        float delay_HPF_F;
        float delay_HPF_LPF;
        float delay_LPF_F;
        float delay_LPF_LPF;
        float delayTime_Mod[4];
        float delayTime_Inc[4];
        int   delayBufferIn;

        float A440Freq;
        float A440Phase;
        bool  A440PulseSign;
        float A440FilterFreq;
        float A440FilterLPF;

        float filterEnvelopeFinal;
        float filterKeyboardFinal;
        float oscillatorA_PW_Final;
        float oscillatorB_PW_Final;
        float mixerOscillatorA_Final;
        float mixerOscillatorB_Final;

        float volumeFinal;
        float delayWetKnob;
        float delayWetFinal;
        float delayDryFinal;
        float delayFeedbackKnob;
        float delayFeedbackFinal;
        float delayInv;
        float delaySum;
        float delayTime;
        float delayTimeRatio;
        float delayTimes[4];
        float delayRate;
        float delayRates[4];
        float delayDepthKnob;
        float delayDepth;
        float delayLFO_Pos[4];
        float delaySpread;
        bool  delayMIDI;
        bool  delaySync;

        float lfoRatio;
        bool  midiTempo;
        float pitchBend;
        float masterTune;
        float tune;
        float velocityAmp;
        float velocityFilter;
        float filterEnvelopeInv;
        float envelopeRelease;
        bool  unisonActive;
        float unisonNote;
        bool  repeatActive;
        bool  droneActive;
        bool  unisonGate;
    };

    struct VoiceState
    {
        bool isActive;

        float gate;
        float noteNumber;
        float pitch;
        float unisonRnd;
        float oscillatorA_Rnd;
        float oscillatorA_Tune;
        float oscillatorB_Rnd;
        float oscillatorB_Tune;

        float envelopeA_Out;
        float envelopeA_Level;
        float envelopeA_Peak;
        float envelopeA_Inc;
        float envelopeA_SDif;
        float envelopeA_Fac;
        int   envelopeA_Stage;

        float envelopeF_Out;
        float envelopeF_Level;
        float envelopeF_Peak;
        float envelopeF_Inc;
        float envelopeF_SDif;
        float envelopeF_Fac;
        int   envelopeF_Stage;

        float oscillatorB_F;
        float oscillatorB_Phase;
        float oscillatorB_Saw_AA2;
        float oscillatorB_Pulse_AA2;

        float polymodEnvRnd;

        float oscillatorA_F;
        float oscillatorA_Phase;
        float oscillatorA_Saw_AA2;
        float oscillatorA_Pulse_AA2;
        float eqLast;
        float filter_Event;
        float filter_BPF;
        float filter_LPF;
        float filter_F_Inc;
        float filter_F_Smooth;
        float filter_RND;
        float filterN_BPF;
        float filterN_LPF;
        float saturator_Out;
        float filter1HP_LPF;

        float filter_S0;
        float filter_S1;
        float filter_S2;
        float filter_S3;
        float filter_X;
        float filter_X_1;
        float filter_X_2;
        float filter_X_3;
        float filter_X_4;
        float filter_F0_4;
        float filter_F0_1;
        float filter_F0_2;
        float filter_F0_3;
    };

    // The envelope stages
    enum { attack = 0, decay = 1, release = 2 };

    float parameters[numParams];
    double tempo = 120.0;

    double frequency = 0;
    float tau = 0, tauFilter = 0, piOverSampleRate = 0;

    MonoSignals state;
    VoiceState voices[numVoices];
    float delayBuffer[delayLength];
    bool testToneActive = false;
    bool sustainActive = false;
    int filterVersion = 0;
    int youngestOffVoice = -1;
    int oldestVoice = -1;
    int activeVoices = 5;

    // The voice allocator's records: the note that each voice is playing (or,
    // counting down from noteOffMarker, the one it last played), and a ring
    // which links the voices in the order they'll be reused.
    int collVoiceNext[numVoices], collVoicePrev[numVoices], collVoiceNote[numVoices];
    bool collVoiceSustain[numVoices], collVoiceHold[numVoices];

    float exponentLUT[2401];
    float exponentLUTSlope[2400];

    //==============================================================================
    void prepare (double sampleRate)
    {
        frequency = sampleRate;
        tau = float (1.0 / frequency);
        tauFilter = float (6.283 / frequency);
        piOverSampleRate = float (3.14159265358979323846 / frequency);

        state = {};
        std::fill (std::begin (voices), std::end (voices), VoiceState());
        std::fill (std::begin (delayBuffer), std::end (delayBuffer), 0.0f);
        testToneActive = false;
        sustainActive = false;
        filterVersion = 0;
        activeVoices = 5;

        state.A440FilterFreq = pitchToFreq (55.0f) * tauFilter;
        state.noiseFilterFreq = pitchToFreq (50.0f) * tauFilter;
        state.filter1HPF = pitchToFreq (-40.0f) * tauFilter;
        state.A440Freq = 440.0f * tau;

        state.filterEnvelopeInv = 1.0f;
        state.glideOut = 36.0f;
        state.controlCount = 100.0f;
        state.noiseMem = 1;

        for (int i = 0; i < numVoices; ++i)
        {
            collVoiceSustain[i] = false;
            collVoiceHold[i] = false;
            collVoiceNote[i] = noteOffMarker;
        }

        allSoundOff();

        for (auto p : paramOrder)
            handleParameter (p, parameters[p]);
    }

    //==============================================================================
    void setParameter (int param, float value, bool onlyIfChanged)
    {
        if (param < 0 || param >= numParams || std::isnan (value))
            return;

        auto& info = paramInfo[param];

        if (info.type == ParamInfo::toggle)
            value = value >= 0.5f ? 1.0f : 0.0f;
        else if (info.type == ParamInfo::voiceCount)
            value = std::floor (value + 0.5f);

        value = clamp (value, info.getMinValue(), info.getMaxValue());

        if (onlyIfChanged && value == parameters[param])
            return;

        parameters[param] = value;
        handleParameter (param, value);
    }

    // These are the event handlers of the Cmajor processor.
    void handleParameter (int param, float value)
    {
        const float amount = value * 0.01f; // a knob's position, from 0 to 1
        const bool on = value != 0;         // a switch's

        switch (param)
        {
            //==============================================================================
            case PolyModFilterEnv:
            {
                const float knob = 0.4f + 4.7f * amount; // 0.4 to 5.1
                state.polymodEnvelopeF = knob * knob - 0.16f;
                break;
            }

            case PolyModOscB:       state.polymodOscillatorB = 3.09f * amount; break;
            case PolyModFreqA:      state.oscillatorA_FM_PM = on ? 0.4f : 0.0f; break;
            case PolyModPWA:        state.oscillatorA_PW_PM = on ? -1.0f : 0.0f; break; // -1 because the oscillators' phase goes over 2 units
            case PolyModFilt:       state.filter_PM = on ? 3.8f : 0.0f; break;

            //==============================================================================
            case OscAFreq:
                // -24 to 24 step 1; add 12 because the phase goes over 2 units
                state.oscillatorA_Freq = float (-24 + int ((0.375f * 127.0f) * amount + 0.4f)) + 12;

                for (auto& v : voices)
                    tuneOscillatorA (v);

                break;

            case OscASaw:
                state.oscillatorA_SawActive = on ? 1.0f : 0.0f;
                state.oscillatorA_SawA = state.mixerOscillatorA * state.oscillatorA_SawActive;
                break;

            case OscAPulse:         state.oscillatorA_PulseActive = on; break;
            case OscAPW:            state.oscillatorA_PW_Final = amount * 2 - 1; smoothValue(); break;
            case OscASync:          state.oscillatorSyncActive = on; break;

            //==============================================================================
            case MixerOscALevel:    state.mixerOscillatorA_Final = amount; smoothValue(); break;
            case MixerOscBLevel:    state.mixerOscillatorB_Final = 2.0f * amount; smoothValue(); break;
            case MixerNoiseLevel:   state.mixerNoise = 0.35f * 5.0f * amount; break;
            case ExternalInputLevel: state.mixerExternal = inputVolume * amount; break;

            //==============================================================================
            case FilterCutoff:
                state.filter_Cutoff = -39.0f + float (int (120.0f * amount)); // step 1
                smoothValue();
                break;

            case FilterResonance:
                state.filter_Res = amount;
                state.filter_K = 4.1f * amount;
                updateResonance();
                break;

            case FilterHPF:
                state.filter_HPF = on;
                updateResonance();
                break;

            case FilterEnvAmt:
                state.filterEnvelopeFinal = state.filterEnvelopeInv * 192.0f * amount;
                smoothValue();
                break;

            case FilterInvertEnv:
                state.filterEnvelopeInv = on ? -1.0f : 1.0f;

                if (state.filterEnvelopeFinal > 0)
                    state.filterEnvelopeFinal = state.filterEnvelopeFinal * state.filterEnvelopeInv;
                else
                    state.filterEnvelopeFinal = -state.filterEnvelopeFinal * state.filterEnvelopeInv;

                smoothValue();
                break;

            case FilterKeyboardTracking:
                state.filterKeyboardFinal = amount;
                smoothValue();
                break;

            case FilterVersion:
                filterVersion = value > 0.6f ? 2 : (value < 0.3f ? 0 : 1);
                break;

            //==============================================================================
            case FilterAttack:
                state.envelopeF_Att = tau * timeToIncrement (-5.0f + 90.7f * amount);
                break;

            case FilterDecay:
                state.envelopeF_D_FAC = envelopeFactor (-3.0f + 85.0f * amount);

                for (auto& v : voices)
                    if (v.envelopeF_Stage == decay)
                        v.envelopeF_Fac = state.envelopeF_D_FAC;

                break;

            case FilterSustain:
                state.envelopeF_Sus = 0.98f * amount;

                for (auto& v : voices)
                {
                    if (v.envelopeF_Stage == decay)
                    {
                        v.envelopeF_Level = state.envelopeF_Sus * v.envelopeF_Peak;
                        v.envelopeF_SDif = v.envelopeF_Out - v.envelopeF_Level;
                    }
                }

                break;

            case FilterRelease:
                state.envelopeF_Rel = -10.0f + 92.7f * amount;
                updateFilterRelease();
                break;

            case AmplifierAttack:
                state.envelopeA_Att = tau * timeToIncrement (-5.0f + 90.7f * amount);
                break;

            case AmplifierDecay:
                state.envelopeA_D_FAC = envelopeFactor (-3.0f + 85.0f * amount);

                for (auto& v : voices)
                    if (v.envelopeA_Stage == decay)
                        v.envelopeA_Fac = state.envelopeA_D_FAC;

                break;

            case AmplifierSustain:
                state.envelopeA_Sus = 0.95f * amount;

                for (auto& v : voices)
                {
                    if (v.envelopeA_Stage == decay)
                    {
                        v.envelopeA_Level = (v.envelopeA_Peak - envelopeAOffset) * state.envelopeA_Sus + envelopeAOffset;
                        v.envelopeA_SDif = v.envelopeA_Out - v.envelopeA_Level;
                    }
                }

                break;

            case AmplifierRelease:
                state.envelopeA_Rel = -10.0f + 92.0f * amount;
                updateAmplifierRelease();
                break;

            case Release:
                state.envelopeRelease = on ? 1.0f : 0.0f;
                updateAmplifierRelease();
                updateFilterRelease();
                break;

            case Velocity:
                state.velocityFilter = on ? 0.55f : 0.0f;
                state.velocityAmp    = on ? 0.8f : 0.0f;
                break;

            case Repeat:            state.repeatActive = on; break;
            case Drone:             setDrone (on); break;

            //==============================================================================
            case LfoMidiSync:
                if (! on && state.midiTempo)
                    state.lfoFreq = 2.0f * state.lfoRatio; // stays quantised, but at 120bpm

                state.midiTempo = on;

                if (on)
                    syncLfoToTempo();

                break;

            case LfoFrequency:
            {
                const float octave = -5.5f + 9.725f * amount; // 10 octaves
                const int octaveInt = int (std::floor (octave)); // 0 = quarter notes; -6 = 16 bars, 4 = 64ths

                state.lfoRatio = 0.015625f * (1 << (octaveInt + 6)); // 2^octaveInt

                if (octave - octaveInt > 0.58)
                    state.lfoRatio *= 1.5f;

                if (state.midiTempo)
                    syncLfoToTempo();
                else
                    state.lfoFreq = pitchToFreq (-92.7f + 116.7f * amount); // -92.7 to 24
                break;
            }

            case LfoShapeSaw:
                setLfoShape (on, state.lfoSawA, state.lfoPulseA, state.lfoTriangleA);
                break;

            case LfoShapeTri:
                setLfoShape (on, state.lfoTriangleA, state.lfoPulseA, state.lfoSawA);
                break;

            case LfoShapePulse:
                setLfoShape (on, state.lfoPulseA, state.lfoTriangleA, state.lfoSawA);
                state.lfoPulseA = on ? 0.4f : 0.0f; // (so sample & hold also has a pulse, if it was this that turned it on)
                break;

            //==============================================================================
            case OscBFreq:
                state.oscillatorB_Freq = 0.375f * 127.0f * amount;
                updateOscillatorBPitch();
                break;

            case OscBKKeyboardTracking:
                state.oscillatorB_Keyboard = on ? 1.0f : 0.0f;
                updateOscillatorBPitch();
                break;

            case OscBFreqFine:
                state.oscillatorB_Fine = 0.015f + 0.935f * amount; // 0.015 to 0.95

                for (auto& v : voices)
                    tuneOscillatorB (v);

                break;

            case OscBSubOsc:
                state.oscillatorB_LOF = on ? -91.3f : 0.0f;

                for (auto& v : voices)
                    tuneOscillatorB (v);

                break;

            case OscBShapeSaw:      state.oscillatorB_SawA = on ? 0.5f : 0.0f; break; // 0.5 because the phase goes over 2 units
            case OscBShapeTri:      state.oscillatorB_TriA = on; break;
            case OscBShapePulse:    state.oscillatorB_PulseA = on ? 1.0f : 0.0f; break;
            case OscBPWAmount:      state.oscillatorB_PW_Final = amount * 2 - 1; smoothValue(); break;

            //==============================================================================
            case WheelModulationLfoNoise:   state.lfoNoiseMix = amount; break;
            case WheelModulationFreqOscA:   state.oscillatorA_FM_WM = on ? 0.4f : 0.0f; break;
            case WheelModulationFreqOscB:   state.oscillatorB_FM_WM = on ? 0.4f : 0.0f; break;
            case WheelModulationPWA:        state.oscillatorA_PW_WM = on ? 1.0f : 0.0f; break;
            case WheelModulationPWB:        state.oscillatorB_PW_WM = on ? 1.0f : 0.0f; break;
            case WheelModulationFilter:     state.filter_WM = on ? 60.0f : 0.0f; break;

            case ModWheel:          setModWheel (amount); break;
            case PitchBend:         setPitchBend (amount); break;

            //==============================================================================
            case Glide:
                state.glide = exp_dB (80.0f - 72.0f * amount);

                if (state.unisonActive)
                    updateGlideDirection();

                break;

            case Unison:            setUnison (on); break;
            case Analog:            setAnalog (amount); break;

            case MasterTune:
                state.masterTune = -0.7f + 1.4f * amount; // -0.7 to 0.7
                updateTune();
                break;

            case Volume:
                state.volumeFinal = masterVolume * amount;
                smoother();
                break;

            case ActiveVoices:
                if (activeVoices != int (value))
                {
                    activeVoices = std::clamp (int (value), 1, numVoices);
                    allSoundOff();
                }

                break;

            case TestTone:          testToneActive = on; break;

            //==============================================================================
            case DelayTime:
            {
                const float octave = -8.97f + (1.0f + 8.97f) * amount; // 10 octaves
                const int octaveInt = int (std::floor (octave)); // 1 = half note, 0 = quarter note, -2 = 16th

                state.delayTimeRatio = 0.001953125f * (1 << (octaveInt + 9)); // 2^octaveInt
                state.delayTimeRatio *= 60.0f; // convert BPM to Hz

                if (octave - octaveInt > 0.58)
                    state.delayTimeRatio *= 1.5f;

                if (state.delayMIDI)
                    state.delayTime = getTempoSyncedDelayTime();
                else
                    state.delayTime = exp_dB (-60.f + 60.0f * amount); // -60 to 0 = 1ms to 1sec

                computeDelayTimes();
                break;
            }

            case DelaySpread:
                state.delaySpread = amount;
                computeDelayTimes();
                break;

            case DelayDepth:
                state.delayDepthKnob = amount > 0 ? exp_E (5.0f * amount) * 0.000185f : 0.0f;
                state.delayDepth = state.delayDepthKnob * float (frequency);
                computeDelayTimes();
                break;

            case DelayRate:
                state.delayRate = pitchToFreq (-60 + 80.0f * amount); // P = -60 to 20, F = 0.26Hz to 26Hz
                updateDelayRates();
                break;

            case DelayFeedback:
                state.delayFeedbackKnob = amount * 0.25f;
                updateDelayFeedback();
                smoother();
                break;

            case DelayHiCut: // (really the low cut: see the note in the parameter list)
                state.delay_HPF_F = min (0.99f, pitchToFreq (0.0f + 120.0f * amount) * tauFilter);
                updateDelayFeedback();
                smoother();
                break;

            case DelayLoCut:
                state.delay_LPF_F = min (0.99f, pitchToFreq (40.0f + 80.0f * amount) * tauFilter);
                break;

            case DelayINV:
                state.delayInv = on ? -1.0f : 1.0f;
                updateDelayFeedback();
                state.delayWetFinal = getDelayWetLevel();
                smoother();
                break;

            case DelayON:           setDelayActive (on); break;

            case DelayWet:
                state.delayWetKnob = amount;

                if (state.delayActive)
                {
                    state.delayWetFinal = getDelayWetLevel();
                    state.delayDryFinal = getDelayDryLevel();
                    smoother();
                }

                break;

            case DelaySync:
                state.delaySync = on;
                updateDelayRates();

                if (! on) // set all LFOs to same phase, will drift apart
                    for (int i = 1; i < 4; ++i)
                        state.delayLFO_Pos[i] = state.delayLFO_Pos[0];

                computeDelayTimes();
                break;

            case DelayMidi:
                if (on)
                {
                    state.delayMIDI = true;
                    state.delayTime = getTempoSyncedDelayTime();
                    computeDelayTimes();
                }
                else if (state.delayMIDI) // switching MIDI sync off?
                {
                    state.delayMIDI = false;
                    state.delayTime = state.delayTimeRatio / 120.0f; // fudge: the time stays quantised, but at 120bpm
                    computeDelayTimes();
                }

                break;

            default:
                break;
        }
    }

    //==============================================================================
    void updateResonance()
    {
        float res;

        if (state.filter_Res < 0.25f)       res = 2.32f * state.filter_Res;
        else if (state.filter_Res < 0.5f)   res = 0.52f * state.filter_Res + 0.45f;
        else if (state.filter_Res < 0.75f)  res = 0.92f * state.filter_Res + 0.25f;
        else                                res = 0.036f * state.filter_Res + 0.913f;

        state.filter_D = 2.0f - 2.0f * res;
        state.filter_FMAX = getMaxFilterFrequency (res);

        res *= 0.8f;
        state.filterN_D = 2.0f - 2.0f * res;
        state.filterN_FMAX = getMaxFilterFrequency (res);

        smoothValue();
    }

    float getMaxFilterFrequency (float res) const
    {
        return state.filter_HPF ? 0.5f + 0.0318f * res * (27.5989f + res)
                                : 0.7352f + 0.2930f * res * (1.3075f + res);
    }

    // Turns one of the envelope time settings into the factor which a decaying
    // envelope's distance from its target gets multiplied by on each frame.
    float envelopeFactor (float time) const
    {
        const float factor = timeToIncrement (time) * tau;
        return factor > 1.0f ? 0.0f : 1.0f - factor;
    }

    void updateAmplifierRelease()
    {
        state.envelopeA_R_FAC = envelopeFactor (state.envelopeRelease * state.envelopeA_Rel);

        for (auto& v : voices)
            if (v.envelopeA_Stage == release)
                v.envelopeA_Fac = state.envelopeA_R_FAC;
    }

    void updateFilterRelease()
    {
        state.envelopeF_R_FAC = envelopeFactor (state.envelopeRelease * state.envelopeF_Rel);

        for (auto& v : voices)
            if (v.envelopeF_Stage == release)
                v.envelopeF_Fac = state.envelopeF_R_FAC;
    }

    // The three LFO shape buttons mix their waveforms, but turning all three on
    // gives sample & hold instead, and having none on leaves a constant level.
    void setLfoShape (bool on, float& shape, float& other1, float& other2)
    {
        if (on)
        {
            state.lfoConst = 0.0f;

            if (state.lfoShA == 0.0f)
            {
                shape = 0.4f;

                if (other1 != 0.0f && other2 != 0.0f) // all three buttons now on?
                {
                    state.lfoShA = 1.0f; // switch on Sample&Hold mode
                    shape = 0.0f;        // switch off all LFO waveforms
                    other1 = 0.0f;
                    other2 = 0.0f;
                }
            }
        }
        else
        {
            shape = 0.0f;

            if (state.lfoShA != 0.0f) // currently in Sample&Hold mode
            {
                state.lfoShA = 0.0f; // switch off Sample&Hold mode
                other1 = 0.4f;       // switch other LFO waveforms back on
                other2 = 0.4f;
            }
            else if (other1 == 0.0f && other2 == 0.0f) // all switches now off?
            {
                state.lfoConst = 0.4f;
            }
        }
    }

    void updateOscillatorBPitch()
    {
        // add 12 because the oscillators' phase goes over 2 units
        state.oscillatorB_Pitch = 12.0f + 11.0f - 35.0f * state.oscillatorB_Keyboard
                                    + float (int ((2.0f - state.oscillatorB_Keyboard)
                                                    * (state.oscillatorB_Freq + 0.4f)));

        for (auto& v : voices)
            tuneOscillatorB (v);
    }

    float getOscillatorATune (const VoiceState& v) const
    {
        return state.oscillatorA_Freq + v.oscillatorA_Rnd + state.tune;
    }

    float getOscillatorBTune (const VoiceState& v) const
    {
        return state.oscillatorB_LOF + state.oscillatorB_Pitch + state.oscillatorB_Fine + v.oscillatorB_Rnd + state.tune;
    }

    void tuneOscillatorA (VoiceState& v)
    {
        v.oscillatorA_Tune = getOscillatorATune (v);
        v.oscillatorA_F = pitchToFreq (v.pitch + v.oscillatorA_Tune);
    }

    void tuneOscillatorB (VoiceState& v)
    {
        v.oscillatorB_Tune = getOscillatorBTune (v);
        v.oscillatorB_F = pitchToFreq (state.oscillatorB_Keyboard * v.pitch + v.oscillatorB_Tune);
    }

    void setVoicePitch (VoiceState& v, float pitch)
    {
        v.pitch = pitch;
        v.oscillatorA_F = pitchToFreq (v.pitch + v.oscillatorA_Tune);
        v.oscillatorB_F = pitchToFreq (state.oscillatorB_Keyboard * v.pitch + v.oscillatorB_Tune);
    }

    void updateTune()
    {
        state.tune = state.masterTune + state.pitchBend;

        for (auto& v : voices)
        {
            tuneOscillatorA (v);
            tuneOscillatorB (v);
        }
    }

    void setPitchBend (float value)
    {
        state.pitchBend = 4.0f * (value - 0.5f); // -2 to 2
        updateTune();
    }

    void setModWheel (float value)
    {
        state.modulationWheel = value * 5; // 0 to 5 (5/127)
    }

    //==============================================================================
    // In unison mode, all the voices follow one pitch, which glides.

    // Starts the glide heading for its target, returning true if it's there already.
    bool updateGlideDirection()
    {
        if (state.glideFinal == state.glideOut)
        {
            state.glideInc = 0.0f;
            return true;
        }

        state.glideInc = (state.glideFinal > state.glideOut ? state.glide : -state.glide) * tau * controlPeriod;
        return false;
    }

    void glideTo (float pitch)
    {
        state.glideFinal = pitch;

        if (updateGlideDirection())
            for (int v = 0; v < activeVoices; ++v)
                setVoicePitch (voices[v], state.glideFinal + voices[v].unisonRnd);
    }

    void updatePitch()
    {
        if (state.unisonActive)
        {
            glideTo (state.unisonNote);
        }
        else
        {
            for (auto& v : voices)
                setVoicePitch (v, v.noteNumber >= 0 ? v.noteNumber : v.pitch);
        }
    }

    void setUnison (bool on)
    {
        state.unisonActive = on;

        if (! on)
        {
            state.glideInc = 0;

            for (auto& v : voices)
            {
                if (v.noteNumber >= 0)
                    setVoicePitch (v, v.noteNumber);

                if (v.gate <= 0)
                    releaseEnvelopes (v);
            }

            state.unisonGate = false;
        }
        else
        {
            glideTo (state.unisonNote);

            if (state.droneActive)
            {
                sustainedNotesOff();
                playHeldNote();
            }
        }
    }

    // The hold button ("drone") keeps notes sounding as a held sustain pedal
    // would, and plays a middle C if there's nothing to keep.
    void setDrone (bool on)
    {
        state.droneActive = on;

        if (on)
        {
            bool alreadyPlaying = false;

            for (int v = 0; v < activeVoices; ++v)
            {
                if (collVoiceNote[v] > 0)
                {
                    alreadyPlaying = true; // don't turn on C3
                    break;
                }
            }

            if (alreadyPlaying)
                sustainActive = true;
            else
                playHeldNote();
        }
        else
        {
            sustainedNotesOff();
        }
    }

    void playHeldNote()
    {
        startNote (60, 64);
        sustainActive = true;
        stopNote (60);
    }

    void setAnalog (float amount)
    {
        const float tune = amount > 0 ? exp_E (5.0f * amount) * 0.19205f : 0.0f;
        uint32_t seed = 1234567890;

        auto getNextRandom = [&seed] // -1 to 1
        {
            const uint32_t feedback = (seed ^ (seed >> 5) ^ (seed >> 13)) << 31;

            if (cmajorCompatibleAnalogRandomness)
                seed = (seed & 0x80000000u) | (seed >> 1); // an arithmetic shift

            else
                seed >>= 1;

            seed |= feedback;
            return -1.0f + (3.0518e-5f * float (seed & 65535));
        };

        for (int i = 0; i < numVoices; ++i)
        {
            auto& v = voices[i];

            v.oscillatorA_Rnd = 0.01f * tune * getNextRandom();
            v.oscillatorB_Rnd = 0.01f * tune * getNextRandom();
            v.filter_RND      = 0.5f * tune * getNextRandom();
            v.polymodEnvRnd   = 1.0f + 0.1f * tune * getNextRandom();

            const float random = getNextRandom();

            if (tune != 0)
                v.unisonRnd = 0.035f * tune * random;
            else // with no randomness, the unison voices get spread out evenly instead
                v.unisonRnd = 0.035f * 2.0f / float (activeVoices) * ((i & 1) != 0 ? ((i + 1) >> 1) : -(i >> 1));

            v.oscillatorA_Tune = getOscillatorATune (v);
            v.oscillatorB_Tune = getOscillatorBTune (v);
        }

        smoothValue();
        updatePitch(); // (which gets the oscillators' frequencies updated)
    }

    //==============================================================================
    // The delay effect

    float getDelayWetLevel() const
    {
        return state.delayWetKnob * (1.828f - 0.828f * state.delayWetKnob) * 0.5f * state.delayInv;
    }

    float getDelayDryLevel() const
    {
        return (1.0f - state.delayWetKnob) * (1.828f - 0.828f * (1.0f - state.delayWetKnob));
    }

    void updateDelayFeedback()
    {
        state.delayFeedbackFinal = state.delayFeedbackKnob * state.delayInv * (1.0f - 0.5f * state.delay_HPF_F);
    }

    void updateDelayRates()
    {
        if (state.delaySync)
        {
            state.delayRates[0] = state.delayRate; // only the first rate is used in sync mode
        }
        else
        {
            state.delayRates[0] = state.delayRate * 1.029f; // +0.5 semitones
            state.delayRates[1] = state.delayRate * 0.917f; // -1.5 semitones
            state.delayRates[2] = state.delayRate * 0.972f; // -0.5 semitones
            state.delayRates[3] = state.delayRate * 1.091f; // +1.5 semitones
        }
    }

    void setDelayActive (bool on)
    {
        if (! on)
        {
            state.delayActive = false;
            state.delayWetFinal = 0.0f;
            state.delayDryFinal = 1.0f;
            smoother();
        }
        else if (! state.delayActive)
        {
            state.delayActive = true;

            state.delayWetFinal = getDelayWetLevel();
            state.delayDryFinal = getDelayDryLevel();
            smoother();

            state.delaySum = 0.0f;
            state.delay_HPF_LPF = 0.0f;
            state.delay_LPF_LPF = 0.0f;

            for (int i = 0; i < 4; ++i)
            {
                state.delayTime_Inc[i] = 0.0f;
                state.delayLFO_Pos[i] = 0.0f;
                state.delayTime_Mod[i] = state.delayTimes[i];
            }

            std::fill (std::begin (delayBuffer), std::end (delayBuffer), 0.0f);
        }
    }

    // The delay has four taps, whose times are spread out around the one that's set.
    void computeDelayTimes()
    {
        static constexpr float syncedSpread[] = { -1.0f, -0.5f, 0.5f, 1.0f };
        static constexpr float freeSpread[]   = { -1.0f, -0.4f, 0.5f, 0.9f };

        // make it an exact number of samples to minimise treble loss
        const float time = float (int (state.delayTime * frequency + 0.5));
        const float timeLimit = (delayLength - 1) - state.delayDepth;
        auto& spread = state.delaySync ? syncedSpread : freeSpread;

        for (int i = 0; i < 4; ++i)
        {
            float t = time * (1.0f + spread[i] * state.delaySpread);

            // lower limit for delay times, so that at full modulation time is still >= 0
            if (t < state.delayDepth)
                t = state.delayDepth;

            // upper limit for delay times
            if (t > timeLimit)
                t = timeLimit;

            state.delayTimes[i] = t;
        }
    }

    //==============================================================================
    // Tempo sync. This isn't in the Cmajor patch, which has no way of knowing the
    // tempo: its "MIDI" buttons just stop the knobs having any effect.

    void setTempo (double bpm)
    {
        bpm = std::clamp (bpm, 20.0, 999.0);

        if (bpm == tempo)
            return;

        tempo = bpm;

        if (state.midiTempo)
            syncLfoToTempo();

        if (state.delayMIDI)
        {
            state.delayTime = getTempoSyncedDelayTime();
            computeDelayTimes();
        }
    }

    void syncLfoToTempo()
    {
        state.lfoFreq = float (tempo / 60.0) * state.lfoRatio;
    }

    float getTempoSyncedDelayTime() const
    {
        return state.delayTimeRatio / float (tempo);
    }

    //==============================================================================
    // Knob changes are smoothed, by ramping to the new values over 20 control periods...
    void smoothValue()
    {
        state.knobSmoothingCounter = knobSmoothingTime;
        const float factor = 1.0f / knobSmoothingTime;

        state.mixerOscillatorA_Inc  = undenormalise ((state.mixerOscillatorA_Final - state.mixerOscillatorA) * factor);
        state.mixerOscillatorB_Inc  = undenormalise ((state.mixerOscillatorB_Final - state.mixerOscillatorB) * factor);
        state.oscillatorA_PW_Inc    = undenormalise ((state.oscillatorA_PW_Final - state.oscillatorA_PW) * factor);
        state.oscillatorB_PW_Inc    = undenormalise ((state.oscillatorB_PW_Final - state.oscillatorB_PW) * factor);
        state.filter_EnvInc         = undenormalise ((state.filterEnvelopeFinal - state.filter_Env) * factor);
        state.filter_KeyboardInc    = undenormalise ((state.filterKeyboardFinal - state.filter_Keyboard) * factor);

        const float final = 35.0f * (1.0f - state.filterKeyboardFinal) + state.filter_Cutoff + 4.5f * state.filter_Res;
        state.filter_EventInc       = undenormalise ((final - state.filter_Event) * factor);
    }

    // ...and the output levels over 20ms
    void smoother()
    {
        state.smootherCount = int (0.02 * frequency);
        const float factor = 1.0f / state.smootherCount;

        state.volumeInc        = undenormalise ((state.volumeFinal - state.volumeSmooth) * factor);
        state.delayWetInc      = undenormalise ((state.delayWetFinal - state.delayWetSmooth) * factor);
        state.delayDryInc      = undenormalise ((state.delayDryFinal - state.delayDrySmooth) * factor);
        state.delayFeedbackInc = undenormalise ((state.delayFeedbackFinal - state.delayFeedbackSmooth) * factor);
    }

    //==============================================================================
    // Envelopes. The attack is a linear ramp up to a peak that depends on the
    // velocity; after that, the envelope closes in on a level exponentially.

    void startAmplifierDecay (VoiceState& v)
    {
        v.envelopeA_Stage = decay;
        v.envelopeA_Fac = state.envelopeA_D_FAC;
        v.envelopeA_Level = (v.envelopeA_Peak - envelopeAOffset) * state.envelopeA_Sus + envelopeAOffset; // the sustain level is a part of the peak level
        v.envelopeA_SDif = v.envelopeA_Out - v.envelopeA_Level;
    }

    void startFilterDecay (VoiceState& v)
    {
        v.envelopeF_Stage = decay;
        v.envelopeF_Fac = state.envelopeF_D_FAC;
        v.envelopeF_Level = v.envelopeF_Peak * state.envelopeF_Sus;
        v.envelopeF_SDif = v.envelopeF_Out - v.envelopeF_Level;
    }

    float getAmplifierPeak (float velocity) const   { return 0.5f + state.velocityAmp * (velocity * 0.0078125f - 0.5f); }
    float getFilterPeak (float velocity) const      { return 0.5f + state.velocityFilter * (velocity * 0.0078125f - 0.5f); }

    void gateOn (VoiceState& v, float velocity)
    {
        const float peakA = getAmplifierPeak (velocity);
        const float peakF = getFilterPeak (velocity);

        v.isActive = true;

        if (v.envelopeA_Out > peakA)
        {
            v.envelopeA_Peak = v.envelopeA_Out;
            startAmplifierDecay (v);
        }
        else
        {
            v.envelopeA_Inc = peakA * state.envelopeA_Att;
            v.envelopeA_Peak = peakA;
            v.envelopeA_Stage = attack;
        }

        if (v.envelopeF_Out > peakF)
        {
            v.envelopeF_Peak = v.envelopeF_Out;
            startFilterDecay (v);
        }
        else
        {
            v.envelopeF_Inc = peakF * state.envelopeF_Att;
            v.envelopeF_Peak = peakF;
            v.envelopeF_Stage = attack;
            v.envelopeF_SDif = 1.0f;
        }
    }

    // Used in unison mode for a note that's played legato: the envelopes carry
    // on, but head for the new note's levels.
    void changeVelocity (VoiceState& v, float velocity)
    {
        const float peakA = getAmplifierPeak (velocity);
        const float peakF = getFilterPeak (velocity);

        v.envelopeA_Peak = peakA;

        if (v.envelopeA_Stage == attack && v.envelopeA_Out < peakA) // still in attack
            v.envelopeA_Inc = peakA * state.envelopeA_Att;
        else
            startAmplifierDecay (v);

        v.envelopeF_Peak = peakF;

        if (v.envelopeF_Stage == attack && v.envelopeF_Out < peakF) // still in attack
        {
            v.envelopeF_Inc = peakF * state.envelopeF_Att;
            v.envelopeF_SDif = 1.0f;
        }
        else
        {
            startFilterDecay (v);
        }
    }

    void releaseEnvelopes (VoiceState& v)
    {
        v.envelopeA_Stage = release;
        v.envelopeA_Fac = state.envelopeA_R_FAC;
        v.envelopeA_Level = envelopeAOffset;
        v.envelopeA_SDif = v.envelopeA_Out - v.envelopeA_Level;

        v.envelopeF_Stage = release;
        v.envelopeF_Fac = state.envelopeF_R_FAC;
        v.envelopeF_Level = 0.0f;
        v.envelopeF_SDif = v.envelopeF_Out - v.envelopeF_Level;
    }

    int countHeldVoices() const
    {
        int num = 0;

        for (int v = 0; v < activeVoices; ++v)
            if (voices[v].gate > 0.0f)
                ++num;

        return num;
    }

    //==============================================================================
    void triggerNoteOn (int voice, int vel)
    {
        auto& v = voices[voice];
        const int notePitch = collVoiceNote[voice];

        v.noteNumber = float (notePitch);

        if (state.unisonActive)
        {
            state.unisonNote = float (notePitch);
            glideTo (v.noteNumber);
            state.unisonGate = true;

            const int numHeld = countHeldVoices();

            if (state.repeatActive)
            {
                if (! state.midiTempo && numHeld == 0)
                    state.lfoPhase = 1.0f; // key sync LFO, when not synced to song tempo
            }
            else // only trigger when Repeat is off, otherwise triggering will happen from LFO
            {
                for (int i = 0; i < activeVoices; ++i)
                {
                    auto& u = voices[i];

                    if (numHeld == 0)
                    {
                        if (u.envelopeA_Out < 0.0f)
                            u.envelopeA_Out = 0.0f;

                        gateOn (u, float (vel));
                    }
                    else // already playing but change in envelope amplitude by velocity is possible
                    {
                        changeVelocity (u, float (vel));
                    }
                }
            }
        }
        else
        {
            if (state.repeatActive)
            {
                if (! state.midiTempo && countHeldVoices() == 0)
                    state.lfoPhase = 1.0f; // key sync LFO, when not synced to song tempo
            }
            else // only trigger when Repeat is off, otherwise triggering will happen from LFO
            {
                gateOn (v, float (vel));
            }

            setVoicePitch (v, v.noteNumber);
            state.unisonNote = float (notePitch);
        }

        v.gate = float (vel);
    }

    void triggerNoteOff (int voice)
    {
        auto& v = voices[voice];
        const int notePitch = collVoiceNote[voice];

        v.noteNumber = float (notePitch);

        if (state.unisonActive)
        {
            if (notePitch != state.unisonNote)
            {
                v.gate = 0.0f;
                return;
            }

            // The note that was sounding has gone, so fall back to the nearest of any others that are held
            int numHeld = 0;
            float nearestNote = 1000.0f;

            for (int i = 0; i < activeVoices; ++i)
            {
                if (voices[i].gate > 0.0f)
                {
                    ++numHeld;

                    const int bestDiff = int (notePitch - nearestNote);
                    const int currDiff = int (notePitch - voices[i].noteNumber);

                    if (currDiff * currDiff < bestDiff * bestDiff && i != voice)
                        nearestNote = voices[i].noteNumber;
                }
            }

            if (v.gate > 0.0f)
            {
                if (numHeld == 1)
                {
                    for (int i = 0; i < activeVoices; ++i)
                        releaseEnvelopes (voices[i]);

                    state.unisonGate = false;
                }
                else if (numHeld > 1)
                {
                    state.unisonNote = nearestNote;
                    glideTo (state.unisonNote);
                }
            }
        }
        else
        {
            releaseEnvelopes (v);
        }

        v.gate = 0.0f;
    }

    //==============================================================================
    // Voice allocation

    void startNote (int note, int vel)
    {
        const int v = wrapVoice (allocateVoice (oldestVoice, note));

        collVoiceNote[v] = note;
        triggerNoteOn (v, vel);
        collVoiceSustain[v] = false;
        collVoiceHold[v] = false;
    }

    void stopNote (int note)
    {
        for (int v = 0; v < activeVoices; ++v)
        {
            if (collVoiceNote[v] == note)
            {
                if (sustainActive)
                    collVoiceSustain[v] = true;

                if (! (collVoiceSustain[v] || collVoiceHold[v]))
                    stopVoice (v);
            }
        }
    }

    void stopVoice (int v)
    {
        triggerNoteOff (v);
        releaseVoice (v);
        collVoiceNote[v] = noteOffMarker - collVoiceNote[v];
    }

    int allocateVoice (int last, int note)
    {
        // If there's a voice that is playing this note, or was the last to, use it again..
        for (int v = 0; v < activeVoices; ++v)
        {
            if (collVoiceNote[v] == note || collVoiceNote[v] == noteOffMarker - note)
            {
                if (v == oldestVoice)
                {
                    oldestVoice = collVoiceNext[wrapVoice (oldestVoice)];

                    if (v == youngestOffVoice)
                        youngestOffVoice = -1;
                }
                else if (v == youngestOffVoice)
                {
                    youngestOffVoice = collVoicePrev[wrapVoice (youngestOffVoice)];
                }

                insertBefore (wrapVoice (last), v);
                return v;
            }
        }

        // ..otherwise take the one that was released longest ago..
        if (youngestOffVoice >= 0)
        {
            const int s = wrapVoice (oldestVoice);
            oldestVoice = collVoiceNext[s];

            if (s == youngestOffVoice)
                youngestOffVoice = -1;

            insertBefore (wrapVoice (last), s);
            return s;
        }

        // ..or, if they're all in use, steal the oldest
        int s = collVoicePrev[wrapVoice (last)];
        int v = s;
        bool foundOldest = false;

        while (! foundOldest)
        {
            s = v;
            v = collVoicePrev[wrapVoice (v)];
            foundOldest = (v == collVoicePrev[wrapVoice (oldestVoice)]);
        }

        if (s == oldestVoice)
            oldestVoice = collVoiceNext[wrapVoice (oldestVoice)];

        insertBefore (wrapVoice (last), wrapVoice (s));
        return s;
    }

    void releaseVoice (int v)
    {
        if (youngestOffVoice == -1)
        {
            insertBefore (wrapVoice (oldestVoice), v);
            oldestVoice = v;
        }
        else
        {
            insertAfter (wrapVoice (youngestOffVoice), v);
        }

        youngestOffVoice = v;
    }

    void insertAfter (int dest, int source)
    {
        if (source == dest)
            return;

        collVoiceNext[wrapVoice (collVoicePrev[source])] = collVoiceNext[source];
        collVoicePrev[wrapVoice (collVoiceNext[source])] = collVoicePrev[source];

        collVoiceNext[source] = collVoiceNext[dest];
        collVoicePrev[source] = dest;
        collVoicePrev[wrapVoice (collVoiceNext[dest])] = source;
        collVoiceNext[dest] = source;
    }

    void insertBefore (int dest, int source)
    {
        if (source != dest)
            insertAfter (wrapVoice (collVoicePrev[dest]), source);
    }

    void initVoiceQueue()
    {
        for (int i = 0; i < numVoices; ++i)
        {
            collVoiceSustain[i] = false;
            collVoiceHold[i] = false;
            triggerNoteOff (i);
            collVoiceNote[i] = noteOffMarker;
        }

        for (int i = 0; i < numVoices; ++i)
        {
            collVoicePrev[i] = i - 1;
            collVoiceNext[i] = i + 1;
        }

        collVoicePrev[0] = activeVoices - 1;
        collVoiceNext[wrapVoice (activeVoices - 1)] = 0;

        youngestOffVoice = activeVoices - 1;
        oldestVoice = 0;
        sustainActive = false;

        if (state.droneActive)
            playHeldNote();
    }

    void sustainedNotesOff()
    {
        sustainActive = false;

        for (int v = 0; v < activeVoices; ++v)
        {
            if (collVoiceSustain[v] && ! collVoiceHold[v])
            {
                collVoiceSustain[v] = false;
                stopVoice (v);
            }
        }
    }

    void setSustain (float value)
    {
        if (value > 0.5f)
            sustainActive = true;
        else
            sustainedNotesOff();
    }

    void setSostenuto (float value)
    {
        if (value > 0.5f)
        {
            // Set sustain on for all notes that are currently playing
            for (int v = 0; v < activeVoices; ++v)
                if (collVoiceNote[v] != noteOffMarker)
                    collVoiceSustain[v] = true;
        }
        else
        {
            sustainedNotesOff();
        }
    }

    // Not something the Cmajor patch does: it ignores the "all notes off"
    // message that hosts send to cut short whatever is still playing.
    void allNotesOff()
    {
        for (int v = 0; v < activeVoices; ++v)
        {
            if (collVoiceNote[v] >= 0)
            {
                collVoiceSustain[v] = false;
                collVoiceHold[v] = false;
                stopVoice (v);
            }
        }
    }

    void allSoundOff()
    {
        for (int i = 0; i < numVoices; ++i)
        {
            auto& v = voices[i];

            v.gate = 0.0f;
            v.noteNumber = 255.0f;

            v.oscillatorA_Phase = std::fmod (float (i) * 1.97f, 2.0f) - 1; // almost random phase distribution
            v.oscillatorB_Phase = std::fmod (float (i) * 1.87f, 2.0f) - 1;

            v.oscillatorA_Saw_AA2 = 0.0f;
            v.oscillatorA_Pulse_AA2 = 0.0f;
            v.oscillatorB_Saw_AA2 = 0.0f;
            v.oscillatorB_Pulse_AA2 = 0.0f;

            v.filter_BPF = 0.0f;
            v.filter_LPF = 0.0f;
            v.filterN_BPF = 0.0f;
            v.filterN_LPF = 0.0f;

            v.envelopeA_Out = 0.0f;
            v.envelopeA_SDif = 0.0f;
            v.envelopeA_Level = 0.0f;
            v.envelopeF_Out = 0.0f;
            v.isActive = false;

            v.filter_Event = state.filter_Event + v.filter_RND;
        }

        state.unisonNote = 36.0f;
        std::fill (std::begin (delayBuffer), std::end (delayBuffer), 0.0f);
        setPitchBend (0.5f);
        initVoiceQueue();
    }

    //==============================================================================
    // Rendering. In the Cmajor source this is all one function.

    // What the monophonic modulation sources come to for a frame
    struct Modulation
    {
        float pulseWidthA, pulseWidthB;
        float oscillatorAFM, oscillatorBFM, filter;
        float noise;
    };

    void renderFrame (float externalIn, float& left, float& right)
    {
        const Modulation mod = renderModulation();

        --state.controlCount;

        if (state.controlCount <= 0)
        {
            state.controlCount = controlPeriod;
            renderControlRate (mod);
        }

        const float inputSignal = externalIn * state.mixerExternal;
        float voiceOutput = 0.0f;

        for (int v = 0; v < activeVoices; ++v)
            if (voices[v].isActive)
                voiceOutput += renderVoice (voices[v], mod, inputSignal);

        if (state.smootherCount > 0)
        {
            --state.smootherCount;

            state.volumeSmooth += state.volumeInc;
            state.delayWetSmooth += state.delayWetInc;
            state.delayDrySmooth += state.delayDryInc;
            state.delayFeedbackSmooth += state.delayFeedbackInc;
        }

        float dryOut = voiceOutput * state.delayDrySmooth;
        float delayL = 0.0f, delayR = 0.0f;

        if (state.delayActive)
            renderDelay (voiceOutput, delayL, delayR);

        if (testToneActive)
            dryOut += renderTestTone();

        left  = state.volumeSmooth * (dryOut + delayL);
        right = state.volumeSmooth * (dryOut + delayR);
    }

    //==============================================================================
    // The LFO and the noise source, mixed and scaled by the mod wheel
    Modulation renderModulation()
    {
        float LFO_Pulse = 0.0f, LFO_Saw = 0.0f, LFO_Tri = 0.0f;

        const float Inc = state.lfoFreq * tau;
        state.lfoPhase += Inc;

        if (state.lfoPulseSign) // in the second half of the cycle
        {
            const float Diff = 1.0f - state.lfoPhase;

            if (Diff > 0.5f * Inc)
            {
                LFO_Pulse = state.lfoPulseA;
                LFO_Saw = state.lfoSawA * state.lfoPhase;
                LFO_Tri = state.lfoTriangleA * (2.0f * state.lfoPhase - 1.5f);
            }
            else // the end of the cycle
            {
                const float diffInc = 0.5f + Diff / Inc;

                LFO_Pulse = state.lfoPulseA * diffInc;
                LFO_Saw = state.lfoSawA * (diffInc - Diff);
                LFO_Tri = state.lfoTriangleA * (0.5f - Inc);

                state.lfoPhase -= 1.0f;
                state.lfoPulseSign = false;
                state.lfoSh = state.lfoShA * state.noiseFilterLPF;

                if (state.repeatActive) // the LFO is triggering the envelopes
                    for (int v = 0; v < activeVoices; ++v)
                        if (voices[v].gate > 0.0f || state.unisonGate)
                            gateOn (voices[v], voices[v].gate);
            }
        }
        else
        {
            const float Diff = 0.5f - state.lfoPhase;

            if (Diff > 0.5f * Inc)
            {
                LFO_Pulse = 0.0f;
                LFO_Tri = state.lfoTriangleA * (0.5f - 2.0f * state.lfoPhase);
            }
            else // half way
            {
                LFO_Pulse = state.lfoPulseA * (0.5f - (Diff / Inc));
                LFO_Tri = state.lfoTriangleA * (-0.5f + Inc);

                state.lfoPulseSign = true;

                if (state.repeatActive)
                    for (int v = 0; v < activeVoices; ++v)
                        if (voices[v].isActive && (voices[v].gate > 0.0f || state.unisonGate))
                            releaseEnvelopes (voices[v]);
            }

            LFO_Saw = state.lfoSawA * state.lfoPhase;
        }

        // Noise, from a shift register
        uint32_t Wort = state.noiseMem;
        const uint32_t Feedback = (Wort ^ (Wort >> 5) ^ (Wort >> 13)) << 31;
        Wort >>= 1;
        Wort |= Feedback;
        state.noiseMem = Wort;

        Modulation mod;
        mod.noise = -0.5f + float (Wort & 1);

        // Noise filter
        const float Hpf = mod.noise - state.noiseFilterLPF;
        state.noiseFilterLPF += Hpf * state.noiseFilterFreq;

        // Xfade
        const float In1 = LFO_Pulse + LFO_Saw + LFO_Tri + state.lfoSh + state.lfoConst;
        const float In2 = state.noiseFilterLPF;
        const float WheelMod = (In1 + state.lfoNoiseMix * (In2 - In1)) * state.modulationWheel;

        mod.pulseWidthA = state.oscillatorA_PW + state.oscillatorA_PW_WM * WheelMod; // clipped after Polymod added
        mod.pulseWidthB = clamp (state.oscillatorB_PW + state.oscillatorB_PW_WM * WheelMod, -1.0f, 1.0f);

        mod.oscillatorAFM = state.oscillatorA_FM_WM * WheelMod;
        mod.oscillatorBFM = state.oscillatorB_FM_WM * WheelMod;
        mod.filter = state.filter_WM * WheelMod;
        return mod;
    }

    //==============================================================================
    void renderControlRate (const Modulation& mod)
    {
        // Knob Smoothing
        if (state.knobSmoothingCounter != 0)
        {
            --state.knobSmoothingCounter;

            state.mixerOscillatorA += state.mixerOscillatorA_Inc;
            state.mixerOscillatorB += state.mixerOscillatorB_Inc;
            state.oscillatorA_SawA = state.mixerOscillatorA * state.oscillatorA_SawActive;

            state.oscillatorA_PW += state.oscillatorA_PW_Inc;
            state.oscillatorB_PW += state.oscillatorB_PW_Inc;

            state.filter_Event += state.filter_EventInc;

            for (int v = 0; v < activeVoices; ++v)
                voices[v].filter_Event = state.filter_Event + voices[v].filter_RND;

            state.filter_Env += state.filter_EnvInc;
            state.filter_Keyboard += state.filter_KeyboardInc;
        }

        // Glide
        if (state.glideInc != 0.0f)
        {
            state.glideOut += state.glideInc;

            if ((state.glideOut - state.glideFinal) * state.glideInc > 0.0f) // gone past
            {
                state.glideOut = state.glideFinal;
                state.glideInc = 0.0f;
            }

            for (int v = 0; v < activeVoices; ++v)
                setVoicePitch (voices[v], state.glideOut + voices[v].unisonRnd);
        }

        for (int i = 0; i < activeVoices; ++i)
        {
            auto& v = voices[i];

            if (! v.isActive)
                continue;

            updateFilterFrequency (v, mod);

            if (v.envelopeA_Out < 0.0f) // the voice has finished
            {
                v.envelopeA_Out = 0.0f;
                v.envelopeA_SDif = 0.0f;
                v.envelopeA_Level = 0.0f;
                v.envelopeF_Out = 0.0f;
                v.isActive = false;
            }
            else if (v.envelopeA_SDif < 0.0001f) // -80dB
            {
                if (v.envelopeA_SDif > -0.0001f) // -80dB
                {
                    v.envelopeA_SDif = 0.0f;
                    v.envelopeA_Level = v.envelopeA_Out;
                }
            }

            if (v.envelopeF_SDif < 0.0001f) // -80dB
            {
                if (v.envelopeF_SDif > -0.0001f) // -80dB
                {
                    v.envelopeF_SDif = 0.0f;
                    v.envelopeF_Level = v.envelopeF_Out;
                }
            }

            if (v.filter1HP_LPF * v.filter1HP_LPF < 0.0000001) // -80dB
                v.filter1HP_LPF = 0.0f;
        }

        // Delay LFOs: triangles, that move the taps' times about.
        // The waveform is made by setting the increments which the times ramp by.
        if (state.delayActive)
        {
            float tri = 0.0f;

            for (int i = 0; i < 4; ++i)
            {
                if (i == 0 || ! state.delaySync) // when synced, the taps all follow the first LFO
                {
                    float& pos = state.delayLFO_Pos[i];
                    pos += state.delayRates[i] * (4 * controlPeriod) * tau;

                    if (pos > 1.0f)
                    {
                        if (pos > 3.0f)
                        {
                            pos -= 4.0f;
                            tri = pos;
                        }
                        else
                        {
                            tri = 2.0f - pos; // pos = 1 ... 3, tri = 1 ... -1
                        }
                    }
                    else
                    {
                        tri = pos; // pos = -1 ... 1, tri = -1 ... 1
                    }
                }

                const float final = state.delayTimes[i] + state.delayDepth * tri;
                state.delayTime_Inc[i] = (final - state.delayTime_Mod[i]) * tau * 25; // transition time = 40ms = 1/25 sec
            }
        }
    }

    // Starts the voice's filter frequency moving towards where its modulation says it should be
    void updateFilterFrequency (VoiceState& v, const Modulation& mod)
    {
        const float final = pitchToFreq (mod.filter + state.filter_Env * v.envelopeF_Out + v.filter_Event
                                           + state.filter_Keyboard * v.pitch);

        v.filter_F_Inc = (final - v.filter_F_Smooth) * (0.8f / controlPeriod);
    }

    //==============================================================================
    float renderVoice (VoiceState& v, const Modulation& mod, float inputSignal)
    {
        // Envelopes
        if (v.envelopeA_Stage != attack)
        {
            v.envelopeA_SDif *= v.envelopeA_Fac;
            v.envelopeA_Out = v.envelopeA_Level + v.envelopeA_SDif;
        }
        else
        {
            v.envelopeA_Out += v.envelopeA_Inc;

            if (v.envelopeA_Out > v.envelopeA_Peak)
            {
                v.envelopeA_Out = v.envelopeA_Peak;
                startAmplifierDecay (v);
            }
        }

        if (v.envelopeF_Stage != attack)
        {
            v.envelopeF_SDif *= v.envelopeF_Fac;
            v.envelopeF_Out = v.envelopeF_Level + v.envelopeF_SDif;
        }
        else
        {
            v.envelopeF_Out += v.envelopeF_Inc;

            if (v.envelopeF_Out > v.envelopeF_Peak)
            {
                v.envelopeF_Out = v.envelopeF_Peak;
                startFilterDecay (v);
                updateFilterFrequency (v, mod);
            }
        }

        // Oscillator B. The oscillators' phases run from -1 to 1.
        float OscB_Saw = 0.0f, OscB_Pulse = 0.0f, OscB_Tri = 0.0f;
        bool oscillatorSyncNow = false; // set when oscillator A needs to be reset in this sample..
        float Osc_Sync_x = 0.0f;        // ..and this is how far through the sample to do it

        {
            float f = (v.oscillatorB_F + v.oscillatorB_F * mod.oscillatorBFM) * tau;
            float ph = v.oscillatorB_Phase;

            f = max (f, 0.0f);

            if (f >= 1.0f)
            {
                ph = 0.0f;
            }
            else
            {
                ph = (ph + f);

                if (ph >= 1.0f)
                {
                    oscillatorSyncNow = state.oscillatorSyncActive; // Sync OSC-A now if sync is on

                    ph = ph - 2.0f;

                    const float d = 1.0f + ph;
                    const float f_rcp = 1.0f / f;
                    const float x = d * f_rcp;
                    const PhaseWrap wrap { f_rcp, stepCorrection1 (x), stepCorrection2 (x) };
                    Osc_Sync_x = x;

                    const float y_saw = ph + v.oscillatorB_Saw_AA2 + wrap.correction1;
                    v.oscillatorB_Saw_AA2 = wrap.correction2;
                    OscB_Saw = state.oscillatorB_SawA * (y_saw + 1.0f); // add one to give correct modulation DC offset

                    if (state.oscillatorB_PulseA != 0)
                        OscB_Pulse = 0.5f - 0.5f * renderPulse (ph, mod.pulseWidthB, f, &wrap, v.oscillatorB_Pulse_AA2);

                    if (state.oscillatorB_TriA)
                    {
                        const float aa_top = (x * (2.0f - x) - 1.0f);

                        if (ph * (ph - f) < 0.0f) // ..and it has gone through the bottom of the triangle too
                        {
                            const float aa_btm = (1.0f - f_rcp * (2.0f + f_rcp));
                            OscB_Tri = 0.5f - (0.5f * f - ph) - f * (aa_top - aa_btm);
                        }
                        else
                        {
                            OscB_Tri = 0.5f - (0.5f * f - ph) - f * (aa_top);
                        }
                    }
                }
                else
                {
                    const float y_saw = ph + v.oscillatorB_Saw_AA2;
                    v.oscillatorB_Saw_AA2 = 0;
                    OscB_Saw = state.oscillatorB_SawA * (y_saw + 1.0f); // add one to give correct modulation DC offset

                    if (state.oscillatorB_PulseA != 0)
                        OscB_Pulse = 0.5f - 0.5f * renderPulse (ph, mod.pulseWidthB, f, nullptr, v.oscillatorB_Pulse_AA2);

                    if (state.oscillatorB_TriA)
                    {
                        const float ph_sign = sign (ph);

                        if (ph * (ph - f) < 0) // gone through the bottom of the triangle
                        {
                            const float x5 = std::abs (ph) / f;
                            const float aa_btm = (x5 * (2.0f - x5) - 1.0f);
                            OscB_Tri = 0.5f + ph_sign * (0.5f * f - ph) + f * aa_btm;
                        }
                        else
                        {
                            OscB_Tri = 0.5f + ph_sign * (0.5f * f - ph);
                        }
                    }
                }
            }

            v.oscillatorB_Phase = ph;
        }

        // Poly Mod
        const float OscB_PulseTri = OscB_Pulse + OscB_Tri;

        const float PolyMod = state.polymodOscillatorB * (OscB_Saw + OscB_PulseTri)
                                + state.polymodEnvelopeF * (v.envelopeF_Out * v.polymodEnvRnd);

        // Oscillator A
        float OscA_Saw = 0.0f, OscA_Pulse = 0.0f;

        {
            float f = (v.oscillatorA_F + v.oscillatorA_F * (mod.oscillatorAFM + state.oscillatorA_FM_PM * PolyMod)) * tau;
            f = max (f, 0.0f);

            float ph = v.oscillatorA_Phase;

            if (f >= 1.0f)
            {
                ph = 0.0f;
            }
            else
            {
                ph = (ph + f);

                if (oscillatorSyncNow)
                {
                    const float x_snc = Osc_Sync_x;
                    const float d_snc = x_snc * f;
                    const float snc_ph = d_snc - 1.0f; // where the phase is after restarting
                    const float ph_adj = ph - d_snc;   // and where it was when it got reset

                    PhaseWrap wrap;
                    const bool wrapped = ph_adj >= 1.0f; // did it wrap before being reset?

                    if (wrapped)
                    {
                        ph = ph - 2.0f;

                        const float d = 1.0f - std::abs (ph);
                        const float f_rcp = 1.0f / f;
                        const float x = d * f_rcp;
                        wrap = { f_rcp, stepCorrection1 (x), stepCorrection2 (x) };
                    }

                    const float ph2 = ph;
                    const float ph_snc_change = (snc_ph - ph);
                    ph = ph + ph_snc_change;

                    const float snc_aa_amp = (-0.5f) * ph_snc_change;
                    const SyncReset sync { d_snc, ph_snc_change, stepCorrection1 (x_snc), stepCorrection2 (x_snc) };

                    const float aa_snc_1_saw = sync.correction1 * snc_aa_amp;
                    const float aa_snc_2_saw = sync.correction2 * snc_aa_amp;

                    if (wrapped)
                    {
                        const float y_saw = ph + v.oscillatorA_Saw_AA2 + wrap.correction1 + aa_snc_1_saw;
                        v.oscillatorA_Saw_AA2 = wrap.correction2 + aa_snc_2_saw;
                        OscA_Saw = state.oscillatorA_SawA * y_saw;
                    }
                    else
                    {
                        const float y_saw = ph + v.oscillatorA_Saw_AA2 + aa_snc_1_saw;
                        v.oscillatorA_Saw_AA2 = aa_snc_2_saw;
                        OscA_Saw = state.oscillatorA_SawA * y_saw;
                    }

                    if (state.oscillatorA_PulseActive)
                    {
                        const float ph_pwm = clamp (mod.pulseWidthA + state.oscillatorA_PW_PM * PolyMod, -1.0f, 1.0f);

                        OscA_Pulse = state.mixerOscillatorA * renderSyncedPulse (ph2, ph, ph_pwm, f, wrapped ? &wrap : nullptr,
                                                                                 sync, v.oscillatorA_Pulse_AA2);
                    }
                }
                else // no sync
                {
                    PhaseWrap wrap;
                    const bool wrapped = ph >= 1.0f;

                    if (wrapped)
                    {
                        ph = ph - 2.0f;

                        const float d = 1.0f + ph;
                        const float f_rcp = 1.0f / f;
                        const float x = d * f_rcp;
                        wrap = { f_rcp, stepCorrection1 (x), stepCorrection2 (x) };

                        const float y_saw = ph + v.oscillatorA_Saw_AA2 + wrap.correction1;
                        v.oscillatorA_Saw_AA2 = wrap.correction2;
                        OscA_Saw = state.oscillatorA_SawA * y_saw;
                    }
                    else
                    {
                        const float y_saw = ph + v.oscillatorA_Saw_AA2;
                        v.oscillatorA_Saw_AA2 = 0;
                        OscA_Saw = state.oscillatorA_SawA * y_saw;
                    }

                    if (state.oscillatorA_PulseActive)
                    {
                        const float ph_pwm = clamp (mod.pulseWidthA + state.oscillatorA_PW_PM * PolyMod, -1.0f, 1.0f);

                        OscA_Pulse = state.mixerOscillatorA * renderPulse (ph, ph_pwm, f, wrapped ? &wrap : nullptr,
                                                                           v.oscillatorA_Pulse_AA2);
                    }
                }
            }

            v.oscillatorA_Phase = ph;
        }

        // EQ
        // boost high frequencies (up to +3.1dB at Nyquist) to compensate loss due to antialiasing
        const float EQ_In = OscA_Pulse + OscA_Saw + state.mixerOscillatorB * (OscB_Saw - OscB_PulseTri);
        const float EQ_Out = EQ_In + -0.171f * v.eqLast;
        v.eqLast = EQ_In;

        // Mixer
        const float Mixer = EQ_Out + state.mixerNoise * mod.noise + inputSignal;

        // Filter
        v.filter_F_Smooth += v.filter_F_Inc;
        const float FM = state.filter_PM * PolyMod * v.filter_F_Smooth;

        return filterVersion == 0 ? renderOldFilter (v, Mixer, FM)
                                  : renderNewFilter (v, Mixer, FM);
    }

    //==============================================================================
    // The Pro-53's filter: a pair of state-variable filters in series, with a
    // saturator in the feedback path.
    float renderOldFilter (VoiceState& v, float Mixer, float FM)
    {
        float FN = (v.filter_F_Smooth + FM) * tauFilter;
        float F = FN;

        if (F < 0.0f)
        {
            FN = 0.0f;
            F = 0.0f;
        }
        else if (F > state.filterN_FMAX)
        {
            FN = state.filterN_FMAX;

            if (F > state.filter_FMAX)
                F = state.filter_FMAX;
        }

        const float In = Mixer - v.saturator_Out;
        float hpf = In - (v.filter_BPF * state.filter_D + v.filter_LPF);
        v.filter_BPF += hpf * F;
        v.filter_LPF += v.filter_BPF * F + 1.0e-7f;

        const float notch = (state.filter_HPF ? hpf : v.filter_LPF) - v.filterN_BPF;
        hpf = notch * state.filterN_D - v.filterN_LPF;
        v.filterN_BPF += hpf * FN;
        v.filterN_LPF += v.filterN_BPF * FN;

        const float satIn = state.filter_HPF ? hpf : v.filterN_LPF;

        // Saturator
        if (satIn > 0.0f)
            v.saturator_Out = satIn < 4.0f ? satIn * (satIn * (-0.125f * 0.13335f) + 0.13335f)
                                           : 0.13335f * 2.0f;
        else
            v.saturator_Out = satIn > -4.0f ? satIn * (satIn * (0.125f * 0.13335f) + 0.13335f)
                                            : -0.13335f * 2.0f;

        // Filt1
        const float filter1_Hpf = satIn - v.filter1HP_LPF;
        v.filter1HP_LPF += filter1_Hpf * state.filter1HPF;

        return -(filter1_Hpf * v.envelopeA_Out);
    }

    // The "new" filter: a four-pole ladder with a tanh in each stage (optionally
    // anti-aliased), which the HPF switch turns into a band-pass.
    float renderNewFilter (VoiceState& v, float Mixer, float FM)
    {
        const float F = max (1.0f, v.filter_F_Smooth + FM);
        const float g = fasttanf (min (1.5f, F * piOverSampleRate));
        const float norm = 1.0f / (1.0f + g);
        const float gnorm = g * norm;
        const float gnorm2 = gnorm * gnorm;
        const float GF = gnorm2 * gnorm2;
        const float G = GF * state.filter_K;

        const float scale = 2.0f / 3.0f;
        float x = Mixer * scale + 1e-5f;
        x *= 1 + state.filter_K * 0.1f;

        float s2, s3; // the outputs of the middle two stages

        if (filterVersion == 2) // anti-aliased
        {
            const float x_ = v.filter_X;
            v.filter_X = x;
            x = (x + x_) * 0.5f;

            const float g2 = g + g, g2norm = g2 * norm;

            float S = v.filter_S0 - g2norm * v.filter_S0;
            S = ((S + v.filter_X_1) * g + v.filter_S1) * norm;
            S = ((S + v.filter_X_2) * g + v.filter_S2) * norm;
            S = ((S + v.filter_X_3) * g + v.filter_S3) * norm;
            S = (GF * (x + x) + S - G * v.filter_X_4) / (1 + G);
            S = tanhaa (S, v.filter_X_4, v.filter_F0_4);
            x = x - S * state.filter_K;

            float u = (x - v.filter_S0) * g2norm;
            v.filter_S0 = v.filter_S0 + u;
            x = v.filter_S0;

            u = tanhaa ((x - v.filter_S1 - v.filter_X_1 * g) * norm, v.filter_X_1, v.filter_F0_1) * g2;
            v.filter_S1 = v.filter_S1 + u;
            x = v.filter_S1;
            s2 = x;

            u = tanhaa ((x - v.filter_S2 - v.filter_X_2 * g) * norm, v.filter_X_2, v.filter_F0_2) * g2;
            v.filter_S2 = v.filter_S2 + u;
            x = v.filter_S2;
            s3 = x;

            u = tanhaa ((x - v.filter_S3 - v.filter_X_3 * g) * norm, v.filter_X_3, v.filter_F0_3) * g2;
            v.filter_S3 = v.filter_S3 + u;
            x = v.filter_S3;
        }
        else
        {
            float S = gnorm * (0 - v.filter_S0) + v.filter_S0;
            S = gnorm * (S - v.filter_S1) + v.filter_S1;
            S = gnorm * (S - v.filter_S2) + v.filter_S2;
            S = gnorm * (S - v.filter_S3) + v.filter_S3;
            S = fasttanhf (S);
            S *= state.filter_K;
            x = (x - S) / (1 + G);

            float u = fasttanhf (x - v.filter_S0) * gnorm;
            x = v.filter_S0 + u;
            v.filter_S0 = u + x;

            u = fasttanhf (x - v.filter_S1) * gnorm;
            x = v.filter_S1 + u;
            v.filter_S1 = u + x;
            s2 = x;

            u = fasttanhf (x - v.filter_S2) * gnorm;
            x = v.filter_S2 + u;
            v.filter_S2 = u + x;
            s3 = x;

            u = fasttanhf (x - v.filter_S3) * gnorm;
            x = v.filter_S3 + u;
            v.filter_S3 = u + x;
        }

        if (state.filter_HPF) // band-pass
            x = s2 - 2 * s3 + x;

        x *= 2 / scale;
        return x * v.envelopeA_Out;
    }

    //==============================================================================
    // The delay: four modulated taps on one delay line, that can run from a
    // comb filter through a chorus to an echo.
    void renderDelay (float voiceOutput, float& delayL, float& delayR)
    {
        // The input is the voices plus the feedback, through a high-pass and a low-pass
        float Hpf = (voiceOutput + state.delaySum * state.delayFeedbackSmooth) - state.delay_HPF_LPF;
        state.delay_HPF_LPF += Hpf * state.delay_HPF_F + 1e-12f;

        Hpf = Hpf - state.delay_LPF_LPF;
        state.delay_LPF_LPF += Hpf * state.delay_LPF_F;

        delayBuffer[state.delayBufferIn & delayMask] = state.delay_LPF_LPF;

        float taps[4];

        for (int i = 0; i < 4; ++i)
        {
            state.delayTime_Mod[i] += state.delayTime_Inc[i];

            const float prec = state.delayTime_Mod[i];
            const int trunc = int (prec);
            const float lo = delayBuffer[(state.delayBufferIn - trunc) & delayMask];
            const float hi = delayBuffer[(state.delayBufferIn - trunc - 1) & delayMask];

            taps[i] = lo + (prec - trunc) * (hi - lo);
        }

        state.delayBufferIn = (state.delayBufferIn + 1) & delayMask;
        state.delaySum = taps[0] + taps[1] + taps[2] + taps[3];

        // 1: half-left, 2: half-right; 3; full-left, 4: full-right
        delayL = state.delayWetSmooth * (0.75f * taps[0] + 0.25f * taps[1] + taps[2]);
        delayR = state.delayWetSmooth * (0.25f * taps[0] + 0.75f * taps[1] + taps[3]);
    }

    //==============================================================================
    // The A-440 tuning tone: a filtered pulse wave
    float renderTestTone()
    {
        const float Inc = state.A440Freq;
        state.A440Phase += Inc;
        float A440_Pulse;

        if (state.A440PulseSign)
        {
            const float diff = 1.0f - state.A440Phase;

            if (diff > 0.5f * Inc)
            {
                A440_Pulse = A440Pulse;
            }
            else
            {
                const float diffInc = 0.5f + diff / Inc;
                A440_Pulse = A440Pulse * diffInc;
                state.A440Phase -= 1.0f;
                state.A440PulseSign = false;
            }
        }
        else
        {
            const float diff = 0.5f - state.A440Phase;

            if (diff > 0.5f * Inc)
            {
                A440_Pulse = 0.0f;
            }
            else
            {
                A440_Pulse = A440Pulse * (0.5f - (diff / Inc));
                state.A440PulseSign = true;
            }
        }

        const float hpf = (A440_Pulse - 0.5f * A440Pulse) - state.A440FilterLPF;
        state.A440FilterLPF += hpf * state.A440FilterFreq;
        return state.A440FilterLPF;
    }

    //==============================================================================
    // Exponentials, from a table of powers of two in steps of a 60th of an octave

    void initExponentLUT()
    {
        exponentLUT[0] = float (std::pow (2.0, -20.0));

        for (int i = 1; i < 2401; ++i)
        {
            const float value = std::pow (2.0f, (float (i) - 1200.0f) / 60.0f);
            exponentLUT[i] = value;
            exponentLUTSlope[i - 1] = value - exponentLUT[i - 1];
        }
    }

    float fastExp (float v) const
    {
        const int trunc = int (v);

        if (trunc > 2399)
            return exponentLUT[2400];

        if (trunc < 0)
            return 0;

        return exponentLUT[trunc] + (v - trunc) * exponentLUTSlope[trunc];
    }

    float pitchToFreq (float v) const        { return fastExp (v * 5.0f + 1381.88158f); }         // a note number to Hz
    float timeToIncrement (float v) const    { return fastExp (1797.94705708f - v * 9.96578428466f); } // dB to 1000 / gain
    float exp_dB (float v) const             { return fastExp (v * 9.96578428466f + 1200.0f); }   // dB to gain
    float exp_E (float v) const              { return fastExp (v * 86.56170245334f + 1200.0f); }  // e^v
};

//==============================================================================
Engine::Engine()  : impl (std::make_unique<Impl>()) {}
Engine::~Engine() = default;

void Engine::prepare (double sampleRate)                { impl->prepare (sampleRate); }
void Engine::setParameter (int param, float value)      { impl->setParameter (param, value, false); }
float Engine::getParameter (int param) const            { return impl->parameters[param]; }

void Engine::setParameters (const float* values)
{
    for (auto p : paramOrder)
        impl->setParameter (p, values[p], true);
}

// These do the same conversions as the MIDI parser which the Cmajor patch puts
// in front of its engine (an MPE one, that deals in notes with float velocities
// and pitch bends in semitones), followed by the engine's own event handlers.
void Engine::noteOn (int note, int velocity)
{
    if (velocity <= 0)
        return noteOff (note);

    const float floatVelocity = float (velocity) * (1.0f / 127.0f);
    impl->startNote (note, int (floatVelocity * 128.0f));
}

void Engine::noteOff (int note)
{
    impl->stopNote (note);
}

void Engine::setPitchWheel (int value14Bit)
{
    const float bendSemitones = (48.0f / 8192.0f) * float (value14Bit - 8192);
    impl->setPitchBend (0.5f + 0.5f * (bendSemitones / 48.0f));
}

void Engine::setController (int controller, int value)
{
    const float floatValue = float (value) * (1.0f / 127.0f);

    switch (controller)
    {
        case 1:     impl->setModWheel (floatValue); break;
        case 64:    impl->setSustain (floatValue); break;
        case 66:    impl->setSostenuto (floatValue); break;
        case 120:   impl->allSoundOff(); break;
        case 123:   impl->allNotesOff(); break;
        default:    break;
    }
}

void Engine::setTempo (double bpm)          { impl->setTempo (bpm); }

float Engine::getPitchBend() const          { return impl->state.pitchBend * 0.25f + 0.5f; }
float Engine::getModWheel() const           { return impl->state.modulationWheel * 0.2f; }

void Engine::process (const float* externalIn, float* left, float* right, int numFrames)
{
    for (int i = 0; i < numFrames; ++i)
        impl->renderFrame (externalIn != nullptr ? externalIn[i] : 0.0f, left[i], right[i]);
}

} // namespace pro54
