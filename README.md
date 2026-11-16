# Pro54

A native VST3 build of the Pro-54: the
[Cmajor example patch](https://github.com/cmajor-lang/cmajor/tree/main/examples/patches/Pro54)
that ports Native Instruments' Pro-53, itself a model of the Prophet-5. The
DSP has been ported from Cmajor to plain C++, and the panel is drawn natively,
so the plugin needs neither the Cmajor runtime nor a web view. It's one 5 MB
shared object that depends on nothing but the usual system libraries.

## Building

    ninja            # builds build/Pro54.vst3
    ninja install    # also copies it into ~/.vst3

This wants `g++`, `ninja`, `pkg-config` and the development files for
freetype2, fontconfig and X11: the same as any JUCE plugin on Linux. JUCE is
taken from `ext/JUCE`, which is a symlink to the JUCE library (9.0.2); point it
to your JUCE directory or the project won't build.

There's no CLAP build because this version of JUCE has no CLAP wrapper. The
engine doesn't use JUCE at all though (see below), so it could be put behind
the CLAP API directly.

## Using it

- **Programs.** The 512 factory presets are the plugin's programs, numbered as
  on a Prophet-5: three digits from 1 to 8, for the file, the bank and the
  program. Click the program name for a menu of them all, or use the mouse
  wheel over the name or number to step through. The numbered switches choose
  the last digit of the number, or the middle one while `BANK` is lit, or the
  first while `FILE` is. Programs can't be changed: to keep a sound of your
  own, save it with the host.
- **Knobs** are dragged up and down (hold shift for fine control). A
  double-click puts one back to its default, and the mouse wheel works too.
  The name display shows the value of whatever was last moved.
- **Voices** (1 to 32) is changed by dragging the number, and the **filter
  model** by clicking on its name. With the two newer filters, the filter's
  `HPF` switch selects a band-pass instead, and its label changes to say so.
- **The keyboard** plays harder the lower down a key is clicked. The window
  can be resized, and remembers its size.
- **MIDI.** Besides notes, pitch bend, the mod wheel, sustain (64) and
  sostenuto (66), nearly every control on the panel has the controller number
  that the Pro-53 gave it: 70 is the filter cutoff and 71 its resonance, for
  instance, 7 is the volume and 5 the glide. The full list is the table at the
  top of `src/Pro54Engine.cc`. Program changes (with bank select) choose
  programs.
- **Tempo.** With the `MIDI` switch of the LFO or of the delay on, its rate or
  time is a note value at the host's tempo, chosen by the knob.
- **External input.** The plugin has an optional mono input, off unless the
  host is asked to connect it, which the mixer's `EXT IN` knob feeds into the
  filter.

## The source

| | |
|-|-|
| `src/Pro54Engine.h`, `.cc` | The synth itself. Plain C++ with no dependencies: parameters and MIDI in, audio out. |
| `src/Pro54Presets.h`, `.cc` | The factory presets, as a table. |
| `src/PluginProcessor.h`, `.cc` | The JUCE plugin: host parameters, programs, state, MIDI. |
| `src/PluginEditor.h`, `.cc` | The panel. |
| `src/JucePluginDefines.h` | The plugin's name, IDs and so on. |
| `build.ninja` | The whole build: there's no configure step. |

The engine keeps the structure and the names of the Cmajor source (which is
itself "a fairly direct port" of the original C++), so the two can be read
side by side. What has changed is that its 1000-line `main()` is split into
functions, and code that was repeated many times over has been gathered up.

## Differences from the Cmajor patch

The aim was the same sound, sample for sample, and with these exceptions that
is what it is:

- **The "analog" detuning is fixed.** The Cmajor code steps the random number
  generator behind it with a signed shift, which makes it stick: from about the
  sixth voice on, every voice gets the same offsets rather than its own. This
  uses the unsigned shift that was evidently intended. Set
  `cmajorCompatibleAnalogRandomness` in `Pro54Engine.cc` to get the Cmajor
  behaviour back.
- **The MIDI sync switches work.** The patch has no tempo input, so in it they
  only stop the knobs from doing anything. Here they follow the host's tempo
  (120 bpm if it doesn't give one).
- **"All notes off" and "all sound off"** (controllers 123 and 120) are acted
  on, as hosts expect. The patch ignores them.
- **Nothing can be stored into a program**, so the panel has no `RECORD`
  switch.
- Two parameters are **named for what they do**: the patch has the names of
  the delay's low cut and high cut the wrong way round. Their IDs are
  unchanged.
- An unfinished alternative oscillator and filter output, which the patch
  contains but never runs, are left out.

Other oddities of the original are reproduced as they are.

## How it was checked

Not against Cmajor itself, which wasn't run. Instead, the Cmajor source was
converted to C++ mechanically, by a script that only rewrites syntax, to give
a reference that owes nothing to the hand-written port. The two were then fed
the same randomly generated notes, controllers and parameter changes, and
their outputs compared bit for bit: several hundred million frames at six
sample rates, covering every preset and all of the engine's code but a few
lines that can't be reached or only guard against bad input, without a
difference. The plugin itself was tested the same way against the engine,
both directly and loaded as a VST3 through JUCE's hosting code.

## Licence

The Cmajor project, including the Pro54 patch and its presets, is published
under the GPLv3 (or a commercial licence from Cmajor Software Ltd). The engine
and the preset table here are derived from it, which makes this project as a
whole subject to the GPLv3 if it's passed on to anyone. JUCE's AGPLv3 option
is compatible with that.
