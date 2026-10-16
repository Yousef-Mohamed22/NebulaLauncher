#pragma once

#include <Data/Database.hpp>

namespace nbl {


struct ImageClass {
    u8 index;

    ImageClass(u8 value) : index(value) {}
    operator u8() { return index; }

    enum Value : u8 {
        UNKNOWN,
        BANNER,
        POSTER,
        ICON,
        LOGO,
        INLINE_ICON,
        ACHIEVEMENT_ICON
    };

    struct Dimensions {
        u32 width;
        u32 height;
    };

    Dimensions dimensions() {
        switch (index) {
            default:
            case UNKNOWN:
                return { 0, 0 };
            case BANNER:
                return { 600, 900 };
            case POSTER:
                return { 3840, 1240 };
            case ICON:
            case LOGO:
                return { 1024, 1024 };
            case INLINE_ICON:
            case ACHIEVEMENT_ICON:
                return { 256, 256 };
        }
    }

    bool shouldCrop() {
        return index < LOGO;
    }
};

struct ImageLoadResult {
    ImageClass imageClass;
    bool isPresent;
    union {
        struct {
            StringView imageUrl;
            i64 deviceId;
        } referenceData;
        struct {
            Span<u8> pixels;
            u32 width;
            u32 height;
        } imageData;
    };
};


typedef std::function_ref<Result(const ImageLoadResult&)> ImageLoadCallback;
typedef std::function_ref<Result(usize, u8**)> ImageAllocationCallback;


class MediaManager {
public:

    Result reserveReference(StringView url, ImageClass imageClass, i64* outRefId);
    Result reserveAndSave(StringView url, ImageClass imageClass, u32 width, u32 height, ConstSpan<u8> pixels, i64* outRefId);

    Result saveImage(i64 refId, u32 width, u32 height, ConstSpan<u8> pixels);
    Result loadImage(i64 imageId, ImageLoadCallback res, ImageAllocationCallback alloc);

    Result pruneUnreferencedImages();

    i64 deviceId() const { return _mDeviceId; }

    static Result create(StringView dbPath, i64 deviceId, MediaManager* outManager);

private:

    DatabaseConnection _mConnection;
    i64 _mDeviceId;
};


}

