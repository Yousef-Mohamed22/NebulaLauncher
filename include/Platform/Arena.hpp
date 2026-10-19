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
 * \file Platform/Allocator.hpp
 * \author jwall64
 * \brief Arena allocator interface
 */

#pragma once


#include <Core/Types.hpp>
#include <Core/Result.hpp>
#include <Core/String.hpp>
#include <Core/Assert.hpp>
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

    static  bool isThreadLocalWriter(Writer writer) noexcept;

private:

    void* _pHead;
    usize _mCapacity;
    usize _mCommitted;
    usize _mIndex;

};


}
