#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
static juce::NormalisableRange<float> getRangeFor (const pro54::ParamInfo& info)
{
    const float interval = info.type == pro54::ParamInfo::knob ? 0.0f
                                                               : (info.type == pro54::ParamInfo::filterVersion ? 0.5f : 1.0f);

    return { info.getMinValue(), info.getMaxValue(), interval };
}

static const juce::StringArray filterVersionNames { "Old", "New", "New AA" };

Pro54Parameter::Pro54Parameter (const pro54::ParamInfo& i)
    : RangedAudioParameter (juce::ParameterID (i.id, 1), i.name),
      info (i),
      range (getRangeFor (i)),
      value (i.defaultValue)
{
}

// Sets the value in the engine's terms. This is how to get it there exactly:
// setValueNotifyingHost() takes it by way of the host's 0 to 1 range.
void Pro54Parameter::set (float newValue)
{
    value.store (range.snapToLegalValue (newValue), std::memory_order_relaxed);
    sendValueChangedMessageToListeners (getValue());
}

float Pro54Parameter::getValue() const              { return range.convertTo0to1 (get()); }
void Pro54Parameter::setValue (float newValue)      { value.store (convertFrom0to1 (newValue), std::memory_order_relaxed); }
float Pro54Parameter::getDefaultValue() const       { return range.convertTo0to1 (info.defaultValue); }

bool Pro54Parameter::isDiscrete() const             { return info.type != pro54::ParamInfo::knob; }
bool Pro54Parameter::isBoolean() const              { return info.type == pro54::ParamInfo::toggle; }

int Pro54Parameter::getNumSteps() const
{
    return isDiscrete() ? juce::roundToInt ((range.end - range.start) / range.interval) + 1
                        : RangedAudioParameter::getNumSteps();
}

const juce::NormalisableRange<float>& Pro54Parameter::getNormalisableRange() const      { return range; }

juce::String Pro54Parameter::getText (float normalisedValue, int) const
{
    const float v = convertFrom0to1 (normalisedValue);

    switch (info.type)
    {
        case pro54::ParamInfo::knob:            return juce::String (v, 1);
        case pro54::ParamInfo::toggle:          return v > 0.5f ? "On" : "Off";
        case pro54::ParamInfo::filterVersion:   return filterVersionNames[juce::roundToInt (v * 2.0f)];
        case pro54::ParamInfo::voiceCount:      return juce::String (juce::roundToInt (v));
    }

    return {};
}

float Pro54Parameter::getValueForText (const juce::String& text) const
{
    const auto trimmed = text.trim();
    float v = trimmed.getFloatValue();

    if (info.type == pro54::ParamInfo::toggle && trimmed.equalsIgnoreCase ("on"))
        v = 1.0f;
    else if (info.type == pro54::ParamInfo::filterVersion && filterVersionNames.contains (trimmed, true))
        v = 0.5f * (float) filterVersionNames.indexOf (trimmed, true);

    return range.convertTo0to1 (range.snapToLegalValue (v));
}

//==============================================================================
Pro54AudioProcessor::Pro54AudioProcessor()
    : AudioProcessor (BusesProperties()
                        // The signal for the mixer's "ext in" knob to feed into the filter. It's
                        // off unless the host is asked to connect something to it.
                        .withInput ("External In", juce::AudioChannelSet::mono(), false)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    for (int i = 0; i < pro54::numParams; ++i)
    {
        auto& info = pro54::paramInfo[i];
        auto param = std::make_unique<Pro54Parameter> (info);

        parameters[(size_t) i] = param.get();

        if (info.midiController >= 0)
            controllerParameters[(size_t) info.midiController] = param.get();

        addParameter (param.release());
    }
}

Pro54AudioProcessor::~Pro54AudioProcessor() = default;

//==============================================================================
void Pro54AudioProcessor::prepareToPlay (double sampleRate, int)
{
    // Hand over the parameters as they stand before starting afresh, so that the
    // engine sets itself up from them rather than treating them as changes.
    updateEngineParameters();
    engine.prepare (sampleRate);

    keyboardState.reset();
}

void Pro54AudioProcessor::releaseResources() {}

// The host wants silence: nothing left sounding, and nothing left in the delay.
void Pro54AudioProcessor::reset()
{
    engine.setController (120, 0); // "all sound off"
    keyboardState.reset();
}

bool Pro54AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto input = layouts.getMainInputChannelSet();

    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
            && (input.isDisabled() || input == juce::AudioChannelSet::mono() || input == juce::AudioChannelSet::stereo());
}

//==============================================================================
void Pro54AudioProcessor::updateEngineParameters()
{
    float values[pro54::numParams];

    for (int i = 0; i < pro54::numParams; ++i)
        values[i] = parameters[(size_t) i]->get();

    engine.setParameters (values);
}

void Pro54AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const int numFrames = buffer.getNumSamples();

    if (auto* playHead = getPlayHead())
        if (const auto position = playHead->getPosition())
            if (const auto bpm = position->getBpm())
                engine.setTempo (*bpm);

    updateEngineParameters();

    keyboardState.processNextMidiBuffer (midiMessages, 0, numFrames, true);

    // The external input, if there is one, arrives in the channels that the output goes
    // out through. The engine can cope with that, as it reads each frame before writing it.
    auto input = getBusBuffer (buffer, true, 0);
    auto output = getBusBuffer (buffer, false, 0);

    if (output.getNumChannels() < 2)
        return;

    if (input.getNumChannels() > 1) // mix a stereo input down to mono
    {
        auto* mixed = input.getWritePointer (0);
        auto* other = input.getReadPointer (1);

        for (int i = 0; i < numFrames; ++i)
            mixed[i] = 0.5f * (mixed[i] + other[i]);
    }

    const float* externalIn = input.getNumChannels() > 0 ? input.getReadPointer (0) : nullptr;
    float* left = output.getWritePointer (0);
    float* right = output.getWritePointer (1);

    // Render up to each MIDI message in turn
    int frame = 0;

    auto renderUpTo = [&] (int end)
    {
        if (end > frame)
        {
            engine.process (externalIn != nullptr ? externalIn + frame : nullptr, left + frame, right + frame, end - frame);
            frame = end;
        }
    };

    for (const auto metadata : midiMessages)
    {
        renderUpTo (juce::jlimit (0, numFrames, metadata.samplePosition));
        handleMidiMessage (metadata.getMessage());
    }

    renderUpTo (numFrames);

    pitchWheelPosition.store (engine.getPitchBend(), std::memory_order_relaxed);
    modWheelPosition.store (engine.getModWheel(), std::memory_order_relaxed);
}

void Pro54AudioProcessor::handleMidiMessage (const juce::MidiMessage& message)
{
    midiActivityCount.fetch_add (1, std::memory_order_relaxed);

    if (message.isNoteOn())
    {
        engine.noteOn (message.getNoteNumber(), message.getVelocity());
    }
    else if (message.isNoteOff())
    {
        engine.noteOff (message.getNoteNumber());
    }
    else if (message.isPitchWheel())
    {
        engine.setPitchWheel (message.getPitchWheelValue());
    }
    else if (message.isController())
    {
        const int number = message.getControllerNumber();
        const int value = message.getControllerValue();

        if (number == 0)
            bankSelect = value;

        // The engine deals with the wheel and the pedals itself..
        engine.setController (number, value);

        // ..and most of the panel has a controller number of its own
        if (auto* param = controllerParameters[(size_t) number])
        {
            param->setValueNotifyingHost ((float) value / 127.0f);
            updateEngineParameters();
        }
    }
    else if (message.isProgramChange())
    {
        setCurrentProgram ((bankSelect * 128 + message.getProgramChangeNumber()) % pro54::numPresets);
        updateEngineParameters();
    }
}

//==============================================================================
bool Pro54AudioProcessor::hasEditor() const                         { return true; }
juce::AudioProcessorEditor* Pro54AudioProcessor::createEditor()     { return new Pro54AudioProcessorEditor (*this); }

const juce::String Pro54AudioProcessor::getName() const             { return JucePlugin_Name; }
bool Pro54AudioProcessor::acceptsMidi() const                       { return true; }
bool Pro54AudioProcessor::producesMidi() const                      { return false; }
bool Pro54AudioProcessor::isMidiEffect() const                      { return false; }
double Pro54AudioProcessor::getTailLengthSeconds() const            { return 0.0; }

//==============================================================================
int Pro54AudioProcessor::getNumPrograms()           { return pro54::numPresets; }
int Pro54AudioProcessor::getCurrentProgram()        { return currentProgram.load(); }

// This can be called on the audio thread as well as the message thread: the host is
// allowed to automate the choice of program, and a MIDI program change lands here too.
void Pro54AudioProcessor::setCurrentProgram (int index)
{
    index = juce::jlimit (0, pro54::numPresets - 1, index);
    currentProgram.store (index);

    auto& preset = pro54::presets[index];

    for (int i = 0; i < pro54::numPresetParams; ++i)
        parameters[(size_t) pro54::paramOrder[i]]->set (preset.getValue (i));
}

void Pro54AudioProcessor::selectProgram (int index)
{
    setCurrentProgram (index);
    updateHostDisplay (ChangeDetails().withProgramChanged (true));
}

const juce::String Pro54AudioProcessor::getProgramName (int index)
{
    if (! juce::isPositiveAndBelow (index, pro54::numPresets))
        return {};

    return juce::String (pro54::getPresetNumber (index)) + ": " + pro54::presets[index].name;
}

void Pro54AudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
// The state is the parameters' values, in their own units and under their own
// IDs, plus the number of the program that was last chosen (and the editor's size).
void Pro54AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::XmlElement xml ("Pro54");
    xml.setAttribute ("program", getCurrentProgram());
    xml.setAttribute ("editorWidth", editorWidth);

    for (int i = 0; i < pro54::numParams; ++i)
        xml.setAttribute (pro54::paramInfo[i].id, (double) parameters[(size_t) i]->get());

    copyXmlToBinary (xml, destData);
}

void Pro54AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    const auto xml = getXmlFromBinary (data, sizeInBytes);

    if (xml == nullptr || ! xml->hasTagName ("Pro54"))
        return;

    // (not setCurrentProgram(), which would go and load the preset)
    currentProgram.store (juce::jlimit (0, pro54::numPresets - 1, xml->getIntAttribute ("program")));
    editorWidth = juce::jlimit (762, 3048, xml->getIntAttribute ("editorWidth", editorWidth));

    for (int i = 0; i < pro54::numParams; ++i)
    {
        const juce::String id (pro54::paramInfo[i].id);

        if (xml->hasAttribute (id))
            parameters[(size_t) i]->set ((float) xml->getDoubleAttribute (id));
    }
}

//==============================================================================
// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Pro54AudioProcessor();
}
