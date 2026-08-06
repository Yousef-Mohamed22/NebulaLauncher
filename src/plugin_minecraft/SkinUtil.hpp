#pragma once

#include <Core/Span.hpp>
#include <Core/Result.hpp>
#include <Core/String.hpp>
#include <Network/HttpClient.hpp>

namespace nbl::mc {

Result downloadSkinHead(StringView skinFolder, StringView url, HttpClient* client, StringView* outSkinId);

Result extractSkinId(StringView url, StringView* skinId);


}
