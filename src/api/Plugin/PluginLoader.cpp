#include <Plugin/PluginLoader.hpp>

#include <Platform/StringBuilder.hpp>
#include <Platform/File.hpp>
#include <Plugin/PluginContext.hpp>
#include <Codec/Json.hpp>
#include <Core/Result.hpp>
#include <Platform/DynamicLibrary.hpp>

nbl::Result nbl::PluginLoader::loadAll() {

    nbl::FolderIterator it = {};
    NBL_CHECK(nbl::FolderIterator::create(_mPluginFolder, &it));

    StringView pluginFolder;
    while(it.next(&pluginFolder)) {

        if(pluginFolder == "." || pluginFolder == "..") continue;

        nbl::ArenaFrame frame(nbl::Arena::threadLocal());
        nbl::StringView fullPath = {};
        NBL_CHECK(nbl::StringBuilder(&frame).append(_mPluginFolder).append('/').append(pluginFolder).build(&fullPath));

        nbl::Result res = _load(fullPath);
        if(res.error()) {
            nbl::log(nbl::LogLevel::WARN, "Unable to load plugin at ", pluginFolder, " (error ", res, ")");
        }
    }

    nbl::log(nbl::LogLevel::INFO, "Loaded ", _mPluginRefs.length, " plugins");

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::PluginLoader::_load(StringView folder) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    StringView manifestPath;
    NBL_CHECK(StringBuilder(&frame).append(folder).append("/plugin.json").build(&manifestPath));

    FileChannel manifestStream;
    NBL_CHECK(FileChannel::open(manifestPath, &manifestStream, FileOpenFlag::READ));

    JsonElement value = {};
    NBL_CHECK(parseJson(manifestStream.reader(), &frame, &value));

    if(value.type != JsonType::OBJECT) return nbl::Result::INVALID_ARGUMENT;

    const JsonElement* idValue = value.object.get("id");
    const JsonElement* nameValue = value.object.get("name");
    const JsonElement* descriptionValue = value.object.get("description");
    const JsonElement* versionValue = value.object.get("version");

    if(
        idValue->type != JsonType::STRING || 
        nameValue->type != JsonType::STRING || 
        descriptionValue->type != JsonType::STRING || 
        versionValue->type != JsonType::STRING) {

        return nbl::Result::INVALID_ARGUMENT;
    }

    LoadedPlugin info = {};

    nbl::ArenaFrame dataFrame(&_mDataArena);
    NBL_CHECK(StringBuilder(&dataFrame).append(idValue->string).buildNullTerminated(&info.id));
    NBL_CHECK(StringBuilder(&dataFrame).append(nameValue->string).buildNullTerminated(&info.name));
    NBL_CHECK(StringBuilder(&dataFrame).append(descriptionValue->string).buildNullTerminated(&info.description));
    NBL_CHECK(StringBuilder(&dataFrame).append(versionValue->string).buildNullTerminated(&info.version));
    dataFrame.release();

    StringView libraryPath = {};
    NBL_CHECK(StringBuilder(&frame).append(folder).append("/plugin").append(dynamicLibraryFileExt()).build(&libraryPath));

    NBL_CHECK(nbl::DynamicLibrary::load(libraryPath, &info.lib));

    i32(*initFunc)(nbl::PluginContext*);
    NBL_CHECK(info.lib.findSymbol("initPlugin", &initFunc));

    i32 rc = initFunc(_pContext);
    if(rc != 0) {
        nbl::log(nbl::LogLevel::ERROR, "An error occurred while initializing a plugin! ", rc);
        return nbl::Result::PLUGIN_ERROR;
    }

    LoadedPlugin* ref = nullptr;
    NBL_CHECK(_mMetaArena.allocate(&ref));
    *ref = info;
    _mPluginRefs.length++;

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::PluginLoader::create(PluginContext* context, StringView pluginFolder, PluginLoader* out) {

    NBL_CHECK(nbl::makeDirectories(pluginFolder));
    NBL_CHECK(nbl::Arena::create(1024 * 1024 * 32, &out->_mDataArena));
    NBL_CHECK(nbl::Arena::create(1024 * 1024 * 4, &out->_mMetaArena));

    nbl::ArenaFrame frame(&out->_mDataArena);
    NBL_CHECK(nbl::StringBuilder(&frame).append(pluginFolder).build(&out->_mPluginFolder));

    out->_mPluginRefs.data = static_cast<LoadedPlugin*>(out->_mMetaArena.end());

    out->_pContext = context;

    return nbl::Result::SUCCESS;
}
