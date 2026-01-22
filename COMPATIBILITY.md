# Cross-Platform Compatibility Strategy

This document outlines the compatibility strategy for MrsWatson across modern command-line operating systems.

## Supported Platforms

### macOS
- **Minimum Version**: macOS 10.13 (High Sierra)
- **Rationale**: macOS 10.13 provides a good balance between modern feature support and compatibility with older hardware. It was released in 2017 and supports:
  - Modern C++ standard library (libc++)
  - Current Xcode toolchains
  - Modern Objective-C runtime features
  - While still supporting systems from 2017 onwards

### Linux
- **Standard**: POSIX-compliant systems
- **C Standard**: C99
- **POSIX Source**: 200809L (POSIX.1-2008)

### Windows
- **Visual Studio**: 2013 or later (supports C99)

## Build System Compatibility

### CMake
- **Minimum Version**: 3.0
- **Generator Support**: 
  - Unix Makefiles
  - Ninja
  - Xcode (macOS)
  - Visual Studio (Windows)

### Compiler Compatibility

#### macOS
- **Default C++ Library**: libc++ (modern, default on macOS)
- **Removed**: libstdc++ (deprecated on macOS)
- **Deployment Target**: Set via `CMAKE_OSX_DEPLOYMENT_TARGET` and `-mmacosx-version-min`

#### Linux
- **C Standard**: C99 (`-std=c99`)
- **POSIX Compliance**: `-D_POSIX_C_SOURCE=200809L`

#### Windows
- **C Standard**: C99 (VS 2013+)
- **Runtime**: Static linking (`/MT`)

## API Modernization

### macOS APIs Updated

1. **NSProcessInfo**
   - **Old**: `operatingSystemVersionString` (deprecated)
   - **New**: `operatingSystemVersion` (modern API)
   - **Location**: `source/base/PlatformInfoMac.m`

2. **NSWindow**
   - **Old**: `NSTitledWindowMask`, `NSResizableWindowMask`, etc. (deprecated)
   - **New**: `NSWindowStyleMaskTitled`, `NSWindowStyleMaskResizable`, etc.
   - **Location**: `source/plugin/PluginVst2xMac.mm`

## Design Principles

### 1. Feature Detection Over OS Detection
Where possible, the code uses feature detection rather than OS detection for better portability.

### 2. POSIX Compliance
Core functionality adheres to POSIX standards for maximum Unix-like system compatibility.

### 3. Abstraction Layers
Platform-specific code is isolated in separate files:
- `PluginVst2xMac.mm` (macOS)
- `PluginVst2xLinux.cpp` (Linux)
- `PluginVst2xWindows.cpp` (Windows)
- `PlatformInfoMac.m` (macOS platform info)

### 4. Modern Standards
- Uses modern C++ standard library (libc++ on macOS)
- Follows current API conventions
- Avoids deprecated APIs where possible

## Testing Recommendations

For maximum compatibility verification:

1. **macOS**: Test on multiple macOS versions (10.13+)
2. **Linux**: Test on multiple distributions (Ubuntu, Debian, Fedora, etc.)
3. **Windows**: Test on Windows 10/11 with Visual Studio 2013+

## Future Considerations

- Consider raising macOS minimum to 10.14 (Mojave) if 10.13 support becomes burdensome
- Monitor for new deprecations in Apple's APIs
- Keep CMake minimum version current with project needs
- Consider C++11/14/17 features if they provide better cross-platform support
