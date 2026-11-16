#pragma once

#include "Pro54Engine.h"

namespace pro54
{

//==============================================================================
// The Pro-53's 512 factory presets, converted from gui/presets/presetBank.js
// in the Cmajor patch.
//
// As on a Prophet-5, they're numbered with three digits which each go from 1
// to 8 - the file, the bank and the program - so the first is 111 and the
// last is 888.
constexpr int numPresets = 512;

struct Preset
{
    const char* name;

    // One value for each of the first numPresetParams entries of paramOrder.
    // The knobs are held as 0-127 for 0-100% (every one of the original values
    // is a 7-bit number of that kind), the switches as 0 or 1, and the number
    // of voices as itself.
    unsigned char values[numPresetParams];

    // Returns the value that this preset gives the parameter paramOrder[index].
    float getValue (int index) const;
};

extern const Preset presets[numPresets];

constexpr int getPresetNumber (int index)
{
    return 111 + 100 * (index >> 6) + 10 * ((index >> 3) & 7) + (index & 7);
}

} // namespace pro54
