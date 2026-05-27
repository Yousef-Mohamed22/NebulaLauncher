// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file StringBuilder.hpp
 * \author jwall64
 * \brief A simple implementation of a string builder that allows for efficient construction of strings using a memory arena for temporary storage.
 */

#pragma once

#include <Core/Result.hpp>
#include <Core/String.hpp>
#include <Platform/Logging.hpp>
#include <Platform/Arena.hpp>

namespace nbl {

class JsonValue;

/**
 * \brief An implementation of a string builder that allows for efficient construction of strings using a memory arena for temporary storage.  
 */
class StringBuilder {
public:

    /**
     * \brief Constructor that initializes the string builder with a reference to an arena frame for storage. The string builder will use the arena frame to allocate memory for the string being built, and will keep track of the starting index in the arena to determine the final string view when building is complete.
     * \param arena Pointer to the arena frame that will be used to store the final string.
     */
    StringBuilder(ArenaFrame* arena) :
        _mLastResult(Result::SUCCESS),
        _mFrame(arena)
    { }

    /**
     * \brief Appends a value of any type to the string being built. This function is a template that can accept any type of value, and it will attempt to convert the value to a string representation and append it to the current string being built. If the append operation fails, the string builder will store the error result and all subsequent append operations will be no-ops.
     * \tparam T The type of the value to be appended. This can be any type for which a toString function is defined that can convert the value to a string representation.
     * \param value The value to be appended to the current string being built. This can be of any type for which a toString function is defined.
     * \return A reference to the string builder for chaining. If the append operation fails, the string builder will store the error result and subsequent append operations will be no-ops.
     */
    template<typename T>
    StringBuilder& append(T&& value) {
        appendChecked(std::forward<T>(value));
        return *this;
    }

    StringBuilder& replace(char oldValue, StringView newValue, usize startIndex = 0) {
        replaceChecked(oldValue, newValue);
        return *this;
    }

    Result replaceChecked(char oldValue, StringView newValue, usize startIndex = 0);


    /**
     * \brief Appends a value of any type to the string being built, with error checking. This function is a template that can accept any type of value, and it will attempt to convert the value to a string representation and append it to the current string being built. If the append operation fails, the string builder will store the error result and all subsequent append operations will be no-ops.
     * \tparam T The type of the value to be appended. This can be any type for which a toString function is defined that can convert the value to a string representation.
     * \param value The value to be appended to the current string being built. This can be of any type for which a toString function is defined.
     * \return A Result indicating success or failure of the append operation. If the append operation fails, the string builder will store the error result and subsequent append operations will be no-ops.
     */
    template<typename T>
    Result appendChecked(T&& value) {
        if(!_mLastResult.success()) {
            return _mLastResult;
        }
        _mLastResult = toString(std::forward<T>(value), _mFrame);
        return _mLastResult;
    }

    /**
     * \brief Builds the final string view from the string being built. If the string builder is in an error state, it will return the stored error result.
     * \param outString Pointer to a variable that will receive the StringView representing the constructed string. 
     * \return A Result indicating success or failure of building the final string view. 
     */
    Result build(StringView* outString) {
        if(!_mLastResult.success()) {
            return _mLastResult;
        }

        void* ptr = _mFrame.begin();
        *outString = StringView(_mFrame.index(), static_cast<char*>(ptr));

        _mFrame.release();
        return Result::SUCCESS;
    }

    /**
     * \brief Builds the final null-terminated string view from the string being built. 
     * \param outString Pointer to a variable that will receive the StringView representing the constructed null-terminated string. 
     * \return A Result indicating success or failure of building the final null-terminated string view. 
     */
    Result buildNullTerminated(StringView* outString) {

        if(!_mLastResult.success()) {
            return _mLastResult;
        }
        NBL_CHECK(_mFrame.pushValue('\0'));

        void* ptr = _mFrame.begin();
        *outString = StringView(_mFrame.index() - 1, static_cast<char*>(ptr));

        _mFrame.release();
        return Result::SUCCESS;
    }

    /**
     * \brief Static helper function that creates a null-terminated string view from a given string.
     * \param string The StringView representing the string to be converted into a null-terminated string view. 
     * \return A Result indicating success or failure of creating the null-terminated string view. If any step of the process fails, it returns the error result. 
     */
    static Result nullTerminate(StringView string, StringView* outString, ArenaFrame* arena) {
        return StringBuilder(arena).append(string).append('\0').build(outString);
    }

private:
    Result _mLastResult;
    ArenaFrame _mFrame;

};

}
