#pragma once

#include <Core/String.hpp>
#include <Core/Optional.hpp>

#include <functional>

namespace nbl {

struct MetadataLookupInfo {
    Optional<StringView> title;
    Optional<u64> releaseDate;
    Span<StringView> platformNames;
    Span<StringView> platformIds;
};

enum MetadataField {
    TITLE,
    DESCRIPTION,
    RELEASE_DATE,
    RATING,
    TAG,
    POSTER,
    BANNER,
    ICON,
    LOGO
};

union MetadataValue {
    StringView string;
    StringView title;
    StringView description;
    u64 releaseDate;
    f64 rating;
    StringView tag;
    StringView poster;
    StringView banner;
    StringView icon;
    StringView logo;
};

typedef std::function_ref<Result(MetadataField, MetadataValue)> MetadataCallback;

struct MetadataProvider {

    StringView id;
    StringView name;
    bool(*pfnCanProvide)(const MetadataLookupInfo& info, void* userData);
    Result(*pfnProvide)(const MetadataLookupInfo& info, MetadataCallback callback, void* userData);
    void* userData;

    bool canProvide(const MetadataLookupInfo& info) const {
        if(pfnCanProvide == nullptr) return false;
        return pfnCanProvide(info, userData);
    }

    Result provide(const MetadataLookupInfo& info, MetadataCallback callback) const {
        if(pfnProvide == nullptr) return nbl::Result::PLUGIN_ERROR;
        return pfnProvide(info, callback, userData);
    }

};


}
