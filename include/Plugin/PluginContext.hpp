#pragma once

#include <Core/Types.hpp>
#include <Platform/Process.hpp>
#include <Platform/Pool.hpp>
#include <Core/String.hpp>

namespace nbl {

struct InstallationType;

class PluginContext {
public:

    const InstallationType* getInstallationType(StringView id) const;

    Result registerInstallationType(const InstallationType& type);

    Span<InstallationType> types() const;

    Result runCommand(i64 installationId, StringView commandLine, StringView workingDir, ProcessHandle* handle);

    StringView databasePath() const { return _mDatabasePath; }

    static Result create(StringView databasePath, PluginContext* ctx);

private:
    Arena _mArena;
    usize _mTypesOffset;
    usize _mNumTypes;
    StringView _mDatabasePath;
    StringView _mWrapperPath;
};


}
