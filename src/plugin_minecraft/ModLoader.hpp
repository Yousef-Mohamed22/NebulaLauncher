#pragma once

#include <Core/String.hpp>
#include "DownloadManager.hpp"
#include "VersionManager.hpp"

namespace nbl::mc {


struct ModLoader {
    enum Value : u32 {
        VANILLA,
        FABRIC,
        _END
    };

    ModLoader(Value v) : value(v) {}
    ModLoader(u32 v) : value(v) {}

    operator u32() { return value; }

    u32 value;

    static inline u32 index(nbl::PluginContext* ctx,StringView name, void* userData) {
        if(name == "fabric") return ModLoader::FABRIC;
        return VANILLA;
    }
    static inline StringView name(nbl::PluginContext* ctx,u32 index, void* userData) {
        switch(index) {
            default:
            case VANILLA: return "vanilla";
            case FABRIC: return "fabric";
        }
    }
    static inline StringView displayName(nbl::PluginContext* ctx,u32 index, void* userData) {
        switch(index) {
            default:
            case VANILLA: return "None";
            case FABRIC: return "Fabric";
        }
    }
    static inline u32 count(nbl::PluginContext* ctx,void* userData) {
        return _END;
    }
};


Result generateFabricVersionMeta(VersionRef* outVersion, StringView folder, DownloadManager* manager, LinearAllocator* allocator);


}
