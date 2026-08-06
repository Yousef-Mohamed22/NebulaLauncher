#include "MinecraftLauncher.hpp"
#include "DownloadManager.hpp"
#include "VersionManager.hpp"
#include "LoggingManager.hpp"
#include "AssetManager.hpp"
#include "LibraryManager.hpp"
#include "RuntimeManager.hpp"
#include "ModLoader.hpp"
#include "AuthManager.hpp"

#include <Network/HttpClient.hpp>
#include <Platform/File.hpp>
#include <Platform/Time.hpp>
#include <Platform/Logging.hpp>
#include <Plugin/Field.hpp>
#include <Plugin/PluginContext.hpp>
#include <Utility/Hash.hpp>
#include <Utility/Placeholder.hpp>
#include <Utility/StringBuilder.hpp>


nbl::Result nbl::mc::MinecraftLauncher::launch(nbl::PluginContext* ctx, StringView version, const OnlineAccount& account, ModLoader modLoader, StringView workingDir, i64 installationId, ProcessTracker* tracker) {

    if(!_mVersionManifest.hasValue()) {
        u32 failures = 0;
        _mHttpClient.awaitAll(&failures);
        if(failures > 0) {
            nbl::log(nbl::LogLevel::WARN, "An error occurred while downloading the version manifest. Attempting to use cached copy. There were ", failures, " failures");
        }

        nbl::FileChannel channel;
        AllocationFrame frame(Arena::threadLocal());
        StringView manifestPath;
        NBL_CHECK(StringBuilder(&frame).append(_mRootFolder).append("/version_manifest_v2.json").buildNullTerminated(&manifestPath));
        NBL_CHECK(FileChannel::open(manifestPath, &channel, FileOpenFlag::READ));

        JsonElement element{};
        NBL_CHECK(nbl::parseJson(channel.reader(), &_mDataArena, &element));
        if(element.type != JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;

        _mVersionManifest = element.object;
    }


    FeatureSet features{};

    nbl::AllocationFrame frame(&_mDataArena);

    DownloadManager downloadManager{};
    NBL_CHECK(DownloadManager::create(6144, 2, 1, 5200, tracker, &downloadManager));

    nbl::StringView folderPath{};
    nbl::StringBuilder(&frame).append(_mRootFolder).append("/versions").build(&folderPath);

    nbl::mc::VersionManager manager{};
    NBL_CHECK(nbl::mc::VersionManager::create(folderPath, &manager));

    nbl::mc::VersionRef baseRef{};
    NBL_CHECK(manager.open(*_mVersionManifest, version, &downloadManager, &frame, &baseRef));
 
    nbl::mc::VersionRef ref = baseRef;

    if(modLoader == ModLoader::FABRIC) {
        ref.parent = &baseRef;
        NBL_CHECK(generateFabricVersionMeta(&ref, _mRootFolder, &downloadManager, &frame));
    }

    JsonObject logger = ref.loggingInfo("client");
    if(logger.entryCount == 0) return nbl::Result::FORMAT_ERROR;


    nbl::StringView loggingFolderPath{};
    nbl::StringBuilder(&frame).append(_mRootFolder).append("/logging").build(&loggingFolderPath);

    nbl::StringView assetFolderPath{};
    nbl::StringBuilder(&frame).append(_mRootFolder).append("/assets").build(&assetFolderPath);

    nbl::StringView libraryFolderPath{};
    nbl::StringBuilder(&frame).append(_mRootFolder).append("/libraries").build(&libraryFolderPath);

    nbl::StringView runtimeFolderPath{};
    nbl::StringBuilder(&frame).append(_mRootFolder).append("/runtime").build(&runtimeFolderPath);

    StringView clientId;
    JsonObject downloadInfo = ref.downloadInfo("client", &clientId);
    StringView clientJarPath;
    StringBuilder(&frame).append(folderPath).append("/client-").append(clientId).append(".jar").buildNullTerminated(&clientJarPath);

    downloadManager.verifyOrDownload(
            downloadInfo.getString("sha1"), 
            clientJarPath, 
            downloadInfo.getString("url"));

    tracker->pushStatus("Verifying assets...");

    nbl::StringBuilder logArgBuilder(&frame);
    NBL_CHECK(nbl::mc::prepareLogging(loggingFolderPath, logger, &downloadManager, &logArgBuilder));
    nbl::StringView logArg;
    NBL_CHECK(logArgBuilder.build(&logArg));

    JsonObject assetObject = ref.assetIndex(); ;
    NBL_CHECK(nbl::mc::downloadAssets(&downloadManager, assetFolderPath, assetObject.getString("id"), assetObject.getString("sha1"), assetObject.getString("url")));

    char* classpathStart = static_cast<char*>(frame.end());
    NBL_CHECK(nbl::mc::downloadLibraries(&downloadManager, libraryFolderPath, ref, features, &frame));

    frame.writer().writeChar(CLASSPATH_SEPARATOR);
    frame.writer().writeString(clientJarPath);

    usize classpathLength = static_cast<char*>(frame.end()) - classpathStart;
    StringView classpath = StringView(classpathStart, classpathLength);
    


    StringView runtimeComponent = ref.javaVersion().getString("component");
    NBL_CHECK(nbl::mc::downloadRuntime(&downloadManager, runtimeFolderPath, runtimeComponent));

    tracker->pushStatus("Downloading assets...");
    NBL_CHECK(downloadManager.awaitAll());

    tracker->pushStatus("Preparing command...", 1.0);

    StringView versionId = ref.id();
    StringView uuid{};
    StringView accessToken{};
    {
        JsonElement fieldsElement{};
        NBL_CHECK(nbl::parseJson(StringReader(account.fields), &frame, &fieldsElement));

        Span<FieldValue> outValues{};
        nbl::decodeFieldValues(ctx, MC_AUTH_FIELDS, fieldsElement.asObject(), &outValues, &frame);

        uuid = outValues[1].stringValue.raw;
        accessToken = outValues[2].stringValue.raw;
    }


    nbl::Array placeholders = {
        Placeholder{ "auth_player_name", account.username }, 
        Placeholder{ "auth_uuid", uuid },
        Placeholder{ "auth_access_token", accessToken },
        Placeholder{ "auth_xuid", "" }, // Note: This doesn't seem to be obtainable without signing in twice... It's only used for telemetry anyway, and passing an empty argument will still work. Perhaps the --xuid and ${auth_xuid} arguments should be blacklisted? Or maybe the wrapper should be reconfigured to accept separate arguments rather than a command string?
        Placeholder{ "clientid", "MzYwY2E3ZjctOTI2YS00NTRmLTgzNDgtZjdlOTYxOGI4NzVkCg" }, // TODO: Generate a proper client ID, store it
        Placeholder{ "version_name", versionId },
        Placeholder{ "game_directory", workingDir },
        Placeholder{ "assets_root", assetFolderPath },
        Placeholder{ "assets_index_name", ref.assets() },
        Placeholder{ "version_type", ref.type() },
        Placeholder{ "resolution_width", "0" },
        Placeholder{ "resolution_height", "0" },
        Placeholder{ "quickPlayPath", "" },
        Placeholder{ "quickPlaySingleplayer", "" },
        Placeholder{ "quickPlayMultiplayer", "" },
        Placeholder{ "quickPlayRealms", "" },
        Placeholder{ "natives_directory", runtimeFolderPath },
        Placeholder{ "launcher_name", "Nebula" },
        Placeholder{ "launcher_version", "0.1.0" },
        Placeholder{ "classpath", classpath }
    };

    StringBuilder command(&frame);
    NBL_CHECK(getRuntimePath(runtimeFolderPath, runtimeComponent, &command));

    NBL_CHECK(appendArgs(ref, "jvm", &command, placeholders, features));
    NBL_CHECK(command.append(' ').appendChecked(logArg));

    NBL_CHECK(appendArgs(ref, "default-user-jvm", &command, placeholders, features)); // TODO: Skip this if the user has supplied args
    
    command.append(' ').append(ref.mainClass());

    NBL_CHECK(appendArgs(ref, "game", &command, placeholders, features));

    StringView finalCmd;
    NBL_CHECK(command.buildNullTerminated(&finalCmd));

    nbl::log(nbl::LogLevel::DEBUG, finalCmd);

    tracker->run(ctx, finalCmd, workingDir);

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::mc::MinecraftLauncher::init(StringView rootFolder, MinecraftLauncher* launcher) {

    NBL_CHECK(nbl::makeDirectories(rootFolder));
    NBL_CHECK(nbl::Arena::create(512 * 1024 * 1024, &launcher->_mDataArena));
    NBL_CHECK(nbl::HttpClient::create(4, 1, 1, 1, &launcher->_mHttpClient));


    u64 maxFilesCur;
    u64 maxFilesMax;
    NBL_CHECK(nbl::getOpenFileLimit(&maxFilesCur, &maxFilesMax));
    NBL_CHECK(nbl::setOpenFileLimit(maxFilesMax));


    char* begin;
    NBL_CHECK(launcher->_mDataArena.allocate(&begin, rootFolder.length));
    memcpy(begin, rootFolder.data, rootFolder.length);

    launcher->_mRootFolder = StringView(begin, rootFolder.length);

    StringView authPath;
    NBL_CHECK(StringBuilder(&launcher->_mDataArena).append(rootFolder).append("/auth.json").buildNullTerminated(&authPath));

    AllocationFrame frame(Arena::threadLocal());
    StringView manifestPath;
    NBL_CHECK(StringBuilder(&frame).append(rootFolder).append("/version_manifest_v2.json").buildNullTerminated(&manifestPath));

    if(!nbl::fileExists(manifestPath)) {
        NBL_CHECK(launcher->_mHttpClient.downloadFile("https://piston-meta.mojang.com/mc/game/version_manifest_v2.json", manifestPath));
    } else {
        u64 modifyTime = nbl::modifyTime(manifestPath);
        if(nbl::getCurrentTime() - modifyTime > 1000 * 60 * 60) {
            NBL_CHECK(launcher->_mHttpClient.downloadFile("https://piston-meta.mojang.com/mc/game/version_manifest_v2.json", manifestPath, {
                .ifModifiedSince = modifyTime
            }));
        }
    }

    return nbl::Result::SUCCESS;
}
