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
