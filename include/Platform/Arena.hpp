#pragma once


#include <Core/Types.hpp>
#include <Core/Result.hpp>
#include <Core/String.hpp>
#include <Platform/Assert.hpp>
#include <Platform/Allocator.hpp>

namespace nbl {


class Arena : public LinearAllocator {
public:

    Arena() = default;

    void* begin() const noexcept override {
        return _pHead;
    }

    void* end() const noexcept override {
        return static_cast<u8*>(_pHead) + _mIndex;
    }

    Result push(usize amount, void** outPtr) noexcept override;

    Result pop(usize amount) noexcept override;

    inline usize index() const noexcept override {
        return _mIndex;
    }

    inline usize capacity() const noexcept {
        return _mCapacity;
    }

    static Result create(usize amount, Arena* outArena) noexcept;

    static Arena* threadLocal() noexcept;

private:

    void* _pHead;
    usize _mCapacity;
    usize _mCommitted;
    usize _mIndex;

};


class ArenaFrame : public LinearAllocator {
public:

    ArenaFrame(Arena* arena) :
        _pArena(arena),
        _mIndex(arena->index()),
        _mStart(_mIndex)
    { }

    ~ArenaFrame() {
        if(_mIndex > _mStart) {
            _pArena->popTo(_mStart);
        }
    }

    void* begin() const noexcept override {
        return static_cast<u8*>(_pArena->begin()) + _mStart;
    }

    void* end() const noexcept override {
        return static_cast<u8*>(_pArena->end());
    }

    Result push(usize amount, void** outPtr) noexcept override {
        NBL_ASSERT(_mIndex == _pArena->index());
        NBL_CHECK(_pArena->push(amount, outPtr));
        _mIndex += amount;
        return nbl::Result::SUCCESS;
    }

    Result pop(usize amount) noexcept override {
        NBL_ASSERT(_mIndex == _pArena->index());
        NBL_CHECK(_pArena->pop(amount));
        _mIndex -= amount;
        return nbl::Result::SUCCESS;
    }

    inline usize index() const noexcept override {
        return _mIndex - _mStart;
    }

    void release() noexcept {
        NBL_ASSERT(_mIndex == _pArena->index());
        _mStart = _mIndex;
    }

private:

    Arena* _pArena;
    usize _mIndex;
    usize _mStart;

};


}
