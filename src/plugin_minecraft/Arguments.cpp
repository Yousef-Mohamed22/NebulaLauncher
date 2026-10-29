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

#include "Arguments.hpp"

#include <Codec/Json.hpp>
#include <Core/Result.hpp>
#include <Platform/PlatformDefines.hpp>
#include <Platform/Logging.hpp>
#include <Utility/Placeholder.hpp>
#include <Utility/StringBuilder.hpp>

#include <regex>
#include <utility>
#include "VersionManager.hpp"

#ifdef NBL_PLATFORM_LINUX

#include <sys/utsname.h>

#endif

nbl::Result nbl::mc::parseArchitecture(StringView name, Architecture *outAction) {
    if(name == "x86") { *outAction = Architecture::X86; return nbl::Result::SUCCESS; }
    if(name == "amd64") { *outAction = Architecture::AMD64; return nbl::Result::SUCCESS; }
    if(name == "aarch64") { *outAction = Architecture::AARCH64; return nbl::Result::SUCCESS; }
    return nbl::Result::NOT_FOUND;
}

nbl::StringView nbl::mc::architectureName(Architecture action) {
    switch(action) {
        case Architecture::X86: return "x86";
        case Architecture::AMD64: return "amd64";
        case Architecture::AARCH64: return "aarch64";
    }
    std::unreachable();
}




nbl::Result nbl::mc::parseRuleAction(StringView name, RuleAction *outAction) {
    if(name == "allow") { *outAction = RuleAction::ALLOW; return nbl::Result::SUCCESS; }
    if(name == "disallow") { *outAction = RuleAction::DISALLOW; return nbl::Result::SUCCESS; }
    return nbl::Result::NOT_FOUND;
}

nbl::StringView nbl::mc::ruleActionName(RuleAction action) {
    switch(action) {
        case RuleAction::ALLOW: return "allow";
        case RuleAction::DISALLOW: return "disallow";
    }
    std::unreachable();
}


bool nbl::mc::OsVersion::atLeast(OsVersion other) {
    return major > other.major ||
        (major == other.major && minor > other.minor) ||
        (major == other.major && minor == other.minor && patch >= other.patch);
}

bool nbl::mc::OsVersion::atMost(OsVersion other) {
    return major < other.major ||
        (major == other.major && minor < other.minor) ||
        (major == other.major && minor == other.minor && patch <= other.patch);
}

nbl::Result nbl::mc::OsVersion::parse(StringView version, OsVersion* out) {
    usize dotIndex = 0;
    nbl::Result res = version.findFirst('.', &dotIndex);

    if(res.error()) {
        *out = { .major = 0, .minor = 0, .patch = 0 };
        return nbl::Result::FORMAT_ERROR;
    }

    JsonNumber numberJson{};
    res = nbl::parseJsonNumber(version.slice(0, dotIndex), &numberJson);
    if(res.error()) {
        *out = { .major = 0, .minor = 0, .patch = 0 };
        return nbl::Result::FORMAT_ERROR;
    }

    out->major = numberJson.asUint();

    StringView lastTwo = version.slice(dotIndex + 1);
    res = lastTwo.findFirst('.', &dotIndex);

    if(res.error()) {
        out->minor = 0;
        out->patch = 0;
        return nbl::Result::SUCCESS;
    }

    res = nbl::parseJsonNumber(lastTwo.slice(0, dotIndex), &numberJson);
    if(res.error()) {
        out->minor = 0;
        out->patch = 0;
        return nbl::Result::SUCCESS;
    }


    out->minor = numberJson.asUint();

    StringView lastOne = lastTwo.slice(dotIndex + 1);
    res = lastOne.findFirst('-', &dotIndex);

    if(res.error()) {
        res = lastOne.findFirst('+', &dotIndex);
        if(res.error()) {
            dotIndex = lastOne.length();
        }
    }

    res = nbl::parseJsonNumber(lastOne.slice(0, dotIndex), &numberJson);
    if(res.error()) {
        out->patch = 0;
        return nbl::Result::SUCCESS;
    }

    out->patch = numberJson.asUint();
    return nbl::Result::SUCCESS;
}

nbl::mc::OsInfo nbl::mc::OsInfo::current() {

    static OsInfo _cached;
    if(_cached.name.empty()) {

#ifdef NBL_PLATFORM_LINUX

        _cached.name = "linux";

        static struct utsname _buffer;
        if(uname(&_buffer) != 0) {
            nbl::log(nbl::LogLevel::WARN, "Unable to call uname() to query OS version!");
            _cached.version = { .major = 0, .minor = 0, .patch = 0 };
            return _cached;
        }

        OsVersion::parse(_buffer.release, &_cached.version);
#endif


#if defined(NBL_ARCH_AMD64)
        _cached.arch = Architecture::AMD64;
#elif defined(NBL_ARCH_X86)
        _cached.arch = Architecture::X86;
#elif defined(NBL_ARCH_AARCH64)
        _cached.arch = Architecture::AARCH64;
#endif

    }

    return _cached;
}


nbl::Result nbl::mc::Rule::parse(JsonObject obj, Rule* out) {

    RuleAction action;
    NBL_CHECK(parseRuleAction(obj.getString("action"), &action));

    JsonObject osObj = obj.getObject("os");
    if(osObj.has("name")) {
        out->osName = osObj.getString("name");
    }
    if(osObj.has("versionRange")) {
        JsonObject versionRangeObj = osObj.getObject("versionRange");
        if(versionRangeObj.has("min")) {
            OsVersion ver;
            NBL_CHECK(OsVersion::parse(versionRangeObj.getString("min"), &ver));
            out->minVersion = ver;
        }
        if(versionRangeObj.has("max")) {
            OsVersion ver;
            NBL_CHECK(OsVersion::parse(versionRangeObj.getString("max"), &ver));
            out->maxVersion = ver;
        }
    }
    if(osObj.has("version")) {
        out->versionRegex = osObj.getString("version");
    }
    if(osObj.has("arch")) {
        Architecture arch;
        NBL_CHECK(parseArchitecture(osObj.getString("arch"), &arch));
        out->architecture = arch;
    }


    JsonObject featuresObj = obj.getObject("features");
    if(featuresObj.has("is_demo_user")) {
        out->isDemoUser = featuresObj.getBoolean("is_demo_user");
    }
    if(featuresObj.has("has_custom_resolution")) {
        out->hasCustomResolution = featuresObj.getBoolean("has_custom_resolution");
    }
    if(featuresObj.has("has_quick_plays_support")) {
        out->hasQuickPlaysSupport = featuresObj.getBoolean("has_quick_plays_support");
    }
    if(featuresObj.has("is_quick_play_singleplayer")) {
        out->isQuickPlaySingleplayer = featuresObj.getBoolean("is_quick_play_singleplayer");
    }
    if(featuresObj.has("is_quick_play_multiplayer")) {
        out->isQuickPlayMultiplayer = featuresObj.getBoolean("is_quick_play_multiplayer");
    }
    if(featuresObj.has("is_quick_play_realms")) {
        out->isQuickPlayRealms = featuresObj.getBoolean("is_quick_play_realms");
    }

    return nbl::Result::SUCCESS;
}

bool nbl::mc::Rule::matches(OsInfo os, FeatureSet features) {

    if(osName.hasValue() && *osName != os.name) return false;
    if(minVersion.hasValue() && !os.version.atLeast(*minVersion)) return false;
    if(maxVersion.hasValue() && !os.version.atMost(*maxVersion)) return false;
    if(versionRegex.hasValue()) {

        AllocationFrame frame(Arena::threadLocal());
        StringView versionString;
        NBL_CHECK(StringBuilder(&frame).append(os.version.major).append('.').append(os.version.minor).append('.').append(os.version.patch).buildNullTerminated(&versionString));

        std::string regexStr(versionRegex->data(), versionRegex->length());
        std::regex re(regexStr);

        if(!std::regex_search(versionString.data(), re)) return false;
    }
    if(architecture.hasValue() && *architecture != os.arch) return false; 

    if(isDemoUser.hasValue() && features.isDemoUser != *isDemoUser) return false;
    if(hasCustomResolution.hasValue() && features.hasCustomResolution != *hasCustomResolution) return false;
    if(hasQuickPlaysSupport.hasValue() && features.hasQuickPlaysSupport != *hasQuickPlaysSupport) return false;
    if(isQuickPlaySingleplayer.hasValue() && features.isQuickPlaySingleplayer != *isQuickPlaySingleplayer) return false;
    if(isQuickPlayMultiplayer.hasValue() && features.isQuickPlaySingleplayer != *isQuickPlayMultiplayer) return false;
    if(isQuickPlayRealms.hasValue() && features.isQuickPlayRealms != *isQuickPlayRealms) return false;

    return true;
}


nbl::Result nbl::mc::appendArgs(const VersionRef& ref, StringView args, StringBuilder* command, Span<Placeholder> placeholders, FeatureSet features) {

    return ref.enumerateArguments(args, [&](JsonElement ele) -> nbl::Result {        
        if(ele.type() == JsonType::STRING) {
            command->append(" \"");
            NBL_CHECK(formatPlaceholders(ele.asString(), command, placeholders));
            command->append("\"");
        } else if(ele.type() == JsonType::OBJECT) {

            if(ele.asObject().has("rules")) {
                bool canUse = true;
                JsonArray arr = ele.asObject().getArray("rules");
                for(JsonElement& rule : arr) {
                    if(rule.type() != JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;
                    Rule r;
                    NBL_CHECK(Rule::parse(rule.asObject(), &r));

                    if(!r.canUse(OsInfo::current(), features)) {
                        canUse = false;
                        break;
                    }
                }
                if(!canUse) return nbl::Result::SUCCESS;
            }

            const JsonElement* value = ele.asObject().get("value");
            if(value == nullptr) return nbl::Result::FORMAT_ERROR;

            if(value->type() == JsonType::STRING) {
                command->append(" \"");
                NBL_CHECK(formatPlaceholders(value->asString(), command, placeholders));
                command->append("\"");
            } else if(value->type() == JsonType::ARRAY) {
                for(const JsonElement& line : value->asArray()) {
                    if(line.type() != JsonType::STRING) return nbl::Result::FORMAT_ERROR;
                    command->append(" \"");
                    NBL_CHECK(formatPlaceholders(line.asString(), command, placeholders));
                    command->append("\"");
                }
            } else {
                return nbl::Result::FORMAT_ERROR;
            }
        } 

        return nbl::Result::SUCCESS;
    });

}
