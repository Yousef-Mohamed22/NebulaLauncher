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
 * \file Utility/StringBuilder.hpp
 * \author jwall64
 * \brief Utility for dynamically creating strings using a linear allocator
 */


#pragma once

#include <Core/Result.hpp>
#include <Core/String.hpp>
#include <Core/Stream.hpp>
#include <Platform/Allocator.hpp>

#include <utility>

namespace nbl {

/**
 * \brief An implementation of a string builder that allows for efficient construction of strings using a memory arena for temporary storage.  
 */
class StringBuilder {
public:

    /**
     * \brief Constructor that initializes the string builder with a reference to an arena frame for storage. The string builder will use the arena frame to allocate memory for the string being built, and will keep track of the starting index in the arena to determine the final string view when building is complete.
     * \param arena Pointer to the arena frame that will be used to store the final string.
     */
    StringBuilder(LinearAllocator* allocator) noexcept :
        _mLastResult(Result::SUCCESS),
        _pAllocator(allocator)
    { }

    /**
     * \brief Appends a value of any type to the string being built. This function is a template that can accept any type of value, and it will attempt to convert the value to a string representation and append it to the current string being built. If the append operation fails, the string builder will store the error result and all subsequent append operations will be no-ops.
     * \tparam T The type of the value to be appended. This can be any type for which a toString function is defined that can convert the value to a string representation.
     * \param value The value to be appended to the current string being built. This can be of any type for which a toString function is defined.
     * \return A reference to the string builder for chaining. If the append operation fails, the string builder will store the error result and subsequent append operations will be no-ops.
     */
    template<typename T>
    StringBuilder& append(T&& value) noexcept {
        appendChecked(std::forward<T>(value));
        return *this;
    }

    StringBuilder& replace(char oldValue, StringView newValue, usize startIndex = 0) noexcept {
        replaceChecked(oldValue, newValue);
        return *this;
    }

    Result replaceChecked(char oldValue, StringView newValue, usize startIndex = 0) noexcept;


    /**
     * \brief Appends a value of any type to the string being built, with error checking. This function is a template that can accept any type of value, and it will attempt to convert the value to a string representation and append it to the current string being built. If the append operation fails, the string builder will store the error result and all subsequent append operations will be no-ops.
     * \tparam T The type of the value to be appended. This can be any type for which a toString function is defined that can convert the value to a string representation.
     * \param value The value to be appended to the current string being built. This can be of any type for which a toString function is defined.
     * \return A Result indicating success or failure of the append operation. If the append operation fails, the string builder will store the error result and subsequent append operations will be no-ops.
     */
    template<typename T>
    Result appendChecked(T&& value) noexcept {
        if(!_mLastResult.success()) {
            return _mLastResult;
        }
        if(_mLength == 0) {
            _mBegin = _pAllocator->index();
        }
        _mLastResult = write(std::forward<T>(value), _pAllocator->writer());
        _mLength = _pAllocator->index() - _mBegin;
        return _mLastResult;
    }

    /**
     * \brief Builds the final string view from the string being built. If the string builder is in an error state, it will return the stored error result.
     * \param outString Pointer to a variable that will receive the StringView representing the constructed string. 
     * \return A Result indicating success or failure of building the final string view. 
     */
    Result build(StringView* outString) noexcept {
        if(!_mLastResult.success()) {
            return _mLastResult;
        }

        char* ptr = static_cast<char*>(_pAllocator->begin()) + _mBegin;
        *outString = StringView(ptr, _mLength);

        return Result::SUCCESS;
    }

    /**
     * \brief Builds the final null-terminated string view from the string being built. 
     * \param outString Pointer to a variable that will receive the StringView representing the constructed null-terminated string. 
     * \return A Result indicating success or failure of building the final null-terminated string view. 
     */
    Result buildNullTerminated(StringView* outString) noexcept {

        if(!_mLastResult.success()) {
            return _mLastResult;
        }
        NBL_CHECK(_pAllocator->write("\0", 1));

        char* ptr = static_cast<char*>(_pAllocator->begin()) + _mBegin;
        *outString = StringView(ptr, _mLength);

        return Result::SUCCESS;
    }

    Result retain(usize index, usize length) noexcept {

        if(index + length > _mLength) return nbl::Result::INVALID_ARGUMENT;

        char* ptr = static_cast<char*>(_pAllocator->begin()) + _mBegin;
        if(length > 0) {
            memmove(ptr, ptr + index, length);
        }

        NBL_CHECK(_pAllocator->popToPtr(ptr + length));
        _mLength = length;

        return nbl::Result::SUCCESS;
    }


    Result retainLast(usize length) noexcept {
        if(length > _mLength) return nbl::Result::INVALID_ARGUMENT;
        return retain(_mLength - length, length);
    }


    operator Writer() noexcept {
        return writer();
    }

    Writer writer() noexcept {
        return Writer {
            .impl = this,
            .pfnWrite = _write
        };       
    }

    inline const LinearAllocator* allocator() const noexcept {
        return _pAllocator;
    }

    inline usize length() const noexcept { return _mLength; }

    static inline bool isStringBuilderWriter(Writer writer) noexcept {
        return writer.pfnWrite == _write;
    }

    /**
     * \brief Static helper function that creates a null-terminated string view from a given string.
     * \param string The StringView representing the string to be converted into a null-terminated string view. 
     * \return A Result indicating success or failure of creating the null-terminated string view. If any step of the process fails, it returns the error result. 
     */
    static inline Result nullTerminate(StringView string, StringView* outString, AllocationFrame* arena) noexcept {
        return StringBuilder(arena).append(string).append('\0').build(outString);
    }

private:
    Result _mLastResult;
    usize _mLength;
    usize _mBegin;
    LinearAllocator* _pAllocator;

    inline static Result _write(void* impl, const void* data, usize length, usize* bytesWritten) noexcept {
        StringBuilder* builder = static_cast<StringBuilder*>(impl);
        NBL_CHECK(builder->appendChecked(StringView(static_cast<const char*>(data), length)));
        if(bytesWritten) *bytesWritten = length;
        return Result::SUCCESS;
    }

};

}
