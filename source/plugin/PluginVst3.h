//
// PluginVst3.h - MrsWatson
// Copyright (c) 2025. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

#ifndef MrsWatson_PluginVst3_h
#define MrsWatson_PluginVst3_h

#include "base/CharString.h"
#include "plugin/Plugin.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * List all available VST3 plugins in common system locations.
 * @param pluginRoot User-provided plugin root path to search
 */
void listAvailablePluginsVst3(const CharString pluginRoot);

/**
 * Create a new instance of a VST3 plugin
 * @param pluginName Plugin name
 * @param pluginRoot User-defined plugin root path
 * @return Initialized Plugin object, or NULL if no such plugin was found
 */
Plugin newPluginVst3(const CharString pluginName, const CharString pluginRoot);

/**
 * See if a VST3 plugin exists with the given name. Absolute paths will also
 * be respected if passed.
 * @param pluginName Plugin name (short name or absolute path)
 * @param pluginRoot User-provided plugin root path
 * @return True if such a plugin exists in any location, false otherwise
 */
boolByte pluginVst3Exists(const CharString pluginName,
                          const CharString pluginRoot);

/**
 * Set a parameter within a VST3 plugin
 * @param self
 * @param index Parameter index
 * @param value New value
 * @return True if the parameter could be set
 */
boolByte pluginVst3SetParameter(Plugin self, unsigned int index, float value);

#ifdef __cplusplus
}
#endif

#endif
