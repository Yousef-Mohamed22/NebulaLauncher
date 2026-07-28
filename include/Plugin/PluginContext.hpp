#pragma once

#include <Core/Types.hpp>
#include <Platform/Process.hpp>
#include <Platform/Pool.hpp>
#include <Core/String.hpp>
#include <Core/Optional.hpp>

namespace nbl {

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

struct InstallationType;

class PluginContext {
public:

    const InstallationType* getInstallationType(StringView id) const;

    Result registerInstallationType(const InstallationType& type);

    Span<InstallationType> types() const;

    Result runCommand(i64 installationId, StringView commandLine, StringView workingDir, ProcessHandle* handle);

    void pushNotification(NotificationType type, StringView title, StringView message) {
        _mNotificationConsumer.notify(type, title, message, {}, {});
    }

    void pushNotification(NotificationType type, StringView title, StringView message, Optional<StringView> icon, Optional<u32> duration) {
        _mNotificationConsumer.notify(type, title, message, icon, duration);
    }

    StringView databasePath() const { return _mDatabasePath; }

    static Result create(StringView databasePath, NotificationConsumer notificationConsumer, PluginContext* ctx);

private:
    Arena _mArena;
    usize _mTypesOffset;
    usize _mNumTypes;
    StringView _mDatabasePath;
    StringView _mWrapperPath;
    NotificationConsumer _mNotificationConsumer;
};


}
