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
 * \file Codec/Qoi.hpp
 * \author jwall64
 * \brief Interface for reading and writing images in QOI format
 */

#pragma once

#include <Core/Types.hpp>
#include <Core/Stream.hpp>
#include <Core/Span.hpp>
#include <Platform/Allocator.hpp>

namespace nbl {

enum class ColorSpace {
    SRGB = 0,
    LINEAR = 1
};


union RGB {
    u32 value : 24;
    struct {
        u8 r;
        u8 g;
        u8 b;
    };
};

union RGBA {
    u32 value;
    struct {
        u8 r;
        u8 g;
        u8 b;
        u8 a;
    };

    bool operator==(const RGBA& other) const {
        return value == other.value;
    }
};

struct QoiImageMeta {

    u32 width;
    u32 height;
    u8 channels;
    ColorSpace colorSpace;

    static Result readMeta(Reader reader, QoiImageMeta* out);
    static Result readImageData(Reader reader, QoiImageMeta meta, LinearAllocator* storage, u8 channels = 0);

    static inline Result read(Reader reader, QoiImageMeta* outMeta, LinearAllocator* storage, u8 channels = 0) {
        NBL_CHECK(readMeta(reader, outMeta));
        return readImageData(reader, *outMeta, storage, channels);
    }

    static Result decodeMeta(ConstSpan<u8> buffer, QoiImageMeta* out);
    static Result decodeImageData(ConstSpan<u8> buffer, QoiImageMeta meta, Span<u8> outputBuffer, u8 channels = 0);

    static inline Result decode(ConstSpan<u8> buffer, QoiImageMeta* outMeta, Span<u8> outputBuffer, u8 channels = 0) {
        NBL_CHECK(decodeMeta(buffer, outMeta));
        return decodeImageData(buffer.slice(14), *outMeta, outputBuffer, channels);
    }


    static Result writeMeta(Writer writer, QoiImageMeta meta);
    static Result writeImageData(Writer writer, QoiImageMeta meta, ConstSpan<u8> imageData);

    static inline Result write(Writer writer, QoiImageMeta meta, ConstSpan<u8> imageData) {
        NBL_CHECK(writeMeta(writer, meta));
        return writeImageData(writer, meta, imageData);
    }

    static Result encodeMeta(Span<u8> buffer, QoiImageMeta meta);
    static Result encodeImageData(LinearAllocator* alloc, QoiImageMeta meta, ConstSpan<u8> imageData);

    static inline Result encode(LinearAllocator* alloc, QoiImageMeta meta, ConstSpan<u8> imageData) {
        Span<u8> header(nullptr, 14);
        NBL_CHECK(alloc->allocate(&header));
        NBL_CHECK(encodeMeta(header, meta));
        return encodeImageData(alloc, meta, imageData);
    }



};

}
