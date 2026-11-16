#pragma once

// Plugin formats built by this target.
#define JucePlugin_Build_VST                    0
#define JucePlugin_Build_VST3                   1
#define JucePlugin_Build_AU                     0
#define JucePlugin_Build_AUv3                   0
#define JucePlugin_Build_AAX                    0
#define JucePlugin_Build_Standalone             0
#define JucePlugin_Build_Unity                  0
#define JucePlugin_Build_LV2                    0
#define JucePlugin_Enable_ARA                   0

#define JucePlugin_Name                         "Pro54"
#define JucePlugin_Desc                         "A port of the Cmajor Pro-54 synth"
#define JucePlugin_Manufacturer                 "voidshine"
#define JucePlugin_ManufacturerWebsite          ""
#define JucePlugin_ManufacturerEmail            ""
#define JucePlugin_ManufacturerCode             0x41737470 // 'Astp'
#define JucePlugin_PluginCode                   0x50723534 // 'Pr54'
#define JucePlugin_VSTUniqueID                  JucePlugin_PluginCode
#define JucePlugin_Vst3Category                 "Instrument|Synth"
#define JucePlugin_CFBundleIdentifier           com.voidshine.pro54

#define JucePlugin_VersionString                "0.1.0"
#define JucePlugin_VersionCode                  0x000100

#define JucePlugin_IsSynth                      1
#define JucePlugin_WantsMidiInput               1
#define JucePlugin_ProducesMidiOutput           0
#define JucePlugin_IsMidiEffect                 0
#define JucePlugin_EditorRequiresKeyboardFocus  0
