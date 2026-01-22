cmake_minimum_required(VERSION 3.5)
project(MrsWatsonCore)

include(${mw_cmake_scripts_DIR}/ConfigureTarget.cmake)

###########
# Sources #
###########

set(core_SOURCES
  app/BuildInfo.c
  app/ProgramOption.c
  audio/AudioSettings.c
  audio/PcmSampleBuffer.c
  audio/SampleBuffer.c
  base/CharString.c
  base/Endian.c
  base/File.c
  base/LinkedList.c
  base/PlatformInfo.c
  io/RiffFile.c
  io/SampleSource.c
  io/SampleSourcePcm.c
  io/SampleSourceSilence.c
  io/SampleSourceWave.c
  logging/ErrorReporter.c
  logging/EventLogger.c
  logging/LogPrinter.c
  midi/MidiEvent.c
  midi/MidiSequence.c
  midi/MidiSource.c
  midi/MidiSourceFile.c
  plugin/Plugin.c
  plugin/PluginChain.c
  plugin/PluginGain.c
  plugin/PluginLimiter.c
  plugin/PluginPassthru.c
  plugin/PluginPreset.c
  plugin/PluginSilence.c
  plugin/PluginVst3.cpp
  time/AudioClock.c
  time/TaskTimer.c

  MrsWatson.c
  MrsWatsonOptions.c
)

set(core_HEADERS
  app/BuildInfo.h
  app/ProgramOption.h
  app/ReturnCodes.h
  audio/AudioSettings.h
  audio/PcmSampleBuffer.h
  audio/SampleBuffer.h
  base/CharString.h
  base/Endian.h
  base/File.h
  base/LinkedList.h
  base/PlatformInfo.h
  base/Types.h
  io/RiffFile.h
  io/SampleSource.h
  io/SampleSourcePcm.h
  io/SampleSourceSilence.h
  io/SampleSourceWave.h
  logging/ErrorReporter.h
  logging/EventLogger.h
  logging/LogPrinter.h
  midi/MidiEvent.h
  midi/MidiSequence.h
  midi/MidiSource.h
  midi/MidiSourceFile.h
  plugin/Plugin.h
  plugin/PluginChain.h
  plugin/PluginGain.h
  plugin/PluginLimiter.h
  plugin/PluginPassthru.h
  plugin/PluginPreset.h
  plugin/PluginSilence.h
  plugin/PluginVst3.h
  time/AudioClock.h
  time/TaskTimer.h

  MrsWatson.h
  MrsWatsonOptions.h
)


if(WITH_AUDIOFILE)
  set(core_SOURCES
    ${core_SOURCES}
    io/SampleSourceAudiofile.c
  )
  set(core_HEADERS
    ${core_HEADERS}
    io/SampleSourceAudiofile.h
  )
  include_directories(${CMAKE_SOURCE_DIR}/vendor/audiofile/libaudiofile)
endif()

if(WITH_VST2X)
  set(core_SOURCES
    ${core_SOURCES}
    plugin/PluginPresetFxp.c
    plugin/PluginPresetInternalProgram.c
  )
  set(core_HEADERS
    ${core_HEADERS}
    plugin/PluginPresetFxp.h
    plugin/PluginPresetInternalProgram.h
  )
endif()

# Platform-specific sources
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(core_PLATFORM_SOURCES
    plugin/PluginVst2xLinux.cpp
  )

elseif(APPLE)
  set(core_PLATFORM_SOURCES
    base/PlatformInfoMac.m
    plugin/PluginVst3Mac.mm
  )
  if(WITH_VST2X)
    set(core_PLATFORM_SOURCES
      ${core_PLATFORM_SOURCES}
      plugin/PluginVst2xMac.mm
      plugin/PluginVst2xMacWindow.h
      plugin/PluginVst2xMacWindow.m
    )
  endif()

elseif(WIN32)
  set(core_PLATFORM_SOURCES
    plugin/PluginVst2xWindows.cpp
  )
endif()

#################
# Source Groups #
#################

source_group(app ".*/app/.*")
source_group(audio ".*/audio/.*")
source_group(base ".*/base/.*")
source_group(io ".*/io/.*")
source_group(logging ".*/logging/.*")
source_group(midi ".*/midi/.*")
source_group(plugin ".*/plugin/.*")
source_group(time ".*/time/.*")

#######################
# Build Configuration #
#######################

# VST SDK include directories (VST2.x is deprecated, but kept for compatibility)
if(WITH_VST2X AND EXISTS ${vst_include_DIR})
  include_directories(${vst_include_DIR})
endif()

# VST3 SDK include directories
set(VST3_SDK_AVAILABLE FALSE)
if(EXISTS ${vst3_include_DIR})
  include_directories(${vst3_include_DIR})
  include_directories(${vst3_public_sdk_DIR}/source)
  include_directories(${vst3_public_sdk_DIR}/source/vst)
  include_directories(${vst3_public_sdk_DIR}/source/vst/hosting)
  add_definitions(-DWITH_VST3_SDK=1)
  set(VST3_SDK_AVAILABLE TRUE)
  # Disable C++14 compatibility warnings for VST3 SDK headers
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-pre-c++14-compat)
  endif()
  message(STATUS "VST3 SDK includes: ${vst3_include_DIR}")
  message(STATUS "VST3 SDK public.sdk: ${vst3_public_sdk_DIR}")
else()
  message(STATUS "VST3 SDK not found - basic plugin loading will work, but audio processing requires SDK")
endif()

if(APPLE)
  set_source_files_properties(${core_PLATFORM_SOURCES}
    PROPERTIES COMPILE_FLAGS
    "-x objective-c++"
  )
endif()

##########
# Target #
##########

function(add_core_target wordsize)
  add_library(mrswatsoncore${wordsize} STATIC
    ${core_SOURCES}
    ${core_PLATFORM_SOURCES}
    ${core_HEADERS}
  )
  configure_target(mrswatsoncore${wordsize} ${wordsize})
  # VST3 SDK requires C++14 or later (base library uses C++17, but interfaces work with C++14)
  if(VST3_SDK_AVAILABLE)
    set_target_properties(mrswatsoncore${wordsize} PROPERTIES
      CXX_STANDARD 14
      CXX_STANDARD_REQUIRED ON
    )
    # Disable C++14 compatibility warnings for VST3 SDK headers
    # This must be done after configure_target to ensure flags are applied
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      target_compile_options(mrswatsoncore${wordsize} PRIVATE -Wno-pre-c++14-compat)
    endif()
    
    # Link VST3 SDK base library
    if(vst3_base_lib AND TARGET ${vst3_base_lib})
      target_link_libraries(mrswatsoncore${wordsize} PRIVATE ${vst3_base_lib})
      message(STATUS "Linking VST3 SDK base library (${vst3_base_lib}) to mrswatsoncore${wordsize}")
    endif()
  endif()
endfunction()

if(mw_BUILD_32)
  add_core_target(32)
endif()

if(mw_BUILD_64)
  add_core_target(64)
endif()
