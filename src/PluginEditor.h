#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"

//==============================================================================
// The synth's panel. It's laid out as the Pro-53's is (and so as the Cmajor
// patch's web view, which was put together from the original's bitmaps), but
// everything is drawn with vectors, so there are no images to ship and it
// stays sharp at any size.
class Pro54AudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit Pro54AudioProcessorEditor (Pro54AudioProcessor&);
    ~Pro54AudioProcessorEditor() override;

    void resized() override;

private:
    Pro54AudioProcessor& pro54Processor;

    // The whole panel lives in a component with the size of the original
    // design (762 x 358), which is scaled to fill the window.
    class Panel;
    std::unique_ptr<Panel> panel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pro54AudioProcessorEditor)
};
