#pragma once

#include <Core/Types.hpp>
#include <Core/String.hpp>

#include <Platform/File.hpp>
#include <Network/HttpClient.hpp>
#include <Codec/Json.hpp>
#include <Platform/Allocator.hpp>

#include "DownloadManager.hpp"

namespace nbl::mc {


struct VersionRef {
public:

    StringView id() const;
    StringView assets() const;
    StringView mainClass() const;
    StringView type() const;

    JsonObject assetIndex() const;
    JsonObject downloadInfo(StringView key, StringView* id) const;
    JsonObject loggingInfo(StringView key) const;
    JsonObject javaVersion() const;

    Result enumerateArguments(StringView key, std::function_ref<Result(JsonElement)> argumentConsumer) const;
    Result enumerateLibraries(std::function_ref<Result(JsonElement)> libraryConsumer) const;

    VersionRef* parent;
    JsonObject root;

    friend class VersionManager;
};


class VersionManager {
public:

    Result open(JsonObject manifestJson, StringView version,  DownloadManager* client, LinearAllocator* allocator, VersionRef* outRef);

    static Result create(StringView folder, VersionManager* out);

private:

    Result _download(JsonObject manifestJson, StringView version, StringView suggestedPath, DownloadManager* client, FileChannel* outFile);

    nbl::StringView _mFolderPath;
};


inline Result VersionManager::create(StringView folder, VersionManager* out) {
    out->_mFolderPath = folder;
    NBL_CHECK(nbl::makeDirectories(folder));
    return nbl::Result::SUCCESS;
}


}
