#pragma once

#include <Core/String.hpp>
#include <Core/Result.hpp>
#include <Codec/Json.hpp>
#include <Network/HttpClient.hpp>
#include <Utility/StringBuilder.hpp>

#include "DownloadManager.hpp"

namespace nbl::mc {
    
Result prepareLogging(StringView folder, JsonObject loggingInfo, DownloadManager* client, StringBuilder* outArg);

}
