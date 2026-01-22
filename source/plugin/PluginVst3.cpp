//
// PluginVst3.cpp - MrsWatson
// VST3 Plugin Support
// Copyright (c) 2025. All rights reserved.
//

#include "PluginVst3.h"

// VST3 SDK headers (when available)
#ifdef WITH_VST3_SDK
// Disable warnings for VST3 SDK headers
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpre-c++14-compat"
#pragma clang diagnostic ignored "-Wswitch-default"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivsthostapplication.h"
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/base/funknown.h"
using namespace Steinberg;
using namespace Steinberg::Vst;
#pragma clang diagnostic pop
#endif

// C headers need extern "C" when included in C++
extern "C" {
#include "audio/AudioSettings.h"
#include "audio/SampleBuffer.h"
#include "base/File.h"
#include "base/LinkedList.h"
#include "base/PlatformInfo.h"
#include "base/Types.h"
#include "logging/EventLogger.h"
#include "midi/MidiEvent.h"
#include "plugin/Plugin.h"
}

// C linkage for functions called from C code
extern "C" {

#if MACOSX
extern LinkedList getVst3PluginLocations(CharString currentDirectory);
extern void* getLibraryHandleForVst3Plugin(const CharString pluginAbsolutePath);
extern void* getVst3PluginFactory(void* libraryHandle);
extern void closeVst3LibraryHandle(void* libraryHandle);
#elif LINUX
// TODO: Linux VST3 support
#elif WINDOWS
// TODO: Windows VST3 support
#endif

// VST3 plugin data structure
typedef struct {
  void* libraryHandle;
  void* pluginFactory;  // IPluginFactory* when SDK available
  void* pluginInstance; // IComponent* when SDK available
  void* audioProcessor; // IAudioProcessor* when SDK available
  boolByte isInitialized;
  int32 inputBusCount;
  int32 outputBusCount;
#ifdef WITH_VST3_SDK
  IHostApplication* hostApplication;
#endif
} PluginVst3DataMembers;
typedef PluginVst3DataMembers *PluginVst3Data;

const char *_getVst3PlatformExtension(void);
const char *_getVst3PlatformExtension(void) {
  PlatformInfo platform = newPlatformInfo();
  PlatformType platformType = platform->type;
  freePlatformInfo(platform);

  switch (platformType) {
  case PLATFORM_MACOSX:
    return ".vst3";

  case PLATFORM_WINDOWS:
    return ".vst3";

  case PLATFORM_LINUX:
    return ".vst3";

  default:
    logError("Unknown platform for VST3 extension");
    return ".vst3";
  }
}

// Helper struct and function for searching plugin locations
struct Vst3SearchData {
  CharString pluginNameWithExtension;
  Plugin plugin;
  boolByte found;
};

static void _searchVst3Location(void *item, void *userData) {
  CharString location = (CharString)item;
  struct Vst3SearchData *data = (struct Vst3SearchData *)userData;
  
  if (data->found) return;
  
  File locationFile = newFileWithPath(location);
  File testPath = newFileWithParent(locationFile, data->pluginNameWithExtension);
  
  if (testPath != NULL && fileExists(testPath)) {
    charStringCopy(data->plugin->pluginAbsolutePath, testPath->absolutePath);
    charStringCopy(data->plugin->pluginLocation, location);
    data->found = true;
  }
  
  if (testPath != NULL) freeFile(testPath);
  freeFile(locationFile);
}

static void _logVst3Location(void *item, void *userData) {
  (void)userData;
  CharString location = (CharString)item;
  logInfo("  %s", location->data);
}

static void _listVst3PluginsInLocation(void *item, void *userData) {
  (void)userData;
  CharString location = (CharString)item;
  File locationFile = newFileWithPath(location);
  
  if (fileExists(locationFile)) {
    logInfo("Searching for VST3 plugins in: %s", location->data);
    // TODO: Implement directory scanning for .vst3 bundles
  }
  
  freeFile(locationFile);
}

struct Vst3ExistsSearchData {
  CharString pluginNameWithExtension;
  boolByte found;
};

static void _checkVst3Location(void *item, void *userData) {
  CharString location = (CharString)item;
  struct Vst3ExistsSearchData *data = (struct Vst3ExistsSearchData *)userData;
  
  if (data->found) return;
  
  File locationFile = newFileWithPath(location);
  File testPath = newFileWithParent(locationFile, data->pluginNameWithExtension);
  
  if (testPath != NULL && fileExists(testPath)) {
    data->found = true;
  }
  
  if (testPath != NULL) freeFile(testPath);
  freeFile(locationFile);
}

void listAvailablePluginsVst3(const CharString pluginRoot) {
  CharString currentDirectory = newCharString();
  if (pluginRoot != NULL && !charStringIsEmpty(pluginRoot)) {
    charStringCopy(currentDirectory, pluginRoot);
  } else {
    charStringAppendCString(currentDirectory, ".");
  }

  LinkedList locations = getVst3PluginLocations(currentDirectory);

  logInfo("VST3 plugin search locations:");
  linkedListForeach(locations, _logVst3Location, NULL);

  // List plugins in each location
  linkedListForeach(locations, _listVst3PluginsInLocation, NULL);
  
  freeLinkedList(locations);
  freeCharString(currentDirectory);
}

boolByte pluginVst3Exists(const CharString pluginName,
                          const CharString pluginRoot) {
  if (pluginName == NULL || charStringIsEmpty(pluginName)) {
    return false;
  }

  CharString currentDirectory = newCharString();
  if (pluginRoot != NULL && !charStringIsEmpty(pluginRoot)) {
    charStringCopy(currentDirectory, pluginRoot);
  } else {
    charStringAppendCString(currentDirectory, ".");
  }

  // Check if it's an absolute path
  File pluginPath = newFileWithPath(pluginName);
  if (fileExists(pluginPath)) {
    freeFile(pluginPath);
    freeCharString(currentDirectory);
    return true;
  }
  freeFile(pluginPath);

  // Search in standard locations
  LinkedList locations = getVst3PluginLocations(currentDirectory);
  const char* extension = _getVst3PlatformExtension();
  CharString pluginNameWithExtension = newCharString();
  charStringCopy(pluginNameWithExtension, pluginName);
  charStringAppendCString(pluginNameWithExtension, extension);

  struct Vst3ExistsSearchData searchData;
  searchData.pluginNameWithExtension = pluginNameWithExtension;
  searchData.found = false;

  linkedListForeach(locations, _checkVst3Location, &searchData);
  
  freeCharString(pluginNameWithExtension);
  freeLinkedList(locations);
  
  if (searchData.found) {
    freeCharString(currentDirectory);
    return true;
  }
  freeCharString(currentDirectory);
  return false;
}

// Basic VST3 plugin loading - will be expanded with VST3 SDK for full functionality
static boolByte _openVst3Plugin(void *pluginPtr) {
  Plugin plugin = (Plugin)pluginPtr;
  PluginVst3Data data = (PluginVst3Data)plugin->extraData;
  
  if (data == NULL) {
    logError("VST3 plugin data is NULL");
    return false;
  }
  
  if (data->isInitialized) {
    logDebug("VST3 plugin '%s' is already initialized", plugin->pluginName->data);
    return true;
  }
  
  logInfo("Opening VST3 plugin '%s'", plugin->pluginName->data);
  
  // Get the plugin path
  File pluginPath = newFileWithPath(plugin->pluginName);
  if (!fileExists(pluginPath)) {
    // Try with .vst3 extension
    CharString pluginNameWithExt = newCharString();
    charStringCopy(pluginNameWithExt, plugin->pluginName);
    charStringAppendCString(pluginNameWithExt, ".vst3");
    
    File pluginLocationPath = newFileWithPath(plugin->pluginLocation);
    freeFile(pluginPath);
    pluginPath = newFileWithParent(pluginLocationPath, pluginNameWithExt);
    
    freeFile(pluginLocationPath);
    freeCharString(pluginNameWithExt);
    
    if (fileExists(pluginPath)) {
      charStringCopy(plugin->pluginAbsolutePath, pluginPath->absolutePath);
    }
  } else {
    charStringCopy(plugin->pluginAbsolutePath, pluginPath->absolutePath);
  }
  
  if (!fileExists(pluginPath)) {
    logError("VST3 plugin file '%s' does not exist", plugin->pluginAbsolutePath->data);
    freeFile(pluginPath);
    return false;
  }
  
  logDebug("VST3 plugin location: %s", pluginPath->absolutePath->data);
  freeFile(pluginPath);
  
  // Load the VST3 plugin library
  data->libraryHandle = getLibraryHandleForVst3Plugin(plugin->pluginAbsolutePath);
  if (data->libraryHandle == NULL) {
    logError("Could not load VST3 plugin library '%s'", plugin->pluginAbsolutePath->data);
    return false;
  }
  
  logInfo("VST3 plugin library loaded successfully");
  
  // Get plugin factory from the loaded library
  data->pluginFactory = getVst3PluginFactory(data->libraryHandle);
  if (data->pluginFactory == NULL) {
    logError("Could not get VST3 plugin factory");
    closeVst3LibraryHandle(data->libraryHandle);
    data->libraryHandle = NULL;
    return false;
  }
  
  logInfo("VST3 plugin factory obtained");
  
#ifdef WITH_VST3_SDK
  // Use VST3 SDK to initialize plugin
  IPluginFactory* factory = (IPluginFactory*)data->pluginFactory;
  
  // Get number of classes
  int32 numClasses = factory->countClasses();
  if (numClasses == 0) {
    logError("VST3 plugin factory has no classes");
    return false;
  }
  
  logDebug("VST3 plugin has %d class(es)", numClasses);
  
  // Find the Audio Module Class (main plugin component)
  PClassInfo classInfo;
  bool foundAudioModule = false;
  for (int32 i = 0; i < numClasses; i++) {
    if (factory->getClassInfo(i, &classInfo) != kResultOk) {
      continue;
    }
    logDebug("VST3 class %d: %s, category: %s", i, classInfo.name, classInfo.category);
    // Look for "Audio Module Class" which is the main plugin component
    if (strcmp(classInfo.category, "Audio Module Class") == 0) {
      foundAudioModule = true;
      break;
    }
  }
  
  if (!foundAudioModule) {
    // Fall back to first class if no Audio Module Class found
    if (factory->getClassInfo(0, &classInfo) != kResultOk) {
      logError("Could not get VST3 plugin class info");
      return false;
    }
    logWarn("No Audio Module Class found, using first class: %s", classInfo.name);
  }
  
  logInfo("VST3 plugin class: %s, category: %s", classInfo.name, classInfo.category);
  
  // Create component instance
  IComponent* component = NULL;
  tresult result = factory->createInstance(classInfo.cid, IComponent::iid, (void**)&component);
  if (result != kResultOk || component == NULL) {
    logError("Could not create VST3 component instance (result: %d)", result);
    return false;
  }
  
  data->pluginInstance = component;
  
  // Initialize component
  // TODO: Create proper host application context
  if (component->initialize(NULL) != kResultOk) {
    logError("Could not initialize VST3 component");
    component->release();
    data->pluginInstance = NULL;
    return false;
  }
  
  // Get audio processor interface
  if (component->queryInterface(IAudioProcessor::iid, (void**)&data->audioProcessor) != kResultOk) {
    logWarn("VST3 plugin does not support IAudioProcessor interface");
    data->audioProcessor = NULL;
  }
  
  // Query plugin capabilities and activate buses
  BusInfo busInfo;
  int32 numInputs = 0, numOutputs = 0;
  int32 inputBusCount = component->getBusCount(kAudio, kInput);
  int32 outputBusCount = component->getBusCount(kAudio, kOutput);
  
  // Activate and count input buses
  for (int32 i = 0; i < inputBusCount; i++) {
    if (component->getBusInfo(kAudio, kInput, i, busInfo) == kResultOk) {
      numInputs += busInfo.channelCount;
      // Activate the bus (required before processing)
      component->activateBus(kAudio, kInput, i, true);
    }
  }
  
  // Activate and count output buses
  for (int32 i = 0; i < outputBusCount; i++) {
    if (component->getBusInfo(kAudio, kOutput, i, busInfo) == kResultOk) {
      numOutputs += busInfo.channelCount;
      // Activate the bus (required before processing)
      component->activateBus(kAudio, kOutput, i, true);
    }
  }
  
  logInfo("VST3 plugin initialized: %d input buses (%d channels), %d output buses (%d channels)", 
          inputBusCount, numInputs, outputBusCount, numOutputs);
  
  // Store bus information for audio processing
  data->inputBusCount = inputBusCount;
  data->outputBusCount = outputBusCount;
  
  // Set up audio processing
  if (data->audioProcessor != NULL) {
    IAudioProcessor* processor = (IAudioProcessor*)data->audioProcessor;
    ProcessSetup setup;
    setup.processMode = kRealtime;
    setup.symbolicSampleSize = kSample32;
    setup.maxSamplesPerBlock = (int32)getBlocksize();
    setup.sampleRate = (double)getSampleRate();
    
    if (processor->setupProcessing(setup) != kResultOk) {
      logWarn("Could not set up VST3 audio processing");
    } else {
      logInfo("VST3 audio processing configured: %dHz, block size %d", 
              (int)setup.sampleRate, (int)setup.maxSamplesPerBlock);
    }
  }
  
  // Activate the component (required before processing)
  if (component->setActive(true) != kResultOk) {
    logWarn("Could not activate VST3 component");
  }
  
  // Determine plugin type
  if (component->getBusCount(kEvent, kInput) > 0) {
    plugin->pluginType = PLUGIN_TYPE_INSTRUMENT;
  } else {
    plugin->pluginType = PLUGIN_TYPE_EFFECT;
  }
  
  data->isInitialized = true;
  return true;
#else
  // Without VST3 SDK, we can only load the library and get the factory
  // Full initialization requires VST3 SDK headers
  data->isInitialized = true;
  plugin->pluginType = PLUGIN_TYPE_EFFECT; // Default to effect
  logWarn("VST3 plugin factory obtained but full initialization requires VST3 SDK headers");
  logWarn("Set WITH_VST3_SDK=1 and provide VST3 SDK path to enable full functionality");
  return true;
#endif
}

static void _closeVst3Plugin(void *pluginPtr) {
  Plugin plugin = (Plugin)pluginPtr;
  PluginVst3Data data = (PluginVst3Data)plugin->extraData;
  if (data != NULL) {
#ifdef WITH_VST3_SDK
    if (data->audioProcessor != NULL) {
      ((IAudioProcessor*)data->audioProcessor)->release();
      data->audioProcessor = NULL;
    }
    if (data->pluginInstance != NULL) {
      ((IComponent*)data->pluginInstance)->terminate();
      ((IComponent*)data->pluginInstance)->release();
      data->pluginInstance = NULL;
    }
    if (data->pluginFactory != NULL) {
      ((IPluginFactory*)data->pluginFactory)->release();
      data->pluginFactory = NULL;
    }
#endif
    if (data->libraryHandle != NULL) {
      closeVst3LibraryHandle(data->libraryHandle);
      data->libraryHandle = NULL;
    }
    free(data);
    plugin->extraData = NULL;
  }
}

static void _processVst3Audio(void *pluginPtr, SampleBuffer inputs,
                              SampleBuffer outputs) {
  Plugin plugin = (Plugin)pluginPtr;
  PluginVst3Data data = (PluginVst3Data)plugin->extraData;
  
  if (data == NULL || !data->isInitialized) {
    // Pass through if not initialized
    if (inputs != NULL && outputs != NULL && inputs->samples != NULL && outputs->samples != NULL) {
      sampleBufferCopyAndMapChannels(outputs, inputs);
    }
    return;
  }
  
#ifdef WITH_VST3_SDK
  if (data->audioProcessor == NULL) {
    // No audio processor, pass through
    if (inputs != NULL && outputs != NULL && inputs->samples != NULL && outputs->samples != NULL) {
      sampleBufferCopyAndMapChannels(outputs, inputs);
    }
    return;
  }
  
  // Process audio using VST3 SDK
  IAudioProcessor* processor = (IAudioProcessor*)data->audioProcessor;
  
  // Set up ProcessData structure
  ProcessData processData;
  processData.processMode = kRealtime;
  processData.symbolicSampleSize = kSample32;
  processData.numSamples = (int32)outputs->blocksize;
  processData.inputParameterChanges = NULL;
  processData.outputParameterChanges = NULL;
  processData.inputEvents = NULL;
  processData.outputEvents = NULL;
  processData.processContext = NULL;
  
  // Set up input/output buffers based on actual bus structure
  // VST3 uses buses - we need to match the plugin's bus structure
  // SampleBuffer uses planar format: samples[channel][sample], which matches VST3's channelBuffers32
  IComponent* component = (IComponent*)data->pluginInstance;
  int32 inputBusCount = data->inputBusCount;
  int32 outputBusCount = data->outputBusCount;
  
  // Validate bus counts
  if (inputBusCount <= 0 || outputBusCount <= 0) {
    logWarn("Invalid bus counts: %d inputs, %d outputs, falling back to pass-through", inputBusCount, outputBusCount);
    if (inputs != NULL && outputs != NULL && inputs->samples != NULL && outputs->samples != NULL) {
      sampleBufferCopyAndMapChannels(outputs, inputs);
    }
    return;
  }
  
  // Allocate bus buffers
  AudioBusBuffers* inputBuffers = new AudioBusBuffers[inputBusCount];
  AudioBusBuffers* outputBuffers = new AudioBusBuffers[outputBusCount];
  
  // Set up input buses
  BusInfo busInfo;
  int32 channelOffset = 0;
  // Allocate a silent buffer for channels we don't have (VST3 requires valid buffers)
  static Sample32* silentInputBuffer = NULL;
  static int32 silentInputBufferSize = 0;
  int32 requiredSilentSize = (int32)outputs->blocksize;
  if (silentInputBuffer == NULL || silentInputBufferSize < requiredSilentSize) {
    if (silentInputBuffer != NULL) {
      delete[] silentInputBuffer;
    }
    silentInputBuffer = new Sample32[requiredSilentSize];
    silentInputBufferSize = requiredSilentSize;
    memset(silentInputBuffer, 0, sizeof(Sample32) * requiredSilentSize);
  }
  
  for (int32 busIdx = 0; busIdx < inputBusCount; busIdx++) {
    if (component->getBusInfo(kAudio, kInput, busIdx, busInfo) == kResultOk) {
      inputBuffers[busIdx].numChannels = busInfo.channelCount;
      inputBuffers[busIdx].silenceFlags = 0;
      // Allocate array of channel buffer pointers for this bus
      Sample32** busChannelPtrs = new Sample32*[busInfo.channelCount];
      // Initialize all pointers to valid buffers (required by VST3)
      for (int32 ch = 0; ch < busInfo.channelCount; ch++) {
        if ((channelOffset + ch) < inputs->numChannels) {
          busChannelPtrs[ch] = (Sample32*)inputs->samples[channelOffset + ch];
        } else {
          // Plugin wants more channels than we have - use silent buffer
          busChannelPtrs[ch] = silentInputBuffer;
          inputBuffers[busIdx].silenceFlags |= (1ULL << ch); // Mark as silent
        }
      }
      inputBuffers[busIdx].channelBuffers32 = busChannelPtrs;
      if (busInfo.channelCount > inputs->numChannels - channelOffset) {
        logDebug("Input bus %d: plugin expects %d channels, we have %d (using silent buffers for extra channels)",
                 busIdx, busInfo.channelCount, inputs->numChannels - channelOffset);
      }
      channelOffset += busInfo.channelCount;
    } else {
      logWarn("Could not get bus info for input bus %d", busIdx);
      // Set up empty bus to avoid crash
      inputBuffers[busIdx].numChannels = 0;
      inputBuffers[busIdx].channelBuffers32 = NULL;
      inputBuffers[busIdx].silenceFlags = 0;
    }
  }
  
  // Set up output buses
  channelOffset = 0;
  // Allocate a silent buffer for channels we don't have (VST3 requires valid buffers)
  static Sample32* silentOutputBuffer = NULL;
  static int32 silentOutputBufferSize = 0;
  if (silentOutputBuffer == NULL || silentOutputBufferSize < requiredSilentSize) {
    if (silentOutputBuffer != NULL) {
      delete[] silentOutputBuffer;
    }
    silentOutputBuffer = new Sample32[requiredSilentSize];
    silentOutputBufferSize = requiredSilentSize;
    memset(silentOutputBuffer, 0, sizeof(Sample32) * requiredSilentSize);
  }
  
  for (int32 busIdx = 0; busIdx < outputBusCount; busIdx++) {
    if (component->getBusInfo(kAudio, kOutput, busIdx, busInfo) == kResultOk) {
      outputBuffers[busIdx].numChannels = busInfo.channelCount;
      outputBuffers[busIdx].silenceFlags = 0;
      // Allocate array of channel buffer pointers for this bus
      Sample32** busChannelPtrs = new Sample32*[busInfo.channelCount];
      // Initialize all pointers to valid buffers (required by VST3)
      for (int32 ch = 0; ch < busInfo.channelCount; ch++) {
        if ((channelOffset + ch) < outputs->numChannels) {
          busChannelPtrs[ch] = (Sample32*)outputs->samples[channelOffset + ch];
        } else {
          // Plugin wants more channels than we have - use silent buffer (we'll ignore the output)
          busChannelPtrs[ch] = silentOutputBuffer;
        }
      }
      outputBuffers[busIdx].channelBuffers32 = busChannelPtrs;
      if (busInfo.channelCount > outputs->numChannels - channelOffset) {
        logDebug("Output bus %d: plugin expects %d channels, we have %d (using silent buffers for extra channels)",
                 busIdx, busInfo.channelCount, outputs->numChannels - channelOffset);
      }
      channelOffset += busInfo.channelCount;
    } else {
      logWarn("Could not get bus info for output bus %d", busIdx);
      // Set up empty bus to avoid crash
      outputBuffers[busIdx].numChannels = 0;
      outputBuffers[busIdx].channelBuffers32 = NULL;
      outputBuffers[busIdx].silenceFlags = 0;
    }
  }
  
  processData.inputs = inputBuffers;
  processData.numInputs = inputBusCount;
  processData.outputs = outputBuffers;
  processData.numOutputs = outputBusCount;
  
  // Process audio
  tresult result = processor->process(processData);
  if (result != kResultOk) {
    logWarn("VST3 audio processing returned error: %d, falling back to pass-through", result);
    // Fall back to pass-through on error
    if (inputs != NULL && outputs != NULL && inputs->samples != NULL && outputs->samples != NULL) {
      sampleBufferCopyAndMapChannels(outputs, inputs);
    }
  }
  
  // Clean up allocated arrays
  for (int32 i = 0; i < inputBusCount; i++) {
    delete[] inputBuffers[i].channelBuffers32;
  }
  delete[] inputBuffers;
  for (int32 i = 0; i < outputBusCount; i++) {
    delete[] outputBuffers[i].channelBuffers32;
  }
  delete[] outputBuffers;
#else
  // Without VST3 SDK, pass through audio
  if (inputs != NULL && outputs != NULL && inputs->samples != NULL && outputs->samples != NULL) {
    sampleBufferCopyAndMapChannels(outputs, inputs);
  }
  logWarn("VST3 audio processing requires VST3 SDK (set WITH_VST3_SDK=1)");
#endif
}

static void _processVst3Midi(void *pluginPtr, LinkedList midiEvents) {
  (void)pluginPtr; (void)midiEvents;
  // TODO: Implement VST3 MIDI processing
}

static boolByte _setVst3Parameter(void *pluginPtr, unsigned int index,
                                   float value) {
  (void)pluginPtr; (void)index; (void)value;
  // TODO: Implement VST3 parameter setting
  return false;
}

static int _getVst3Setting(void *pluginPtr, PluginSetting setting) {
  Plugin plugin = (Plugin)pluginPtr;
  PluginVst3Data data = (PluginVst3Data)plugin->extraData;
  
  if (data == NULL || !data->isInitialized) {
    return 0;
  }
  
#ifdef WITH_VST3_SDK
  if (data->pluginInstance != NULL) {
    IComponent* component = (IComponent*)data->pluginInstance;
    BusInfo busInfo;
    
    switch (setting) {
      case PLUGIN_NUM_INPUTS: {
        int32 numInputs = 0;
        for (int32 i = 0; i < component->getBusCount(kAudio, kInput); i++) {
          if (component->getBusInfo(kAudio, kInput, i, busInfo) == kResultOk) {
            numInputs += busInfo.channelCount;
          }
        }
        return numInputs > 0 ? numInputs : 2; // Default to stereo if unknown
      }
      case PLUGIN_NUM_OUTPUTS: {
        int32 numOutputs = 0;
        for (int32 i = 0; i < component->getBusCount(kAudio, kOutput); i++) {
          if (component->getBusInfo(kAudio, kOutput, i, busInfo) == kResultOk) {
            numOutputs += busInfo.channelCount;
          }
        }
        return numOutputs > 0 ? numOutputs : 2; // Default to stereo if unknown
      }
      case PLUGIN_INITIAL_DELAY:
        if (data->audioProcessor != NULL) {
          IAudioProcessor* processor = (IAudioProcessor*)data->audioProcessor;
          return processor->getLatencySamples();
        }
        return 0;
      case PLUGIN_SETTING_TAIL_TIME_IN_MS:
        // VST3 doesn't have a direct tail time query, would need to check parameters
        return 0;
      default:
        return 0;
    }
  }
#endif
  
  // Default values when SDK not available or instance not created
  switch (setting) {
    case PLUGIN_NUM_INPUTS:
      return 2; // Default stereo input
    case PLUGIN_NUM_OUTPUTS:
      return 2; // Default stereo output
    case PLUGIN_INITIAL_DELAY:
      return 0;
    case PLUGIN_SETTING_TAIL_TIME_IN_MS:
      return 0;
    default:
      return 0;
  }
}

static void _displayVst3Info(void *pluginPtr) {
  Plugin plugin = (Plugin)pluginPtr;
  if (plugin != NULL) {
    logInfo("VST3 Plugin: %s", plugin->pluginName->data);
    logInfo("  Location: %s", plugin->pluginLocation->data);
  }
}

static void _prepareVst3ForProcessing(void *pluginPtr) {
  (void)pluginPtr;
  // TODO: Implement VST3 prepare
}

static void _showVst3Editor(void *pluginPtr) {
  (void)pluginPtr;
  logUnsupportedFeature("VST3 editor display");
}

static void _freeVst3Data(void *data) {
  if (data != NULL) {
    free(data);
  }
}

Plugin newPluginVst3(const CharString pluginName, const CharString pluginRoot) {
  if (!pluginVst3Exists(pluginName, pluginRoot)) {
    return NULL;
  }

  Plugin plugin = _newPlugin(PLUGIN_TYPE_VST_3, PLUGIN_TYPE_UNKNOWN);
  if (plugin == NULL) {
    return NULL;
  }

  PluginVst3Data data = (PluginVst3Data)malloc(sizeof(PluginVst3DataMembers));
  if (data == NULL) {
    freePlugin(plugin);
    return NULL;
  }
  // Initialize all fields
  data->libraryHandle = NULL;
  data->pluginFactory = NULL;
  data->pluginInstance = NULL;
  data->audioProcessor = NULL;
  data->isInitialized = false;
  data->inputBusCount = 0;
  data->outputBusCount = 0;
#ifdef WITH_VST3_SDK
  data->hostApplication = NULL;
#endif

  data->libraryHandle = NULL;
  data->pluginFactory = NULL;
  data->pluginInstance = NULL;
  data->audioProcessor = NULL;
  data->isInitialized = false;
#ifdef WITH_VST3_SDK
  data->hostApplication = NULL;
#else
  // Ensure audioProcessor field exists even without SDK
  (void)data->audioProcessor; // Suppress unused warning
#endif

  plugin->extraData = data;
  plugin->openPlugin = _openVst3Plugin;
  plugin->closePlugin = _closeVst3Plugin;
  plugin->processAudio = _processVst3Audio;
  plugin->processMidiEvents = _processVst3Midi;
  plugin->setParameter = _setVst3Parameter;
  plugin->getSetting = _getVst3Setting;
  plugin->displayInfo = _displayVst3Info;
  plugin->prepareForProcessing = _prepareVst3ForProcessing;
  plugin->showEditor = _showVst3Editor;
  plugin->freePluginData = _freeVst3Data;

  charStringCopy(plugin->pluginName, pluginName);

  // Find and set plugin location
  CharString currentDirectory = newCharString();
  if (pluginRoot != NULL && !charStringIsEmpty(pluginRoot)) {
    charStringCopy(currentDirectory, pluginRoot);
  } else {
    charStringAppendCString(currentDirectory, ".");
  }

  File pluginPath = newFileWithPath(pluginName);
  if (fileExists(pluginPath)) {
    charStringCopy(plugin->pluginAbsolutePath, pluginPath->absolutePath);
    File parent = fileGetParent(pluginPath);
    if (parent != NULL) {
      charStringCopy(plugin->pluginLocation, parent->absolutePath);
      freeFile(parent);
    }
  } else {
    LinkedList locations = getVst3PluginLocations(currentDirectory);
    const char* extension = _getVst3PlatformExtension();
    CharString pluginNameWithExtension = newCharString();
    charStringCopy(pluginNameWithExtension, pluginName);
    charStringAppendCString(pluginNameWithExtension, extension);

    // Search through locations
    struct Vst3SearchData searchData;
    searchData.pluginNameWithExtension = pluginNameWithExtension;
    searchData.plugin = plugin;
    searchData.found = false;

    linkedListForeach(locations, _searchVst3Location, &searchData);

    freeCharString(pluginNameWithExtension);
    freeLinkedList(locations);
  }

  freeFile(pluginPath);
  freeCharString(currentDirectory);

  return plugin;
}

boolByte pluginVst3SetParameter(Plugin self, unsigned int index, float value) {
  if (self == NULL || self->interfaceType != PLUGIN_TYPE_VST_3) {
    return false;
  }
  return self->setParameter(self, index, value);
}

} // extern "C"
