# VST3 SDK Setup Instructions

## Overview

MrsWatson now supports VST3 plugins with a framework ready for full VST3 SDK integration. The codebase includes:

- ✅ VST3 plugin detection and loading
- ✅ Library loading and factory retrieval
- ✅ Framework for VST3 SDK integration (when SDK is available)
- ⚠️ Full audio processing requires VST3 SDK headers

## Current Status

Without VST3 SDK:
- Plugins are detected and loaded
- Factory is obtained from plugin library
- Audio passes through (not processed by plugin)

With VST3 SDK:
- Full plugin initialization
- Audio processing through plugin
- Parameter control
- Plugin capabilities querying

## Obtaining VST3 SDK

The VST3 SDK is available from Steinberg's developer portal:
- Website: https://www.steinberg.net/en/company/developers.html
- Direct download: https://download.steinberg.net/sdk_downloads/
- Latest version: VST_SDK_3.7.x (check for latest)

**Note:** The VST3 SDK requires accepting Steinberg's license agreement.

## Setting Up VST3 SDK

### Option 1: Manual Download and Extract

1. Download the VST3 SDK zipfile from Steinberg
2. Extract it to a location (e.g., `~/SDKs/VST3_SDK`)
3. Configure CMake with:
   ```bash
   cmake -DWITH_VST_SDK=~/SDKs/VST3_SDK ..
   ```

### Option 2: Place in Build Directory

1. Download and extract VST3 SDK
2. Rename the extracted folder to `VST3_SDK`
3. Place it in the `build/` directory
4. CMake will automatically detect it

### Option 3: Automatic Download (Future)

The CMake configuration is set up to support automatic download, but this is currently disabled due to licensing requirements. You must manually download and accept the license.

## Verifying VST3 SDK Integration

After setting up the SDK, rebuild:

```bash
cd build
cmake ..
cmake --build . --config Release
```

You should see messages like:
```
-- VST3 SDK found at: /path/to/VST3_SDK
-- VST3 SDK includes: /path/to/VST3_SDK/pluginterfaces
```

## Testing

Once the SDK is integrated, test with:

```bash
./main/mrswatson64 -p "HPL Processor Ultimate" --display-info
./main/mrswatson64 -p "HPL Processor Ultimate" -i input.wav -o output.wav
```

With the SDK, you should see:
- "VST3 plugin initialized: X inputs, Y outputs"
- "VST3 audio processing configured: 44100Hz, block size 512"
- Audio will be processed by the plugin (not just passed through)

## Implementation Details

The VST3 integration code is in:
- `source/plugin/PluginVst3.cpp` - Main VST3 plugin implementation
- `source/plugin/PluginVst3Mac.mm` - macOS-specific loading code
- `source/plugin/PluginVst3.h` - VST3 plugin interface

Key VST3 SDK interfaces used:
- `IPluginFactory` - Plugin factory for creating instances
- `IComponent` - Plugin component interface
- `IAudioProcessor` - Audio processing interface
- `IHostApplication` - Host application context (TODO)

## Next Steps

When VST3 SDK is available:
1. The code will automatically use SDK headers (via `WITH_VST3_SDK` define)
2. Plugins will be fully initialized
3. Audio will be processed through the plugin
4. Parameters can be controlled
5. Plugin capabilities will be queried

## Troubleshooting

**"VST3 SDK not found" warning:**
- Ensure the SDK is downloaded and extracted
- Check that the path is correct
- Verify the SDK contains `pluginterfaces/` and `public.sdk/` directories

**"Could not get VST3 plugin factory":**
- Plugin library may not export `GetPluginFactory`
- Check plugin compatibility
- Verify plugin is a valid VST3 plugin

**Compilation errors with SDK:**
- Ensure SDK version is compatible (3.7.x recommended)
- Check that all required SDK headers are present
- Verify include paths are correct
