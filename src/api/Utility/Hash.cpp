#include <Utility/Hash.hpp>
#include <Platform/File.hpp>

#include <openssl/evp.h>

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

    if(outBytes.length < 20) return nbl::Result::INVALID_ARGUMENT;
    
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if(ctx == nullptr) {
        return nbl::Result::LIBRARY_ERROR;
    }

    if(EVP_DigestInit_ex(ctx, EVP_sha1(), nullptr) != 1) {
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

    u32 outputLen = 20;
    if(EVP_DigestFinal_ex(ctx, outBytes.data, &outputLen) != 1) {
        EVP_MD_CTX_free(ctx);
        return nbl::Result::LIBRARY_ERROR;
    }

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::sha256(Reader reader, Span<u8> outBytes) {

    if(outBytes.length < 32) return nbl::Result::INVALID_ARGUMENT;
    
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if(ctx == nullptr) {
        return nbl::Result::LIBRARY_ERROR;
    }

    if(EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
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
