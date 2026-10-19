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
 * \file Utility/Hash.cpp
 * \author jwall64
 * \brief Implementation of Utility/Hash.hpp
 */

#include <Utility/Hash.hpp>
#include <Platform/File.hpp>

#include <openssl/evp.h>


namespace {

nbl::Result _hash(const EVP_MD* md, nbl::Reader reader, nbl::Span<u8> outBytes) {

    if(outBytes.length < EVP_MD_size(md)) return nbl::Result::INVALID_ARGUMENT;
    
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if(ctx == nullptr) {
        return nbl::Result::LIBRARY_ERROR;
    }

    if(EVP_DigestInit_ex(ctx, md, nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        return nbl::Result::LIBRARY_ERROR;
    }

    char dataBuffer[4096] = {};
    usize bytesRead = 0;
    nbl::Result res;
    while((res = reader.read(dataBuffer, 4096, &bytesRead).success())) {
        if(EVP_DigestUpdate(ctx, dataBuffer, bytesRead) != 1) {
            EVP_MD_CTX_free(ctx);
            return nbl::Result::LIBRARY_ERROR;
        }
    }

    u32 outputLen = 32;
    if(EVP_DigestFinal_ex(ctx, outBytes.data, &outputLen) != 1) {
        EVP_MD_CTX_free(ctx);
        return nbl::Result::LIBRARY_ERROR;
    }

    return nbl::Result::SUCCESS;
}

nbl::Result _hash(const EVP_MD* md, nbl::ConstSpan<u8> buffer, nbl::Span<u8> outBytes) {

    if(outBytes.length < EVP_MD_size(md)) return nbl::Result::INVALID_ARGUMENT;
    
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if(ctx == nullptr) {
        return nbl::Result::LIBRARY_ERROR;
    }

    if(EVP_DigestInit_ex(ctx, md, nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        return nbl::Result::LIBRARY_ERROR;
    }

    if(EVP_DigestUpdate(ctx, buffer.data, buffer.length) != 1) {
        EVP_MD_CTX_free(ctx);
        return nbl::Result::LIBRARY_ERROR;
    }

    u32 outputLen = 32;
    if(EVP_DigestFinal_ex(ctx, outBytes.data, &outputLen) != 1) {
        EVP_MD_CTX_free(ctx);
        return nbl::Result::LIBRARY_ERROR;
    }

    return nbl::Result::SUCCESS;
}

}


nbl::Result nbl::fileSha1(nbl::StringView filePath, Span<u8> outBytes) {

    if(outBytes.length < 20) return nbl::Result::INVALID_ARGUMENT;

    FileChannel file{};
    NBL_CHECK(FileChannel::open(filePath, &file, FileOpenFlag::READ));

    return sha1(file.reader(), outBytes);
}

nbl::Result nbl::fileSha256(nbl::StringView filePath, Span<u8> outBytes) {

    if(outBytes.length < 32) return nbl::Result::INVALID_ARGUMENT;

    FileChannel file{};
    NBL_CHECK(FileChannel::open(filePath, &file, FileOpenFlag::READ));

    return sha256(file.reader(), outBytes);
}

nbl::Result nbl::sha1(nbl::Reader reader, Span<u8> outBytes) {
    return _hash(EVP_sha1(), reader, outBytes);
}

nbl::Result nbl::sha256(Reader reader, Span<u8> outBytes) {
    return _hash(EVP_sha256(), reader, outBytes);
}

nbl::Result nbl::sha256(ConstSpan<u8> bytes, Span<u8> outBytes) {
    return _hash(EVP_sha256(), bytes, outBytes);
}

nbl::Result nbl::md5(Reader reader, Span<u8> outBytes) {
    return _hash(EVP_md5(), reader, outBytes);
}

nbl::Result nbl::md5(ConstSpan<u8> bytes, Span<u8> outBytes) {
    return _hash(EVP_md5(), bytes, outBytes);
}
