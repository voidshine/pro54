#include "PluginEditor.h"

#include <bitset>

namespace
{

using pro54::Param;

//==============================================================================
// Everything here is in the coordinates of the original panel: the positions
// of the controls are the ones in the stylesheet of the Cmajor patch's web view.
constexpr int panelWidth = 762, panelHeight = 358;
constexpr int knobSize = 25, switchWidth = 13, switchHeight = 19;

struct ControlPosition
{
    Param param;
    int x, y;
};

const ControlPosition knobPositions[] =
{
    { pro54::PolyModFilterEnv, 23, 23 },        { pro54::PolyModOscB, 61, 23 },
    { pro54::OscAFreq, 168, 23 },               { pro54::OscAPW, 244, 23 },
    { pro54::MixerOscALevel, 313, 23 },         { pro54::MixerOscBLevel, 349, 23 },
    { pro54::MixerNoiseLevel, 385, 23 },        { pro54::ExternalInputLevel, 385, 69 },
    { pro54::FilterCutoff, 432, 23 },           { pro54::FilterResonance, 468, 23 },
    { pro54::FilterEnvAmt, 504, 23 },           { pro54::FilterKeyboardTracking, 540, 23 },
    { pro54::FilterAttack, 432, 69 },           { pro54::FilterDecay, 468, 69 },
    { pro54::FilterSustain, 504, 69 },          { pro54::FilterRelease, 540, 69 },
    { pro54::DelayTime, 607, 23 },              { pro54::DelaySpread, 643, 23 },
    { pro54::DelayDepth, 679, 23 },             { pro54::DelayRate, 715, 23 },
    { pro54::DelayFeedback, 607, 69 },          { pro54::DelayHiCut, 643, 69 },
    { pro54::DelayLoCut, 679, 69 },             { pro54::DelayWet, 643, 115 },
    { pro54::LfoFrequency, 61, 69 },
    { pro54::OscBFreq, 168, 69 },               { pro54::OscBFreqFine, 206, 69 },
    { pro54::OscBPWAmount, 302, 69 },
    { pro54::WheelModulationLfoNoise, 23, 115 },
    { pro54::Glide, 168, 115 },                 { pro54::Analog, 302, 115 },
    { pro54::AmplifierAttack, 432, 115 },       { pro54::AmplifierDecay, 467, 115 },
    { pro54::AmplifierSustain, 504, 115 },      { pro54::AmplifierRelease, 540, 115 },
    { pro54::MasterTune, 607, 161 },            { pro54::Volume, 679, 161 }
};

const ControlPosition switchPositions[] =
{
    { pro54::PolyModFreqA, 95, 26 },            { pro54::PolyModPWA, 115, 26 },
    { pro54::PolyModFilt, 135, 26 },
    { pro54::OscASaw, 202, 26 },                { pro54::OscAPulse, 222, 26 },
    { pro54::OscASync, 281, 26 },
    { pro54::FilterHPF, 574, 26 },              { pro54::FilterInvertEnv, 574, 72 },
    { pro54::Repeat, 21, 72 },                  { pro54::LfoMidiSync, 41, 72 },
    { pro54::LfoShapeSaw, 95, 72 },             { pro54::LfoShapeTri, 115, 72 },
    { pro54::LfoShapePulse, 135, 72 },
    { pro54::OscBShapeSaw, 240, 72 },           { pro54::OscBShapeTri, 260, 72 },
    { pro54::OscBShapePulse, 280, 72 },
    { pro54::OscBSubOsc, 336, 72 },             { pro54::OscBKKeyboardTracking, 356, 72 },
    { pro54::WheelModulationFreqOscA, 55, 118 }, { pro54::WheelModulationFreqOscB, 75, 118 },
    { pro54::WheelModulationPWA, 95, 118 },     { pro54::WheelModulationPWB, 115, 118 },
    { pro54::WheelModulationFilter, 135, 118 },
    { pro54::Unison, 212, 118 },
    { pro54::Velocity, 346, 118 },              { pro54::Release, 386, 118 },
    { pro54::Drone, 574, 118 },
    { pro54::DelayINV, 721, 72 },               { pro54::DelayON, 613, 118 },
    { pro54::DelaySync, 685, 118 },             { pro54::DelayMidi, 721, 118 },
    { pro54::TestTone, 649, 164 }
};

// The outlined groups that the panel is divided into
struct Section
{
    const char* title;
    int x, y, width, height;
};

const Section sections[] =
{
    { "POLY-MOD",     15,  17,  139, 39 },
    { "OSCILLATOR A", 161, 17,  138, 39 },
    { "MIXER",        306, 17,  112, 39 },
    { "FILTER",       424, 17,  169, 85 },
    { "DELAY EFFECT", 599, 17,  149, 131 },
    { "LFO",          15,  63,  139, 39 },
    { "OSCILLATOR B", 161, 63,  213, 39 },
    { "WHEEL-MOD",    15,  109, 139, 39 },
    { "AMPLIFIER",    424, 109, 169, 39 },
    { "FILTER",       15,  155, 65,  39 },
    { "PROGRAMMER",   194, 155, 400, 39 }
};

// The text under (or over) the controls. Each is centred on its position; one
// with a non-zero ruleLeft has a line drawn out on either side of it, to there
// and to ruleRight.
struct Label
{
    const char* text;
    float x, y;
    float ruleLeft = 0.0f, ruleRight = 0.0f;
};

const float row1 = 52.0f, row2 = 98.0f, row3 = 144.0f, row4 = 190.0f;       // under each row of controls
const float over1 = 22.0f, over2 = 68.0f, over3 = 114.0f, over4 = 160.0f;   // and above them

const Label labels[] =
{
    { "FILTER ENV", 35.5f, row1 },      { "OSC B", 73.5f, row1 },       { "DESTINATION", 121.5f, row1, 94.0f, 149.0f },
    { "FREQ A", 101.5f, over1 },        { "PW A", 121.5f, over1 },      { "FILT", 141.5f, over1 },
    { "FREQ", 180.5f, row1 },           { "SHAPE", 218.5f, row1 },      { "PW", 256.5f, row1 },         { "SYNC", 287.5f, row1 },
    { "OSC A", 325.5f, row1 },          { "OSC B", 361.5f, row1 },      { "NOISE", 397.5f, row1 },
    { "CUTOFF", 444.5f, row1 },         { "RESO", 480.5f, row1 },       { "ENV AMT", 516.5f, row1 },    { "KBD", 552.5f, row1 },
    { "TIME", 619.5f, row1 },           { "SPREAD", 655.5f, row1 },     { "DEPTH", 691.5f, row1 },      { "RATE", 727.5f, row1 },

    { "ENV", 27.5f, over2 },            { "TRIG", 27.5f, row2 },        { "MIDI", 47.5f, row2 },        { "FREQ", 73.5f, row2 },
    { "SHAPE", 121.5f, row2, 94.0f, 149.0f },
    { "FREQ", 180.5f, row2 },           { "FINE", 218.5f, row2 },       { "SHAPE", 266.5f, row2 },      { "PW", 314.5f, row2 },
    { "LOFRQ", 342.0f, row2 },          { "KBD", 363.0f, row2 },        { "EXT IN", 397.5f, row2 },
    { "ATTACK", 444.5f, row2 },         { "DECAY", 480.5f, row2 },      { "SUSTAIN", 516.5f, row2 },    { "RELEASE", 552.5f, row2 },
    { "INV", 580.5f, row2 },
    { "FDBACK", 619.5f, row2 },         { "LO CUT", 655.5f, row2 },     { "HI CUT", 691.5f, row2 },     { "INV", 727.5f, row2 },

    { "LFO NOISE", 37.0f, row3 },       { "DESTINATION", 103.5f, row3, 60.0f, 149.0f },
    { "FRQ A", 61.5f, over3 },          { "B", 81.5f, over3 },          { "PW A", 101.5f, over3 },      { "B", 121.5f, over3 },
    { "FILT", 141.5f, over3 },
    { "GLIDE", 180.5f, row3 },          { "UNISON", 218.5f, row3 },     { "VOICES", 260.5f, row3 },     { "ANALOG", 314.5f, row3 },
    { "VEL", 352.5f, row3 },            { "RELEASE", 392.5f, row3 },
    { "ATTACK", 444.5f, row3 },         { "DECAY", 479.5f, row3 },      { "SUSTAIN", 516.5f, row3 },    { "RELEASE", 552.5f, row3 },
    { "HOLD", 580.5f, row3 },
    { "ON", 619.5f, row3 },             { "WET", 655.5f, row3 },        { "SYNC", 691.5f, row3 },       { "MIDI", 727.5f, row3 },

    { "FILE", 208.5f, row4 },           { "BANK", 228.5f, row4 },       { "PROGRAM", 260.5f, row4 },
    { "PROGRAM SELECT", 362.5f, row4, 285.0f, 440.0f },                 { "PROGRAM NAME", 516.5f, row4 },
    { "1", 292.5f, over4 },             { "2", 312.5f, over4 },         { "3", 332.5f, over4 },         { "4", 352.5f, over4 },
    { "5", 372.5f, over4 },             { "6", 392.5f, over4 },         { "7", 412.5f, over4 },         { "8", 432.5f, over4 },
    { "TUNE", 619.5f, row4 },           { "A-440", 655.5f, row4 },      { "VOLUME", 691.5f, row4 },     { "MIDI", 727.5f, row4 },

    { "PITCH", 32.0f, 335.5f },         { "MOD", 60.0f, 335.5f }
};

// The little pictures of waveforms over the shape switches
enum class Wave { saw, triangle, pulse };

struct WaveIcon
{
    Wave wave;
    float x, y;
};

const WaveIcon waveIcons[] =
{
    { Wave::saw, 208.5f, over1 },   { Wave::pulse, 228.5f, over1 },
    { Wave::saw, 101.5f, over2 },   { Wave::triangle, 121.5f, over2 },  { Wave::pulse, 141.5f, over2 },
    { Wave::saw, 246.5f, over2 },   { Wave::triangle, 266.5f, over2 },  { Wave::pulse, 286.5f, over2 }
};

//==============================================================================
namespace colours
{
    const juce::Colour panel        { 0xff3a3a3c };
    const juce::Colour outline      { 0xff8e8e90 };
    const juce::Colour text         { 0xffd6d6d6 };
    const juce::Colour redLed       { 0xffff3a26 };
    const juce::Colour blueLed      { 0xff35a6ff };
    const juce::Colour displayBack  { 0xff240000 };
    const juce::Colour displayOn    { 0xffff2d1c };
    const juce::Colour displayOff   { 0xff440807 };
}

juce::Font getLabelFont (float height = 6.0f)
{
    return juce::FontOptions (height, juce::Font::bold);
}

void drawLed (juce::Graphics& g, juce::Point<float> centre, float radius, bool on, juce::Colour colour)
{
    if (on)
    {
        const float glow = radius * 2.8f;
        g.setGradientFill (juce::ColourGradient (colour.withAlpha (0.6f), centre.x, centre.y,
                                                 colour.withAlpha (0.0f), centre.x + glow, centre.y, true));
        g.fillEllipse (juce::Rectangle<float> (glow * 2.0f, glow * 2.0f).withCentre (centre));
    }

    g.setColour (on ? colour : colour.withMultipliedBrightness (0.3f));
    g.fillEllipse (juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (centre));

    if (on)
    {
        g.setColour (juce::Colours::white.withAlpha (0.55f));
        g.fillEllipse (juce::Rectangle<float> (radius * 0.8f, radius * 0.8f).withCentre (centre));
    }
}

// The window of one of the red LED displays
void drawDisplayWindow (juce::Graphics& g, juce::Rectangle<float> area)
{
    g.setColour (colours::displayBack);
    g.fillRect (area);
    g.setColour (juce::Colours::black.withAlpha (0.7f));
    g.drawRect (area, 0.6f);
}

void drawSevenSegmentDigit (juce::Graphics& g, juce::Rectangle<float> area, int digit)
{
    // which of the segments a-g (bits 0-6) are lit for each digit
    static constexpr juce::uint8 lit[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f };

    const float x = area.getX(), y = area.getY(), w = area.getWidth(), h = area.getHeight();
    const float t = w * 0.2f, mid = y + (h - t) * 0.5f, side = (h - 3.0f * t) * 0.5f;

    const juce::Rectangle<float> segments[] =
    {
        { x + t, y, w - 2.0f * t, t },              // a: top
        { x + w - t, y + t, t, side },              // b: top right
        { x + w - t, mid + t, t, side },            // c: bottom right
        { x + t, y + h - t, w - 2.0f * t, t },      // d: bottom
        { x, mid + t, t, side },                    // e: bottom left
        { x, y + t, t, side },                      // f: top left
        { x + t, mid, w - 2.0f * t, t }             // g: middle
    };

    for (int i = 0; i < 7; ++i)
    {
        g.setColour ((lit[digit % 10] & (1 << i)) != 0 ? colours::displayOn : colours::displayOff);
        g.fillRect (segments[i].reduced (0.12f));
    }
}

//==============================================================================
// The parts of the panel that never change: the woodwork, the outlines and the lettering.
class Backdrop final : public juce::Component
{
public:
    Backdrop()
    {
        setOpaque (true);
        setInterceptsMouseClicks (false, false);
        setBufferedToImage (true);
    }

    // The label under the filter's mode switch depends on which filter is in
    // use: the newer ones have a band-pass mode where the old one has a high-pass.
    void setFilterModeLabel (const juce::String& newLabel)
    {
        if (filterModeLabel != newLabel)
        {
            filterModeLabel = newLabel;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override
    {
        drawWood (g);

        // the control panel
        const juce::Rectangle<float> panelArea (9.0f, 9.0f, 744.0f, 191.0f);
        g.setGradientFill (juce::ColourGradient (colours::panel.brighter (0.06f), 0.0f, panelArea.getY(),
                                                 colours::panel.darker (0.12f), 0.0f, panelArea.getBottom(), false));
        g.fillRect (panelArea);
        g.setColour (juce::Colours::black.withAlpha (0.6f));
        g.drawRect (panelArea, 1.0f);

        // the well that the keyboard sits in, and the wheels' panel beside it
        g.setColour (juce::Colour (0xff101010));
        g.fillRect (9, 246, 744, 106);

        const juce::Rectangle<float> wheelArea (15.0f, 254.0f, 62.0f, 88.0f);
        g.setColour (colours::panel.darker (0.1f));
        g.fillRoundedRectangle (wheelArea, 2.5f);
        g.setColour (colours::outline);
        g.drawRoundedRectangle (wheelArea, 2.5f, 0.6f);

        g.setFont (juce::Font (juce::FontOptions (juce::Font::getDefaultSerifFontName(), 27.0f, juce::Font::bold | juce::Font::italic)));
        g.setColour (juce::Colour (0xff2c1505).withAlpha (0.85f));
        g.drawText ("pro-54", juce::Rectangle<float> (560.0f, 205.0f, 172.0f, 36.0f), juce::Justification::centredRight, false);

        for (auto& section : sections)
            drawSection (g, section);

        g.setFont (getLabelFont());

        for (auto& label : labels)
            drawLabel (g, label);

        drawLabel (g, { filterModeLabel.toRawUTF8(), 580.5f, row1 });

        for (auto& icon : waveIcons)
            drawWaveIcon (g, icon);
    }

private:
    static void drawWood (juce::Graphics& g)
    {
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffb9641c), 0.0f, 0.0f,
                                                 juce::Colour (0xff934a12), 0.0f, (float) panelHeight, false));
        g.fillAll();

        // some grain, which is the same every time
        juce::Random random (54);

        for (int i = 0; i < 260; ++i)
        {
            const float y = random.nextFloat() * (float) panelHeight;
            const float x = random.nextFloat() * (float) panelWidth - 100.0f;
            const float length = 60.0f + random.nextFloat() * 400.0f;

            g.setColour ((random.nextBool() ? juce::Colour (0xff5a2a08) : juce::Colour (0xffe0913a))
                           .withAlpha (0.05f + 0.13f * random.nextFloat()));
            g.drawLine (x, y, x + length, y + random.nextFloat() * 3.0f - 1.5f, 0.4f + random.nextFloat() * 0.9f);
        }

        g.setColour (juce::Colours::black.withAlpha (0.55f));
        g.drawRect (0.0f, 0.0f, (float) panelWidth, (float) panelHeight, 1.0f);
    }

    static void drawSection (juce::Graphics& g, const Section& section)
    {
        const auto area = juce::Rectangle<int> (section.x, section.y, section.width, section.height).toFloat();
        const auto font = getLabelFont (6.6f).withExtraKerningFactor (0.12f);
        const juce::String title (section.title);
        const float titleWidth = juce::GlyphArrangement::getStringWidth (font, title) + 6.0f;

        // leave a gap in the top of the outline for the title to go in
        {
            juce::Graphics::ScopedSaveState state (g);
            g.excludeClipRegion (juce::Rectangle<float> (titleWidth, 8.0f).withCentre ({ area.getCentreX(), area.getY() }).toNearestInt());
            g.setColour (colours::outline);
            g.drawRoundedRectangle (area, 3.0f, 0.7f);
        }

        g.setFont (font);
        g.setColour (colours::text);
        g.drawText (title, juce::Rectangle<float> (titleWidth + 20.0f, 10.0f).withCentre ({ area.getCentreX(), area.getY() - 0.3f }),
                    juce::Justification::centred, false);
    }

    static void drawLabel (juce::Graphics& g, const Label& label)
    {
        const juce::String text (label.text);

        g.setColour (colours::text);
        g.drawText (text, juce::Rectangle<float> (90.0f, 10.0f).withCentre ({ label.x, label.y }), juce::Justification::centred, false);

        if (label.ruleLeft > 0.0f)
        {
            const float halfWidth = juce::GlyphArrangement::getStringWidth (g.getCurrentFont(), text) * 0.5f + 2.5f;

            g.setColour (colours::outline);
            g.fillRect (juce::Rectangle<float> (label.ruleLeft, label.y - 0.3f, label.x - halfWidth - label.ruleLeft, 0.6f));
            g.fillRect (juce::Rectangle<float> (label.x + halfWidth, label.y - 0.3f, label.ruleRight - label.x - halfWidth, 0.6f));
        }
    }

    static void drawWaveIcon (juce::Graphics& g, const WaveIcon& icon)
    {
        const float l = icon.x - 3.5f, r = icon.x + 3.5f, top = icon.y - 2.3f, bottom = icon.y + 2.3f;
        juce::Path p;

        if (icon.wave == Wave::saw)
        {
            p.startNewSubPath (l, bottom);
            p.lineTo (r - 1.0f, top);
            p.lineTo (r - 1.0f, bottom);
            p.lineTo (r, bottom);
        }
        else if (icon.wave == Wave::triangle)
        {
            p.startNewSubPath (l, bottom);
            p.lineTo (icon.x, top);
            p.lineTo (r, bottom);
        }
        else
        {
            p.startNewSubPath (l, bottom);
            p.lineTo (l + 1.5f, bottom);
            p.lineTo (l + 1.5f, top);
            p.lineTo (r - 1.5f, top);
            p.lineTo (r - 1.5f, bottom);
            p.lineTo (r, bottom);
        }

        g.setColour (colours::text);
        g.strokePath (p, juce::PathStrokeType (0.75f, juce::PathStrokeType::mitered));
    }

    juce::String filterModeLabel { "HPF" };
};

//==============================================================================
// Something that shows a parameter and lets it be changed.
class ParamControl : public juce::Component
{
public:
    explicit ParamControl (juce::RangedAudioParameter& p) : param (p) {}

    // Called regularly, to catch up with any changes made by the host
    virtual void refresh()
    {
        const float value = param.getValue();

        if (! juce::exactlyEqual (value, shownValue))
        {
            shownValue = value;
            repaint();
        }
    }

    juce::RangedAudioParameter& param;

    // Called when the control has been used to change the parameter
    std::function<void()> onUserChange;

protected:
    void setValue (float newValue)
    {
        param.setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, newValue));
        refresh();

        if (onUserChange != nullptr)
            onUserChange();
    }

    // sets a value as a single action, rather than as part of a drag
    void setValueAsGesture (float newValue)
    {
        param.beginChangeGesture();
        setValue (newValue);
        param.endChangeGesture();
    }

    float shownValue = -1.0f;
};

//==============================================================================
class Knob final : public ParamControl
{
public:
    Knob (juce::RangedAudioParameter& p, bool isMetal) : ParamControl (p), metal (isMetal) {}

    void paint (juce::Graphics& g) override
    {
        const auto centre = getLocalBounds().toFloat().getCentre();
        const float angle = juce::MathConstants<float>::pi * 1.5f * (param.getValue() - 0.5f); // 270 degrees of travel

        auto pointAt = [centre] (float a, float radius)
        {
            return centre + juce::Point<float> (std::sin (a), -std::cos (a)) * radius;
        };

        g.setColour (colours::text.withAlpha (0.75f));

        for (int i = -3; i <= 3; ++i)
        {
            const float a = juce::MathConstants<float>::pi * 0.25f * (float) i;
            g.drawLine ({ pointAt (a, 10.3f), pointAt (a, 12.3f) }, 0.6f);
        }

        g.setColour (juce::Colours::black.withAlpha (0.4f)); // shadow
        g.fillEllipse (juce::Rectangle<float> (19.0f, 19.0f).withCentre (centre.translated (1.4f, 2.0f)));

        const auto rim = juce::Rectangle<float> (18.4f, 18.4f).withCentre (centre);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffc8c8c8), rim.getX(), rim.getY(),
                                                 juce::Colour (0xff4a4a4a), rim.getRight(), rim.getBottom(), false));
        g.fillEllipse (rim);

        const auto cap = rim.reduced (1.3f);

        if (metal)
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xfff2f2f2), cap.getX(), cap.getY(),
                                                     juce::Colour (0xff7c7c7c), cap.getRight(), cap.getBottom(), false));
        else
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2c2c2e), cap.getX(), cap.getY(),
                                                     juce::Colour (0xff050505), cap.getRight(), cap.getBottom(), false));

        g.fillEllipse (cap);

        g.setColour (metal ? juce::Colour (0xff141414) : juce::Colours::white);
        g.drawLine ({ pointAt (angle, 2.2f), pointAt (angle, 7.9f) }, 1.5f);
    }

    void mouseDown (const juce::MouseEvent&) override
    {
        dragStartValue = param.getValue();
        param.beginChangeGesture();
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        // 100 units of the panel for the full range, or five times that with shift held
        const float distance = (float) -e.getDistanceFromDragStartY() * (e.mods.isShiftDown() ? 0.002f : 0.01f);
        setValue (dragStartValue + distance);
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        param.endChangeGesture();
    }

    void mouseDoubleClick (const juce::MouseEvent&) override
    {
        setValueAsGesture (param.getDefaultValue());
    }

    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override
    {
        setValueAsGesture (param.getValue() + wheel.deltaY * 0.1f);
    }

private:
    const bool metal;
    float dragStartValue = 0.0f;
};

//==============================================================================
// The switches with lights in them: most are for a parameter, and the ones in
// the programmer just do something when they're pressed.
enum class SwitchStyle { dark, grey };

void drawSwitch (juce::Graphics& g, juce::Rectangle<float> area, SwitchStyle style, bool lit)
{
    const auto body = area.reduced (1.0f);
    const auto base = style == SwitchStyle::dark ? juce::Colour (0xff434d58) : juce::Colour (0xffb2b2b2);

    g.setColour (juce::Colours::black);
    g.fillRoundedRectangle (area, 1.5f);

    g.setColour (base.darker (0.25f));
    g.fillRect (body.withHeight (body.getHeight() * 0.5f));
    g.setColour (base);
    g.fillRect (body.withTop (body.getCentreY()));

    g.setColour (juce::Colours::white.withAlpha (0.35f));
    g.fillRect (body.withTop (body.getCentreY()).withHeight (0.6f));

    drawLed (g, { area.getCentreX(), area.getY() + 5.3f }, 1.5f, lit, colours::redLed);
}

class ParamSwitch final : public ParamControl
{
public:
    ParamSwitch (juce::RangedAudioParameter& p, SwitchStyle s) : ParamControl (p), style (s) {}

    void paint (juce::Graphics& g) override
    {
        drawSwitch (g, getLocalBounds().toFloat(), style, param.getValue() > 0.5f);
    }

    void mouseDown (const juce::MouseEvent&) override
    {
        setValueAsGesture (param.getValue() > 0.5f ? 0.0f : 1.0f);
    }

private:
    const SwitchStyle style;
};

class ActionSwitch final : public juce::Component
{
public:
    void setLit (bool shouldBeLit)
    {
        if (lit != shouldBeLit)
        {
            lit = shouldBeLit;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override
    {
        drawSwitch (g, getLocalBounds().toFloat(), SwitchStyle::grey, lit);
    }

    void mouseDown (const juce::MouseEvent&) override
    {
        if (onClick != nullptr)
            onClick();
    }

    std::function<void()> onClick;

private:
    bool lit = false;
};

//==============================================================================
// The pitch and modulation wheels
class Wheel final : public ParamControl
{
public:
    Wheel (juce::RangedAudioParameter& p, std::function<float()> positionGetter, bool springsBackToCentre)
        : ParamControl (p), getPosition (std::move (positionGetter)), springBack (springsBackToCentre)
    {
    }

    void refresh() override
    {
        // The wheel shows where the engine has it, which MIDI may have moved
        const float position = isMouseButtonDown() ? param.getValue() : getPosition();

        if (! juce::exactlyEqual (position, shownValue))
        {
            shownValue = position;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override
    {
        const auto area = getLocalBounds().toFloat();

        g.setColour (juce::Colours::black);
        g.fillRoundedRectangle (area, 1.5f);

        const auto face = area.reduced (1.2f, 1.5f);
        juce::ColourGradient shading (juce::Colour (0xff141414), 0.0f, face.getY(), juce::Colour (0xff141414), 0.0f, face.getBottom(), false);
        shading.addColour (0.5, juce::Colour (0xff8c8c8c));
        g.setGradientFill (shading);
        g.fillRect (face);

        // the notch
        const float y = face.getBottom() - 3.0f - juce::jlimit (0.0f, 1.0f, shownValue) * (face.getHeight() - 6.0f);
        g.setColour (juce::Colours::black.withAlpha (0.8f));
        g.fillRect (face.getX(), y - 2.2f, face.getWidth(), 4.4f);
        g.setColour (juce::Colour (0xffe6e6e6));
        g.fillRect (face.getX(), y - 0.8f, face.getWidth(), 1.6f);
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        param.beginChangeGesture();
        mouseDrag (e);
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        setValue (1.0f - ((float) e.y - 4.0f) / ((float) getHeight() - 8.0f));
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        if (springBack)
            setValue (0.5f);

        param.endChangeGesture();
    }

private:
    std::function<float()> getPosition;
    const bool springBack;
};

//==============================================================================
// The number of voices, which is changed by dragging it up and down
class VoicesDisplay final : public ParamControl
{
public:
    using ParamControl::ParamControl;

    void paint (juce::Graphics& g) override
    {
        const auto area = getLocalBounds().toFloat();
        drawDisplayWindow (g, area);

        const int voices = juce::roundToInt (param.convertFrom0to1 (param.getValue()));
        const auto digits = area.withSizeKeepingCentre (17.0f, 10.0f);

        if (voices >= 10)
            drawSevenSegmentDigit (g, digits.withWidth (7.0f), voices / 10);

        drawSevenSegmentDigit (g, digits.withLeft (digits.getRight() - 7.0f), voices);
    }

    void mouseDown (const juce::MouseEvent&) override
    {
        dragStartValue = param.convertFrom0to1 (param.getValue());
        param.beginChangeGesture();
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        setValue (param.convertTo0to1 (juce::jlimit (1.0f, 32.0f, dragStartValue - (float) (e.getDistanceFromDragStartY() / 4))));
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        param.endChangeGesture();
    }

    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override
    {
        const float voices = param.convertFrom0to1 (param.getValue()) + (wheel.deltaY > 0 ? 1.0f : -1.0f);
        setValueAsGesture (param.convertTo0to1 (juce::jlimit (1.0f, 32.0f, voices)));
    }

private:
    float dragStartValue = 0.0f;
};

//==============================================================================
// The choice of filter: three lights to click on
class FilterVersionSelector final : public ParamControl
{
public:
    using ParamControl::ParamControl;

    int getVersion() const      { return juce::roundToInt (param.getValue() * 2.0f); }

    void paint (juce::Graphics& g) override
    {
        static const char* const names[] = { "OLD", "NEW", "NEW AA" };

        const int version = getVersion();
        g.setFont (getLabelFont());

        for (int i = 0; i < 3; ++i)
        {
            const float y = getRowY (i);

            drawLed (g, { 3.5f, y }, 1.5f, version == i, i == 2 ? colours::blueLed : colours::redLed);

            g.setColour (colours::text);
            g.drawText (names[i], juce::Rectangle<float> (10.5f, y - 5.0f, 40.0f, 10.0f), juce::Justification::centredLeft, false);
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        for (int i = 0; i < 3; ++i)
            if (std::abs ((float) e.y - getRowY (i)) < 4.7f)
                setValueAsGesture ((float) i * 0.5f);
    }

private:
    static float getRowY (int version)      { return 23.0f - 9.3f * (float) version; } // the old one is at the bottom
};

//==============================================================================
// The three digits of the program number
class ProgramNumberDisplay final : public juce::Component
{
public:
    void setNumber (int newNumber)
    {
        if (number != newNumber)
        {
            number = newNumber;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override
    {
        const auto area = getLocalBounds().toFloat();
        drawDisplayWindow (g, area);

        auto digits = area.withSizeKeepingCentre (26.0f, 10.0f);

        for (int divisor : { 100, 10, 1 })
        {
            drawSevenSegmentDigit (g, digits.removeFromLeft (7.0f), number / divisor);
            digits.removeFromLeft (2.5f);
        }
    }

    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override
    {
        if (onStep != nullptr)
            onStep (wheel.deltaY > 0 ? 1 : -1);
    }

    std::function<void (int)> onStep;

private:
    int number = 0;
};

// The program's name, which gives a list of all of them when it's clicked on. It
// also gets used to show what a control has just been set to.
class NameDisplay final : public juce::Component
{
public:
    void setText (const juce::String& newText)
    {
        if (text != newText)
        {
            text = newText;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override
    {
        const auto area = getLocalBounds().toFloat();
        drawDisplayWindow (g, area);

        g.setFont (juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), 9.0f, juce::Font::bold)));
        g.setColour (colours::displayOn);
        g.drawFittedText (text, getLocalBounds().reduced (4, 1), juce::Justification::centredLeft, 1, 0.5f);
    }

    void mouseDown (const juce::MouseEvent&) override
    {
        if (onClick != nullptr)
            onClick();
    }

    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override
    {
        if (onStep != nullptr)
            onStep (wheel.deltaY > 0 ? 1 : -1);
    }

    std::function<void()> onClick;
    std::function<void (int)> onStep;

private:
    juce::String text;
};

//==============================================================================
// Lights up for a moment when there's any MIDI coming in
class MidiLight final : public juce::Component
{
public:
    void setLit (bool shouldBeLit)
    {
        if (lit != shouldBeLit)
        {
            lit = shouldBeLit;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override
    {
        drawLed (g, getLocalBounds().toFloat().getCentre(), 1.6f, lit, colours::redLed);
    }

private:
    bool lit = false;
};

//==============================================================================
// Five octaves of keys to click on, which also show the notes arriving over MIDI.
// (JUCE has a keyboard component, but it's in a module that would bring several
// others, and a dependency on ALSA, along with it.)
class Keyboard final : public juce::Component
{
public:
    explicit Keyboard (juce::MidiKeyboardState& s) : state (s) {}

    ~Keyboard() override
    {
        releaseNote();
    }

    void refresh()
    {
        std::bitset<128> notes;

        for (int note = lowestNote; note <= highestNote; ++note)
            notes[(size_t) note] = state.isNoteOnForChannels (0xffff, note);

        if (notes != shownNotes)
        {
            shownNotes = notes;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override
    {
        const float keyWidth = getWhiteKeyWidth(), height = (float) getHeight();

        for (int note = lowestNote; note <= highestNote; ++note)
        {
            if (isBlackKey (note))
                continue;

            const juce::Rectangle<float> key ((float) getWhiteKeyIndex (note) * keyWidth, 0.0f, keyWidth, height);

            g.setGradientFill (juce::ColourGradient (juce::Colour (0xffdcdcdc), 0.0f, 0.0f, juce::Colours::white, 0.0f, height * 0.25f, false));
            g.fillRect (key);

            if (shownNotes[(size_t) note])
            {
                g.setColour (juce::Colours::black.withAlpha (0.27f));
                g.fillRect (key);
            }

            g.setColour (juce::Colour (0xff303030));
            g.fillRect (key.withWidth (0.6f));
        }

        for (int note = lowestNote; note <= highestNote; ++note)
        {
            if (! isBlackKey (note))
                continue;

            const auto key = getBlackKeyArea (note);
            const bool down = shownNotes[(size_t) note];

            g.setColour (juce::Colours::black);
            g.fillRect (key);

            g.setGradientFill (juce::ColourGradient (juce::Colour (down ? 0xff9a9a9a : 0xff5a5a5a), 0.0f, 0.0f,
                                                     juce::Colour (down ? 0xff505050 : 0xff1c1c1c), 0.0f, key.getBottom(), false));
            g.fillRect (key.reduced (1.4f, 0.0f).withTrimmedBottom (down ? 1.5f : 5.0f));
        }
    }

    void mouseDown (const juce::MouseEvent& e) override     { playNoteAt (e.position); }
    void mouseDrag (const juce::MouseEvent& e) override     { playNoteAt (e.position); }
    void mouseUp (const juce::MouseEvent&) override         { releaseNote(); }

private:
    static constexpr int lowestNote = 36, highestNote = 96, numWhiteKeys = 36;

    static bool isBlackKey (int note)
    {
        return ((1 << (note % 12)) & 0x54a) != 0; // C#, D#, F#, G#, A#
    }

    // The index among the white keys of this note, or of the white key before it if it's a black one
    static int getWhiteKeyIndex (int note)
    {
        static constexpr int indexInOctave[] = { 0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6 };
        return (note / 12 - lowestNote / 12) * 7 + indexInOctave[note % 12];
    }

    float getWhiteKeyWidth() const      { return (float) getWidth() / (float) numWhiteKeys; }

    juce::Rectangle<float> getBlackKeyArea (int note) const
    {
        const float keyWidth = getWhiteKeyWidth(), width = keyWidth * 0.6f;
        return { (float) (getWhiteKeyIndex (note) + 1) * keyWidth - width * 0.5f, 0.0f, width, (float) getHeight() * 0.62f };
    }

    void playNoteAt (juce::Point<float> position)
    {
        int note = -1;

        for (int n = lowestNote; n <= highestNote && note < 0; ++n)
            if (isBlackKey (n) && getBlackKeyArea (n).contains (position))
                note = n;

        for (int n = lowestNote; n <= highestNote && note < 0; ++n)
            if (! isBlackKey (n) && getWhiteKeyIndex (n) == (int) std::floor (position.x / getWhiteKeyWidth()))
                note = n;

        if (! getLocalBounds().toFloat().contains (position))
            note = -1;

        if (note != playedNote)
        {
            releaseNote();
            playedNote = note;

            if (note >= 0) // the further down the key, the harder it's hit
                state.noteOn (1, note, juce::jlimit (0.2f, 1.0f, 0.2f + position.y / (float) getHeight()));
        }
    }

    void releaseNote()
    {
        if (playedNote >= 0)
            state.noteOff (1, playedNote, 0.0f);

        playedNote = -1;
    }

    juce::MidiKeyboardState& state;
    std::bitset<128> shownNotes;
    int playedNote = -1;
};

} // namespace

//==============================================================================
class Pro54AudioProcessorEditor::Panel final : public juce::Component,
                                               private juce::Timer
{
public:
    explicit Panel (Pro54AudioProcessor& p)
        : processor (p),
          pitchWheel (p.getParam (pro54::PitchBend), [&p] { return p.getPitchWheelPosition(); }, true),
          modWheel (p.getParam (pro54::ModWheel), [&p] { return p.getModWheelPosition(); }, false),
          voices (p.getParam (pro54::ActiveVoices)),
          filterVersion (p.getParam (pro54::FilterVersion)),
          keyboard (p.keyboardState)
    {
        setSize (panelWidth, panelHeight);
        setOpaque (true);

        addAndMakeVisible (backdrop);
        backdrop.setBounds (getLocalBounds());

        for (auto& position : knobPositions)
        {
            const bool metal = position.param == pro54::MasterTune || position.param == pro54::Volume;
            auto* knob = knobs.add (new Knob (processor.getParam (position.param), metal));
            addControl (*knob, { position.x, position.y, knobSize, knobSize });
        }

        for (auto& position : switchPositions)
        {
            const auto style = position.param == pro54::TestTone ? SwitchStyle::grey : SwitchStyle::dark;
            auto* paramSwitch = switches.add (new ParamSwitch (processor.getParam (position.param), style));
            addControl (*paramSwitch, { position.x, position.y, switchWidth, switchHeight });
        }

        addControl (pitchWheel, { 27, 258, 10, 71 });
        addControl (modWheel, { 55, 258, 10, 71 });
        addControl (voices, { 245, 119, 31, 15 });
        addControl (filterVersion, { 24, 162, 50, 28 });

        // The programmer: the eight numbered switches choose the last digit of the program
        // number - or the middle one if "bank" is lit, or the first if "file" is.
        addAndMakeVisible (fileSwitch);
        fileSwitch.setBounds (202, 164, switchWidth, switchHeight);
        fileSwitch.onClick = [this] { digitBeingChosen = (digitBeingChosen == 2 ? 0 : 2); updateProgrammer(); };

        addAndMakeVisible (bankSwitch);
        bankSwitch.setBounds (222, 164, switchWidth, switchHeight);
        bankSwitch.onClick = [this] { digitBeingChosen = (digitBeingChosen == 1 ? 0 : 1); updateProgrammer(); };

        for (int i = 0; i < 8; ++i)
        {
            auto* programSwitch = programSwitches.add (new ActionSwitch());
            addAndMakeVisible (programSwitch);
            programSwitch->setBounds (286 + 20 * i, 164, switchWidth, switchHeight);
            programSwitch->onClick = [this, i] { chooseDigit (i); };
        }

        addAndMakeVisible (programNumber);
        programNumber.setBounds (245, 165, 31, 15);
        programNumber.onStep = [this] (int delta) { stepProgram (delta); };

        addAndMakeVisible (nameDisplay);
        nameDisplay.setBounds (451, 165, 131, 15);
        nameDisplay.onClick = [this] { showProgramMenu(); };
        nameDisplay.onStep = [this] (int delta) { stepProgram (delta); };

        addAndMakeVisible (midiLight);
        midiLight.setBounds (723, 168, 9, 9);

        addAndMakeVisible (keyboard);
        keyboard.setBounds (82, 249, 669, 102);

        lastMidiActivityCount = processor.getMidiActivityCount();
        timerCallback();
        startTimerHz (30);
    }

private:
    //==============================================================================
    void addControl (ParamControl& control, juce::Rectangle<int> bounds)
    {
        addAndMakeVisible (control);
        control.setBounds (bounds);
        control.onUserChange = [this, &control] { showValueOf (control.param); };
        controls.add (&control);
    }

    // Puts the name and value of a parameter up on the name display for a while
    void showValueOf (juce::RangedAudioParameter& param)
    {
        message = param.getName (64) + "  " + param.getCurrentValueAsText();
        messageTimeout = 40;
        updateProgrammer();
        updateFilterModeLabel();
    }

    // The filter's mode switch means something different to the newer filters
    void updateFilterModeLabel()
    {
        backdrop.setFilterModeLabel (filterVersion.getVersion() == 0 ? "HPF" : "BPF");
    }

    void timerCallback() override
    {
        for (auto* control : controls)
            control->refresh();

        keyboard.refresh();
        updateFilterModeLabel();

        const auto midiActivityCount = processor.getMidiActivityCount();

        if (midiActivityCount != lastMidiActivityCount)
        {
            lastMidiActivityCount = midiActivityCount;
            midiLightTimeout = 6;
        }
        else if (midiLightTimeout > 0)
        {
            --midiLightTimeout;
        }

        midiLight.setLit (midiLightTimeout > 0);

        if (messageTimeout > 0)
            --messageTimeout;

        updateProgrammer();
    }

    //==============================================================================
    // A program's number has a digit from 1 to 8 for its file, its bank, and its place in that
    static int getDigit (int program, int digit)                { return (program >> (3 * digit)) & 7; }
    static int withDigit (int program, int digit, int value)    { return (program & ~(7 << (3 * digit))) | (value << (3 * digit)); }

    void chooseDigit (int value)
    {
        processor.selectProgram (withDigit (processor.getCurrentProgram(), digitBeingChosen, value));
        messageTimeout = 0;
        updateProgrammer();
    }

    void stepProgram (int delta)
    {
        processor.selectProgram ((processor.getCurrentProgram() + delta + pro54::numPresets) % pro54::numPresets);
        messageTimeout = 0;
        updateProgrammer();
    }

    void updateProgrammer()
    {
        const int program = processor.getCurrentProgram();

        programNumber.setNumber (pro54::getPresetNumber (program));
        nameDisplay.setText (messageTimeout > 0 ? message : juce::String (pro54::presets[program].name));

        fileSwitch.setLit (digitBeingChosen == 2);
        bankSwitch.setLit (digitBeingChosen == 1);

        for (int i = 0; i < programSwitches.size(); ++i)
            programSwitches.getUnchecked (i)->setLit (getDigit (program, digitBeingChosen) == i);
    }

    void showProgramMenu()
    {
        const int current = processor.getCurrentProgram();
        juce::PopupMenu menu;

        for (int file = 0; file < 8; ++file)
        {
            juce::PopupMenu fileMenu;

            for (int i = 0; i < 64; ++i)
            {
                const int program = file * 64 + i;

                if (i > 0 && i % 16 == 0)
                    fileMenu.addColumnBreak();

                fileMenu.addItem (program + 1, processor.getProgramName (program), true, program == current);
            }

            const int first = pro54::getPresetNumber (file * 64);
            menu.addSubMenu (juce::String (first) + " - " + juce::String (first + 77), fileMenu, true, nullptr, current / 64 == file);
        }

        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (nameDisplay),
                            [safeThis = juce::Component::SafePointer<Panel> (this)] (int result)
                            {
                                if (safeThis != nullptr && result > 0)
                                {
                                    safeThis->processor.selectProgram (result - 1);
                                    safeThis->messageTimeout = 0;
                                    safeThis->updateProgrammer();
                                }
                            });
    }

    //==============================================================================
    Pro54AudioProcessor& processor;

    Backdrop backdrop;
    juce::OwnedArray<Knob> knobs;
    juce::OwnedArray<ParamSwitch> switches;
    Wheel pitchWheel, modWheel;
    VoicesDisplay voices;
    FilterVersionSelector filterVersion;
    juce::Array<ParamControl*> controls; // everything with a refresh() to call

    ActionSwitch fileSwitch, bankSwitch;
    juce::OwnedArray<ActionSwitch> programSwitches;
    ProgramNumberDisplay programNumber;
    NameDisplay nameDisplay;
    int digitBeingChosen = 0; // 0 for the program, 1 for the bank, 2 for the file

    MidiLight midiLight;
    Keyboard keyboard;

    juce::String message;
    int messageTimeout = 0, midiLightTimeout = 0;
    juce::uint32 lastMidiActivityCount = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Panel)
};

//==============================================================================
Pro54AudioProcessorEditor::Pro54AudioProcessorEditor (Pro54AudioProcessor& p)
    : AudioProcessorEditor (&p),
      pro54Processor (p),
      panel (std::make_unique<Panel> (p))
{
    // (this has to be read before anything here changes the size, which stores the new one)
    const int width = p.editorWidth;

    setOpaque (true);
    addAndMakeVisible (*panel);

    setResizable (true, true);
    setResizeLimits (panelWidth, panelHeight, panelWidth * 4, panelHeight * 4);
    getConstrainer()->setFixedAspectRatio ((double) panelWidth / (double) panelHeight);

    setSize (width, juce::roundToInt ((double) width * panelHeight / panelWidth));
}

Pro54AudioProcessorEditor::~Pro54AudioProcessorEditor() = default;

void Pro54AudioProcessorEditor::paint (juce::Graphics& g)
{
    // The panel covers all of this, bar the odd fraction of a pixel that rounding can leave along the bottom
    g.fillAll (juce::Colours::black);
}

void Pro54AudioProcessorEditor::resized()
{
    pro54Processor.editorWidth = getWidth();
    panel->setTransform (juce::AffineTransform::scale ((float) getWidth() / (float) panelWidth));
}
