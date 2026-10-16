#include <Platform/Allocator.hpp>
#include <Core/Result.hpp>

#include <cstring>

nbl::Result nbl::LinearAllocator::readAll(Reader reader) noexcept {
    u8 copyBuffer[4096]{};
    Result res;
    usize bytesRead;
    while(!(res = reader.read(copyBuffer, 4096, &bytesRead)).error()) {
        if(bytesRead > 0) {
            void* dest;
            NBL_CHECK(push(bytesRead, &dest));
            ::memcpy(dest, copyBuffer, bytesRead);
        }
        if(res == Result::END_OF_STREAM) {
            res = Result::SUCCESS;
            break;
        }
    }
    return res;
}


nbl::Result nbl::LinearAllocator::write(const void* data, usize length) noexcept {
    if(data == nullptr || length == 0) return nbl::Result::INVALID_ARGUMENT;
    void* allocated = nullptr;
    NBL_CHECK(push(length, &allocated));
    memcpy(allocated, data, length);
    return nbl::Result::SUCCESS;
}


nbl::Result nbl::InPlaceAllocator::push(usize amount, void** outPtr) noexcept {
    if(amount == 0 || outPtr == nullptr) return nbl::Result::INVALID_ARGUMENT;
    
    usize remaining = _mCapacity - _mIndex;
    if(remaining < amount) return nbl::Result::OUT_OF_MEMORY;

    *outPtr = end();
    _mIndex += amount;

    memset(*outPtr, 0, amount);

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::InPlaceAllocator::pop(usize amount) noexcept {

    if(amount == 0 || amount > _mIndex) return nbl::Result::INVALID_ARGUMENT;
    _mIndex -= amount;

    return nbl::Result::SUCCESS;
}
