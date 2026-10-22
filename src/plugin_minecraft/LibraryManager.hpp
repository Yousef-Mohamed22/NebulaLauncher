/*
 * Copyright 2026 jwall64
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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

