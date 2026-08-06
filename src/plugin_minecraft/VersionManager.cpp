#include "VersionManager.hpp"

#include <Platform/Arena.hpp>
#include <Platform/File.hpp>
#include <Codec/Json.hpp>
#include <Network/HttpClient.hpp>
#include <Core/Result.hpp>
#include <Platform/Allocator.hpp>
#include <Utility/StringBuilder.hpp>



nbl::StringView nbl::mc::VersionRef::id() const {
    if(parent && !root.has("id")) {
        return parent->id();
    }
    return root.getString("id");
}

nbl::StringView nbl::mc::VersionRef::assets() const {
if(parent && !root.has("assets")) {
        return parent->assets();
    }
    return root.getString("assets");
}
nbl::StringView nbl::mc::VersionRef::mainClass() const {
if(parent && !root.has("mainClass")) {
        return parent->mainClass();
    }
    return root.getString("mainClass");
}
nbl::StringView nbl::mc::VersionRef::type() const {
    if(parent && !root.has("type")) {
        return parent->type();
    }
    return root.getString("type");
}
nbl::JsonObject nbl::mc::VersionRef::assetIndex() const {
    if(parent && !root.has("assetIndex")) {
        return parent->assetIndex();
    }
    return root.getObject("assetIndex");
}
nbl::JsonObject nbl::mc::VersionRef::downloadInfo(StringView key, StringView* id) const {
    if(parent && (!root.has("downloads") || !root.getObject("downloads").has(key))) {
        return parent->downloadInfo(key, id);
    }
    *id = this->id();
    return root.getObject("downloads").getObject(key);
}
nbl::JsonObject nbl::mc::VersionRef::loggingInfo(StringView key) const {
    if(parent && (!root.has("logging") || !root.getObject("logging").has(key))) {
        return parent->loggingInfo(key);
    }
    return root.getObject("logging").getObject(key);

}
nbl::JsonObject nbl::mc::VersionRef::javaVersion() const {
    if(parent && !root.has("javaVersion")) {
        return parent->javaVersion();
    }
    return root.getObject("javaVersion");
}

nbl::Result nbl::mc::VersionRef::enumerateArguments(StringView key, std::function_ref<Result(JsonElement)> argumentConsumer) const {

    if(parent) {
        NBL_CHECK(parent->enumerateArguments(key, argumentConsumer));
    }
    for(const JsonElement& ele : root.getObject("arguments").getArray(key)) {
        NBL_CHECK(argumentConsumer(ele));
    }
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::mc::VersionRef::enumerateLibraries(std::function_ref<Result(JsonElement)> libraryConsumer) const {
    if(parent) {
        NBL_CHECK(parent->enumerateLibraries(libraryConsumer));
    }
    for(const JsonElement& ele : root.getArray("libraries")) {
        NBL_CHECK(libraryConsumer(ele));
    }
    return nbl::Result::SUCCESS;
}



nbl::Result nbl::mc::VersionManager::open(JsonObject manifestJson, StringView version, DownloadManager* client, LinearAllocator* allocator, VersionRef* outRef) {

    nbl::AllocationFrame frame(nbl::Arena::threadLocal());

    nbl::StringView path{};
    nbl::StringBuilder(&frame).append(_mFolderPath).append('/').append(version).append(".json").build(&path);

    nbl::FileChannel file{};
    nbl::Result res = nbl::FileChannel::open(path, &file, FileOpenFlag::READ);
    if(res == nbl::Result::NOT_FOUND) {
        res = _download(manifestJson, version, path, client, &file);
        if(res.error()) return res;
    }

    JsonElement ele{};
    NBL_CHECK(nbl::parseJson(file.reader(), allocator, &ele));

    if(ele.type != JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;
    
    StringView parent = ele.object.getString("inheritsFrom");
    if(!parent.empty()) {
        NBL_CHECK(allocator->allocate(&outRef->parent));
        NBL_CHECK(open(manifestJson, parent, client, allocator, outRef->parent));
    }
    outRef->root = ele.object;

    return res;
}

nbl::Result nbl::mc::VersionManager::_download(JsonObject manifestJson, StringView version, StringView path, DownloadManager* client, FileChannel* outFile) {

    nbl::AllocationFrame frame(nbl::Arena::threadLocal());
    nbl::Result res = nbl::Result::SUCCESS;
    

    if(version == "latest" || version == "latest_release" || version == "latest_snapshot") {
        
        const JsonElement* latestObj = manifestJson.get("latest");
        if(latestObj == nullptr || latestObj->type != JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;

        const JsonElement* release = latestObj->object.get(version == "latest_snapshot" ? "snapshot" : "release");
        if(release == nullptr || release->type != JsonType::STRING) return nbl::Result::FORMAT_ERROR;

        version = release->string;
        nbl::StringBuilder(&frame).append(_mFolderPath).append('/').append(version).append(".json").build(&path);
        
        res = nbl::FileChannel::open(path, outFile, FileOpenFlag::READ);
        if(res != nbl::Result::NOT_FOUND) return res;
    }

    const JsonElement* versionsArr = manifestJson.get("versions");
    if(versionsArr == nullptr || versionsArr->type != JsonType::ARRAY) return nbl::Result::FORMAT_ERROR;

    nbl::StringView url{};
    nbl::StringView sha1{};
    for(const JsonElement& ele : versionsArr->array) {
        if(ele.type != JsonType::OBJECT) continue;

        StringView id = ele.object.getString("id");
        if(id != version) continue;

        url = ele.object.getString("url");
        sha1 = ele.object.getString("sha1");
    }

    if(url.empty()) return nbl::Result::NOT_FOUND;
    if(sha1.empty()) return nbl::Result::IO_ERROR;

    NBL_CHECK(client->verifyOrDownloadNow(sha1, path, url));

    return nbl::FileChannel::open(path, outFile, FileOpenFlag::READ);
}
