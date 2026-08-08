#pragma once

#include <Core/Types.hpp>
#include <Core/Result.hpp>
#include <Core/Stream.hpp>

namespace nbl {

class LinearAllocator {
public:
    
    inline virtual ~LinearAllocator() {};

    virtual void* begin() const noexcept = 0;
    virtual void* end() const noexcept = 0;

    virtual Result push(usize amount, void** outPtr) noexcept = 0;

    Result write(const void* data, usize length) noexcept;
    
    template<typename T>
    inline Result allocate(T** location, usize amount = 1) noexcept {
        return push(amount * sizeof(T), reinterpret_cast<void**>(location));
    }

    virtual Result pop(usize amount) noexcept = 0;
    
    inline Result popTo(usize index) {
        return pop(this->index() - index);
    }

    virtual usize index() const noexcept = 0;
    
    inline Writer writer() noexcept {
        return Writer {
            .impl = this,
            .pfnWrite = _write
        };
    }

private:

    static inline Result _write(void* impl, const void* data, usize dataLength, usize* written) noexcept {
        LinearAllocator* self = static_cast<LinearAllocator*>(impl);
        NBL_CHECK(self->write(data, dataLength));
        if(written) *written = dataLength;
        return nbl::Result::SUCCESS;
    }

};

class InPlaceAllocator : public LinearAllocator {
public:

    InPlaceAllocator(void* buffer, usize capacity) : 
        _pData(buffer),
        _mCapacity(capacity)
    { }

    inline void* begin() const noexcept override { return _pData; }
    inline void* end() const noexcept override { return static_cast<u8*>(_pData) + _mIndex; }

    Result push(usize amount, void** outPtr) noexcept override;
    Result pop(usize amount) noexcept override;

    inline usize index() const noexcept override {
        return _mIndex;
    }

private:
    void* _pData;
    usize _mCapacity;
    usize _mIndex;
};

}
