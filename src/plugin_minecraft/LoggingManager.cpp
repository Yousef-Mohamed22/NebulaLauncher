#include "LoggingManager.hpp"

#include <Platform/File.hpp>
#include <Utility/Placeholder.hpp>
#include <Utility/Hash.hpp>

nbl::Result nbl::mc::prepareLogging(StringView folder, JsonObject loggingInfo, DownloadManager* client, StringBuilder* outArg) {

    StringView argument = loggingInfo.getString("argument");
    JsonObject file = loggingInfo.getObject("file");
    StringView id = file.getString("id");

    StringView path{};
    char pathBuf[4096];
    InPlaceAllocator alloc(pathBuf, 4096);
    NBL_CHECK(StringBuilder(&alloc).append(folder).append('/').append(id).buildNullTerminated(&path));
    NBL_CHECK(client->verifyOrDownload(file.getString("sha1"), path, file.getString("url")));
  
    return nbl::formatPlaceholders(argument, outArg, nbl::Array{
        nbl::Placeholder{.key="path", .value=path}
    });
}
