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

#include "VersionManager.hpp"

#include <Core/String.hpp>
#include <Core/Optional.hpp>
#include <Codec/Json.hpp>
#include <Utility/StringBuilder.hpp>
#include <Utility/Placeholder.hpp>

namespace nbl::mc {


enum class Architecture {
    X86,
    AMD64,
    AARCH64
};

Result parseArchitecture(StringView name, Architecture* outAction);
StringView architectureName(Architecture action);


enum class RuleAction {
    ALLOW,
    DISALLOW
};


Result parseRuleAction(StringView name, RuleAction* outAction);
StringView ruleActionName(RuleAction action);

struct OsVersion {
    u32 major;
    u32 minor;
    u64 patch;

    bool atLeast(OsVersion other);
    bool atMost(OsVersion other);

    static Result parse(StringView str, OsVersion* out);
};

struct OsInfo {
    StringView name;
    OsVersion version;
    Architecture arch;

    static OsInfo current();
};


struct FeatureSet {
    bool isDemoUser;
    bool hasCustomResolution;
    bool hasQuickPlaysSupport;
    bool isQuickPlaySingleplayer;
    bool isQuickPlayMultiplayer;
    bool isQuickPlayRealms;
};

struct Rule {

    RuleAction action;

    Optional<StringView> osName;
    Optional<OsVersion> maxVersion;
    Optional<OsVersion> minVersion;
    Optional<StringView> versionRegex;
    Optional<Architecture> architecture;
    Optional<bool> isDemoUser;
    Optional<bool> hasCustomResolution;
    Optional<bool> hasQuickPlaysSupport;
    Optional<bool> isQuickPlaySingleplayer;
    Optional<bool> isQuickPlayMultiplayer;
    Optional<bool> isQuickPlayRealms;

    static Result parse(JsonObject obj, Rule* out);
    bool matches(OsInfo os, FeatureSet features);

    inline bool canUse(OsInfo os, FeatureSet features) {
        return matches(os, features) == (action == RuleAction::ALLOW);
    }

};


Result appendArgs(const VersionRef& ref, StringView args, StringBuilder* command, Span<Placeholder> placeholders, FeatureSet features);


}
