#pragma once

#include <Core/Types.hpp>
#include <Platform/Process.hpp>
#include <Platform/Pool.hpp>
#include <Core/String.hpp>
#include <Core/Optional.hpp>

namespace nbl {

class PluginRegistries {
public:

    enum Registry : usize {
        INSTALLATION_TYPE,
        ACCOUNT_PROVIDER,
        _END
    };

    constexpr static usize count() { return _END; }

};

enum NotificationType {
    INFO,
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

class PluginContext {
public:

    const InstallationType* getInstallationType(StringView id) const;
    ConstSpan<const InstallationType*> types() const;

    const AccountProvider* getAccountProvider(StringView id) const;
    ConstSpan<const AccountProvider*> accountProviders() const;

    Result runCommand(i64 installationId, StringView commandLine, StringView workingDir, ProcessHandle* handle);

    void pushNotification(NotificationType type, StringView title, StringView message) {
        _mNotificationConsumer.notify(type, title, message, {}, {});
    }

    void pushNotification(NotificationType type, StringView title, StringView message, Optional<StringView> icon, Optional<u32> duration) {
        _mNotificationConsumer.notify(type, title, message, icon, duration);
    }

    inline StringView databasePath() const { return _mDatabasePath; }
    inline StringView pluginDataPath() const { return _mPluginDataPath; }

    static Result create(StringView databasePath, StringView pluginDataPath, NotificationConsumer notificationConsumer, PluginContext* ctx);

private:
    Arena _mArena;

    void* _pStarts[PluginRegistries::count()];
    usize _mCounts[PluginRegistries::count()];

    StringView _mDatabasePath;
    StringView _mPluginDataPath;
    StringView _mWrapperPath;

    NotificationConsumer _mNotificationConsumer;

    friend class PluginLoader;
};


}
