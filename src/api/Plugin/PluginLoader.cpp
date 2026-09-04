#include <Plugin/PluginLoader.hpp>

#include <Core/Result.hpp>
#include <Core/Stream.hpp>
#include <Platform/StringBuilder.hpp>
#include <Platform/File.hpp>
#include <Platform/DynamicLibrary.hpp>
#include <Platform/Allocator.hpp>
#include <Codec/Json.hpp>
#include <Plugin/PluginInitContext.hpp>
#include <Plugin/PluginContext.hpp>
#include <Plugin/InstallationType.hpp>

nbl::Result nbl::PluginLoader::loadAll() {

    nbl::FolderIterator it{};
    NBL_CHECK(nbl::FolderIterator::create(_mPluginFolder, &it));

    nbl::Arena registrationsTempArena{};
    NBL_CHECK(nbl::Arena::create(1024 * 1024 * 64, &registrationsTempArena));

    {
        PluginInitContext builtin{};
        LoadedPlugin builtinPlugin{};
        NBL_CHECK(PluginInitContext::create(&registrationsTempArena, &builtinPlugin, _pContext->databaseConnection(), _pContext->databasePath(), _pContext->pluginDataPath(), &builtin));

        builtin.registerInstallationType(&InstallationType::COMMAND);
    }

    StringView pluginFolder;
    while(it.next(&pluginFolder)) {

        if(pluginFolder == "." || pluginFolder == "..") continue;

        nbl::log(nbl::LogLevel::DEBUG, "Attempting to load plugin in folder ", pluginFolder);

        nbl::ArenaFrame frame(nbl::Arena::threadLocal());
        nbl::StringView fullPath = {};
        NBL_CHECK(nbl::StringBuilder(&frame).append(_mPluginFolder).append('/').append(pluginFolder).build(&fullPath));

        nbl::Result res = _load(fullPath, &registrationsTempArena);
        if(res.error()) {
            nbl::log(nbl::LogLevel::WARN, "Unable to load plugin at ", pluginFolder, " (error ", res, ")");
        }
    }

    // Process registrations
    if(registrationsTempArena.index() > 0) {

        usize registryCounts[PluginRegistries::count()];
        usize offsets[PluginRegistries::count()];
        usize totalSize = 0;

        {
            u8* current = static_cast<u8*>(registrationsTempArena.begin());
            while(current < registrationsTempArena.end()) {
                PluginRegistries::Registry reg = *reinterpret_cast<PluginRegistries::Registry*>(current);
                current += sizeof(PluginRegistries::Registry);

                registryCounts[reg]++;
                current += sizeof(void*);
                totalSize += sizeof(void*);
            }
        }

        u8* allocation;
        NBL_CHECK(_pContext->_mArena.push(totalSize, reinterpret_cast<void**>(&allocation)));


        for(usize i = 0 ; i < PluginRegistries::count() ; i++) {

            usize thisSize = registryCounts[i] * sizeof(void*);
            totalSize -= thisSize;
            offsets[i] = totalSize;
            _pContext->_pStarts[i] = allocation + offsets[i];
            _pContext->_mCounts[i] = registryCounts[i];
        }

        {
            u8* current = static_cast<u8*>(registrationsTempArena.begin());
            while(current < registrationsTempArena.end()) {
                PluginRegistries::Registry reg = *reinterpret_cast<PluginRegistries::Registry*>(current);
                current += sizeof(PluginRegistries::Registry);

                usize realOffset = offsets[reg] + (--registryCounts[reg]) * sizeof(void*);
                memcpy(allocation + realOffset, current, sizeof(void*));


                current += sizeof(void*);
            }
        }
    }
 
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::PluginLoader::_load(StringView folder, LinearAllocator* registrations) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    StringView manifestPath{};
    NBL_CHECK(StringBuilder(&frame).append(folder).append("/plugin.json").build(&manifestPath));

    FileChannel manifestStream;
    NBL_CHECK(FileChannel::open(manifestPath, &manifestStream, FileOpenFlag::READ));

    JsonElement value = {};
    NBL_CHECK(parseJson(manifestStream.reader(), &frame, &value));
    if(value.type != JsonType::OBJECT) return nbl::Result::INVALID_ARGUMENT;

    StringView id = value.object.getString("id");
    StringView name = value.object.getString("name");
    StringView description = value.object.getString("description");
    StringView version = value.object.getString("version");

    if(id.empty() || name.empty() || version.empty()) return nbl::Result::INVALID_ARGUMENT;

    LoadedPlugin lp = {};

    NBL_CHECK(StringBuilder(&_mDataArena).append(id).buildNullTerminated(&lp.meta.id));
    NBL_CHECK(StringBuilder(&_mDataArena).append(name).buildNullTerminated(&lp.meta.name));
    NBL_CHECK(StringBuilder(&_mDataArena).append(description).buildNullTerminated(&lp.meta.description));
    NBL_CHECK(StringBuilder(&_mDataArena).append(version).buildNullTerminated(&lp.meta.version));

    StringView libraryPath = {};
    NBL_CHECK(StringBuilder(&frame).append(folder).append("/plugin").append(dynamicLibraryFileExt()).build(&libraryPath));

    NBL_CHECK(nbl::DynamicLibrary::load(libraryPath, &lp.lib));

    i32(*initFunc)(nbl::PluginInitContext*);
    NBL_CHECK(lp.lib.findSymbol("initPlugin", &initFunc));

    PluginInitContext ctx;
    NBL_CHECK(PluginInitContext::create(registrations, &lp, _pContext->databaseConnection(), _pContext->databasePath(), _pContext->pluginDataPath(), &ctx));

    i32 rc = initFunc(&ctx);
    if(rc != 0) {
        nbl::log(nbl::LogLevel::ERROR, "An error occurred while initializing a plugin! ", rc);
        return nbl::Result::PLUGIN_ERROR;
    }

    LoadedPlugin* ref = nullptr;
    NBL_CHECK(_mMetaArena.allocate(&ref));
    *ref = lp;
    _mPluginRefs.length++;

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::PluginLoader::create(PluginContext* context, StringView pluginFolder, PluginLoader* out) {

    NBL_CHECK(nbl::makeDirectories(pluginFolder));
    NBL_CHECK(nbl::Arena::create(1024 * 1024 * 32, &out->_mDataArena));
    NBL_CHECK(nbl::Arena::create(1024 * 1024 * 4, &out->_mMetaArena));

    NBL_CHECK(nbl::StringBuilder(&out->_mDataArena).append(pluginFolder).build(&out->_mPluginFolder));

    out->_mPluginRefs.data = static_cast<LoadedPlugin*>(out->_mMetaArena.end());

    out->_pContext = context;

    return nbl::Result::SUCCESS;
}
