#pragma once

#include <Core/Types.hpp>
#include <Core/String.hpp>
#include <Network/HttpClient.hpp>
#include <Utility/StringBuilder.hpp>

#include "DownloadManager.hpp"

namespace nbl::mc {

Result downloadRuntime(DownloadManager* client, StringView runtimeFolder, StringView runtimeComponent);

Result getRuntimePath(StringView runtimeFolder, StringView runtimeComponent, StringBuilder* out);

}
