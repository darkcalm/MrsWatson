cmake_minimum_required(VERSION 3.5)
project(vst3base)

# Build VST3 SDK base library without strict warnings
# This matches how the VST3 SDK itself builds these libraries
# Override parent's strict warning flags - VST3 SDK doesn't use -Werror

# Remove -Werror from CXX flags (inherited from parent)
string(REPLACE "-Werror" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
string(REPLACE "-Werror" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

set(VST3_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../vst3sdk")

add_library(vst3base64 STATIC
  ${VST3_SDK_ROOT}/base/source/baseiids.cpp
  ${VST3_SDK_ROOT}/base/source/fbuffer.cpp
  ${VST3_SDK_ROOT}/base/source/fdebug.cpp
  ${VST3_SDK_ROOT}/base/source/fdynlib.cpp
  ${VST3_SDK_ROOT}/base/source/fobject.cpp
  ${VST3_SDK_ROOT}/base/source/fstreamer.cpp
  ${VST3_SDK_ROOT}/base/source/fstring.cpp
  ${VST3_SDK_ROOT}/base/source/timer.cpp
  ${VST3_SDK_ROOT}/base/source/updatehandler.cpp
  ${VST3_SDK_ROOT}/base/thread/source/fcondition.cpp
  ${VST3_SDK_ROOT}/base/thread/source/flock.cpp
  ${VST3_SDK_ROOT}/pluginterfaces/base/conststringtable.cpp
  ${VST3_SDK_ROOT}/pluginterfaces/base/coreiids.cpp
  ${VST3_SDK_ROOT}/public.sdk/source/vst/vstinitiids.cpp
)

target_include_directories(vst3base64 PUBLIC ${VST3_SDK_ROOT})

set_target_properties(vst3base64 PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)

if(APPLE)
  set_target_properties(vst3base64 PROPERTIES OSX_ARCHITECTURES "x86_64")
endif()
