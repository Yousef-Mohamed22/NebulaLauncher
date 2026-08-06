#include "SkinUtil.hpp"
#include "Platform/File.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "vendor/stb_image.h"
#include "vendor/stb_image_write.h"

#include <Platform/Arena.hpp>
#include <Utility/StringBuilder.hpp>

nbl::Result nbl::mc::downloadSkinHead(StringView skinFolder, StringView url, HttpClient* client, StringView* outSkinId) {

    NBL_CHECK(extractSkinId(url, outSkinId));

    nbl::AllocationFrame frame(nbl::Arena::threadLocal());
    StringView skinFile;
    NBL_CHECK(StringBuilder(&frame).append(skinFolder).append('/').append(*outSkinId).append(".png").buildNullTerminated(&skinFile));

    if(nbl::fileExists(skinFile)) { 
        return nbl::Result::SUCCESS;
    }
    NBL_CHECK(nbl::makeDirectories(skinFolder));

    stbi_uc* buf = static_cast<stbi_uc*>(frame.end());
    client->makeRequest(url, frame.writer());

    u32 numFailures;
    client->awaitAll(&numFailures);
    if(numFailures > 0) {
        return nbl::Result::IO_ERROR;
    }

    u8 outputData[8 * 8 * 4]{};

    int width, height, channels;
    u8* data = stbi_load_from_memory(buf, frame.index(), &width, &height, &channels, 4);

    if(channels != 4 || width != 64 || height < 32) {
        stbi_image_free(data);
        return nbl::Result::FORMAT_ERROR;
    }

    int rowLength = width * channels;


    // Copy head
    for(auto y = 0 ; y < 8 ; y++) {
        for(auto x = 0 ; x < 8 ; x++) {
            auto outIndex = (x * 4) + (y * 32);
            auto inIndex = (rowLength * (y + 8)) + 32 + (x * 4);

            unsigned char a = data[inIndex + 3];
            if(a == 0) {

                outputData[outIndex] = 0;
                outputData[outIndex + 1] = 0;
                outputData[outIndex + 2] = 0;
                outputData[outIndex + 3] = 255;

            } else if(a == 255) {

                outputData[outIndex] = data[inIndex];
                outputData[outIndex + 1] = data[inIndex + 1];
                outputData[outIndex + 2] = data[inIndex + 2];
                outputData[outIndex + 3] = 255;

            } else {

                float red = data[inIndex] / 255.0f;
                float green = data[inIndex + 1] / 255.0f;
                float blue = data[inIndex + 2] / 255.0f;

                float alpha = a / 255.0f;

                outputData[outIndex]     = 255 * (red * alpha);
                outputData[outIndex + 1] = 255 * (green * alpha);
                outputData[outIndex + 2] = 255 * (blue * alpha);
                outputData[outIndex + 3] = 255;
            }

        }
    }

    // Copy hat 
    for(auto y = 0 ; y < 8 ; y++) {
        for(auto x = 0 ; x < 8 ; x++) {

            auto outIndex = (x * 4) + (y * 32);
            auto inIndex = (rowLength * (y + 8)) + 160 + (x * 4);

            unsigned char a = data[inIndex + 3];
            if(a == 0) {
                continue;

            } else if(a == 255) {

                outputData[outIndex] = data[inIndex];
                outputData[outIndex + 1] = data[inIndex + 1];
                outputData[outIndex + 2] = data[inIndex + 2];
                outputData[outIndex + 3] = 255;

            } else {

                float baseRed = outputData[outIndex] / 255.0f;
                float baseGreen = outputData[outIndex + 1] / 255.0f;
                float baseBlue = outputData[outIndex + 2] / 255.0f;

                float red = data[inIndex] / 255.0f;
                float green = data[inIndex + 1] / 255.0f;
                float blue = data[inIndex + 2] / 255.0f;

                float alpha = a / 255.0f;

                outputData[outIndex]     = 255 * ((red * alpha)   + (baseRed   * (1.0f - alpha)));
                outputData[outIndex + 1] = 255 * ((green * alpha) + (baseGreen * (1.0f - alpha)));
                outputData[outIndex + 2] = 255 * ((blue * alpha)  + (baseBlue  * (1.0f - alpha)));
                outputData[outIndex + 3] = 255;
            }
        }
    }

    stbi_write_png(skinFile.data, 8, 8, 4, outputData, 32);
    stbi_image_free(data);

    return nbl::Result::SUCCESS;
}
nbl::Result nbl::mc::extractSkinId(StringView url, StringView *skinId) {

    if(!url.startsWith("http://textures.minecraft.net/texture/")) {
        return nbl::Result::INVALID_ARGUMENT;
    }
    *skinId = url.tail(38);
    if(skinId->length == 0 || skinId->length > 64) {
        return nbl::Result::INVALID_ARGUMENT;
    }
    return nbl::Result::SUCCESS;
}
