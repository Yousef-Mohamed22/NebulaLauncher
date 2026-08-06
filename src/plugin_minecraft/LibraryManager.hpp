#pragma once

#include <Core/String.hpp>
#include <Network/HttpClient.hpp>
#include <Codec/Json.hpp>
#include <Utility/StringBuilder.hpp>

#include "Arguments.hpp"
#include "DownloadManager.hpp"
#include "VersionManager.hpp"

#ifdef NBL_PLATFORM_WINDOWS
#define CLASSPATH_SEPARATOR ';'
#else
#define CLASSPATH_SEPARATOR ':'
#endif

namespace nbl::mc {


Result downloadLibraries(DownloadManager* client, StringView libraryFolder, const VersionRef& ref, FeatureSet features, AllocationFrame* classpath);
Result verifyStrict(StringView libraryFolder, const VersionRef& ref, FeatureSet features);


}

