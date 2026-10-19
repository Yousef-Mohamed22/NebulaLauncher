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


/**
 * \file Plugin/InstallationType.cpp
 * \author jwall64
 * \brief Implementation of Plugin/InstallationType.hpp
 */

#include <Plugin/InstallationType.hpp>

#include <Core/Array.hpp>
#include <Core/Result.hpp>
#include <Platform/Logging.hpp>
#include <Plugin/Field.hpp>

namespace {

nbl::Array _fields = {
    nbl::Field {
        .type = nbl::FieldType::STRING,
        .key = "command",
        .name = "Command",
        .stringData = { },
        .flags = nbl::FieldFlag::APPLY_GAME_PLACEHOLDERS | 0
    },
    nbl::Field {
        .type = nbl::FieldType::STRING,
        .key = "working_dir",
        .name = "Working Directory",
        .stringData = { },
        .flags = nbl::FieldFlag::APPLY_GAME_PLACEHOLDERS | nbl::FieldFlag::STRICT_PLACEHOLDERS
    }
};

nbl::Result _launch(
        const nbl::InstallationType* type, 
        nbl::PluginContext* ctx, 
        nbl::Span<nbl::FieldValue> fields, 
        const nbl::InstallationMeta& meta, 
        nbl::ProcessTracker* tracker, 
        void* userData) {

    return tracker->run(ctx, fields[0].stringValue.resolved, fields[1].stringValue.resolved);
}

nbl::Result _getFolder(
        const nbl::InstallationType* type, 
        nbl::PluginContext* ctx, 
        nbl::Span<nbl::FieldValue> fields, 
        const nbl::InstallationMeta& meta, 
        nbl::StringView* outFolder,
        void* userData) {

    *outFolder = fields[1].stringValue.resolved;
    return nbl::Result::SUCCESS;
}

}

nbl::InstallationType nbl::InstallationType::COMMAND = {
    .id = "command",
    .name = "Command",
    .fields = _fields,
    .launchFunc = {
        .pfnLaunchInstallation = _launch,
        .userData = nullptr
    },
    .getFolderFunc = {
        .pfnGetFolder = _getFolder,
        .userData = nullptr
    }
};

