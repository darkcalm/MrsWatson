//
// PluginVst3Mac.mm - MrsWatson
// Created for VST3 support
// Copyright (c) 2025. All rights reserved.
//

#if MACOSX
#include "base/CharString.h"
#include "base/LinkedList.h"
#include "logging/EventLogger.h"
#include "plugin/Plugin.h"
#include <Foundation/Foundation.h>
#include <CoreFoundation/CFBundle.h>
#include <dlfcn.h>

extern "C" {

    LinkedList getVst3PluginLocations(CharString currentDirectory);
    LinkedList getVst3PluginLocations(CharString currentDirectory)
    {
        LinkedList locations = newLinkedList();
        char *home = NULL;
        CharString locationBuffer;

        linkedListAppend(locations, currentDirectory);

        locationBuffer = newCharString();
        snprintf(locationBuffer->data, (size_t)(locationBuffer->capacity), "/Library/Audio/Plug-Ins/VST3");
        linkedListAppend(locations, locationBuffer);

        home = getenv("HOME");
        if (home == NULL) {
            logWarn("Could not get $HOME environment variable");
        } else {
            locationBuffer = newCharString();
            snprintf(locationBuffer->data, (size_t)(locationBuffer->capacity), "%s/Library/Audio/Plug-Ins/VST3", home);
            linkedListAppend(locations, locationBuffer);
        }

        return locations;
    }

    void* getLibraryHandleForVst3Plugin(const CharString pluginAbsolutePath);
    void* getLibraryHandleForVst3Plugin(const CharString pluginAbsolutePath)
    {
        // VST3 plugins are bundles, get the executable path
        CFStringRef pluginPathStringRef = CFStringCreateWithCString(NULL, pluginAbsolutePath->data, kCFStringEncodingUTF8);
        CFURLRef bundleUrl = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, pluginPathStringRef, kCFURLPOSIXPathStyle, true);

        if (bundleUrl == NULL) {
            logError("Couldn't make URL reference for VST3 plugin");
            return NULL;
        }

        CFBundleRef bundleRef = CFBundleCreate(kCFAllocatorDefault, bundleUrl);
        if (bundleRef == NULL) {
            logError("Couldn't create bundle reference for VST3 plugin");
            CFRelease(pluginPathStringRef);
            CFRelease(bundleUrl);
            return NULL;
        }

        // Get the executable path
        CFURLRef executableUrl = CFBundleCopyExecutableURL(bundleRef);
        if (executableUrl == NULL) {
            logError("Couldn't get executable URL from VST3 bundle");
            CFRelease(pluginPathStringRef);
            CFRelease(bundleUrl);
            CFRelease(bundleRef);
            return NULL;
        }

        // Use CFURLGetFileSystemRepresentation for proper path conversion
        char executablePathC[PATH_MAX + 1];
        Boolean success = CFURLGetFileSystemRepresentation(executableUrl, true, (UInt8*)executablePathC, PATH_MAX);

        void* handle = NULL;
        if (success) {
            logDebug("Attempting to load VST3 executable: %s", executablePathC);
            handle = dlopen(executablePathC, RTLD_LAZY);
            if (handle == NULL) {
                const char* dlerr = dlerror();
                logError("Could not load VST3 plugin executable '%s': %s", executablePathC, dlerr ? dlerr : "unknown error");
            } else {
                logDebug("Successfully loaded VST3 plugin executable");
            }
        } else {
            logError("Could not get file system representation of executable URL");
        }

        CFRelease(executableUrl);
        CFRelease(pluginPathStringRef);
        CFRelease(bundleUrl);
        CFRelease(bundleRef);

        return handle;
    }

    void* getVst3PluginFactory(void* libraryHandle);
    void* getVst3PluginFactory(void* libraryHandle)
    {
        if (libraryHandle == NULL) {
            return NULL;
        }
        
        // VST3 plugins export GetPluginFactory function
        typedef void* (*GetPluginFactoryFunc)(void);
        
        union {
            GetPluginFactoryFunc funcPtr;
            void* voidPtr;
        } factoryFunc;
        
        factoryFunc.voidPtr = dlsym(libraryHandle, "GetPluginFactory");
        
        if (factoryFunc.voidPtr == NULL) {
            logError("Could not find GetPluginFactory function in VST3 plugin");
            return NULL;
        }
        
        void* factory = factoryFunc.funcPtr();
        if (factory == NULL) {
            logError("GetPluginFactory returned NULL");
            return NULL;
        }
        
        logDebug("Successfully obtained VST3 plugin factory");
        return factory;
    }

    void closeVst3LibraryHandle(void* libraryHandle);
    void closeVst3LibraryHandle(void* libraryHandle)
    {
        if (libraryHandle != NULL) {
            dlclose(libraryHandle);
        }
    }

} // extern "C"
#endif
