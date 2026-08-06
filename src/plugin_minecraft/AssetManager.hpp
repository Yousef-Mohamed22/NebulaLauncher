#pragma once

#include <Core/String.hpp>
#include <Network/HttpClient.hpp>

#include "DownloadManager.hpp"

namespace nbl::mc {

Result downloadAssets(DownloadManager* client, StringView assetFolder, StringView id, StringView sha1, StringView url);

Result verifyStrict(StringView assetFolder, StringView id, StringView sha1);

}
