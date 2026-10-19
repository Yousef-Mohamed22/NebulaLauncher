/*
 * Copyright 2026 jwall64
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/**
 * \file Plugin/PluginContext.hpp
 * \author jwall64
 * \brief The bridge between plugins and the application itself
 */

#pragma once

#include <Core/Types.hpp>
#include <Platform/Process.hpp>
#include <Platform/Pool.hpp>
#include <Core/String.hpp>
#include <Core/Optional.hpp>
#include <Data/Database.hpp>
#include <Plugin/Registry.hpp>
#include <Plugin/PluginLoader.hpp>
#include "Data/MediaManager.hpp"

namespace nbl {

enum NotificationType {
    INFO,
    WARNING,
    ERROR
};

struct NotificationConsumer {
    void(*pfnNotify)(void*,NotificationType,StringView,StringView,Optional<StringView>,Optional<u32>);
    void* userData;

    void notify(NotificationType type, StringView title, StringView message, Optional<StringView> icon, Optional<u32> duration) {
        if(!pfnNotify) return;
        pfnNotify(userData,type,title,message,icon,duration);
    }
};

struct AccountProvider;
struct InstallationType;
struct MetadataProvider;
struct GameProvider;

class PluginContext {
public:

    template<typename T>
    inline ConstSpan<const T*> registry() {
        NBL_ASSERT(_mLoader.awaitLoaded().success());
        return _mRegistry.get<T>();
    }

    template<typename T>
    inline const T* get(StringView name) {
        NBL_ASSERT(_mLoader.awaitLoaded().success());
        return _mRegistry.get<T>(name);
    }

    Result runCommand(i64 installationId, StringView commandLine, StringView workingDir, ProcessHandle* handle);

    void pushNotification(NotificationType type, StringView title, StringView message) {
        _mNotificationConsumer.notify(type, title, message, {}, {});
    }

    void pushNotification(NotificationType type, StringView title, StringView message, Optional<StringView> icon, Optional<u32> duration) {
        _mNotificationConsumer.notify(type, title, message, icon, duration);
    }

    inline DatabaseConnection* databaseConnection() { return &_mConnection; }
    
    inline MediaManager* mediaManager() { return &_mMediaManager; }
    inline const MediaManager* mediaManager() const { return &_mMediaManager; }

    inline StringView databasePath() const { return _mDatabasePath; }
    inline StringView pluginDataPath() const { return _mPluginDataPath; }

    inline Span<LoadedPlugin> loadedPlugins() { 
        return _mLoader.loadedPlugins();
    }

    inline Result beginLoad() {
        return _mLoader.beginLoadAsync();
    }

    static Result create(StringView deviceName, StringView databasePath, StringView mediaDbPath, StringView pluginFolder, StringView pluginDataPath, NotificationConsumer notificationConsumer, PluginContext* ctx);

private:
    Arena _mArena;
    Registry _mRegistry;

    StringView _mDatabasePath;
    StringView _mPluginDataPath;
    StringView _mWrapperPath;

    PluginLoader _mLoader;
    MediaManager _mMediaManager;

    NotificationConsumer _mNotificationConsumer;
    DatabaseConnection _mConnection;

    friend class PluginLoader;
};


}
