// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Arena.hpp
 * \author jwall64
 * \brief A simple implementation of a memory arena allocator and an arena frame for managing temporary allocations.
 */

#pragma once

#include <Core/Result.hpp>
#include <Core/Types.hpp>
#include <Core/String.hpp>
#include <Core/Stream.hpp>
#include <Core/Math.hpp>

#include <Platform/Logging.hpp>
#include <Platform/Assert.hpp>

#include <stdlib.h>

#include <new>

namespace nbl {

/**
 * \brief A simple implementation of a memory arena allocator that allows for efficient allocation of memory in a contiguous block.
 */
class Arena {
public:

    /**
     * \brief Default constructor that initializes the arena with no allocated memory.
     */
    Arena() :
        _pData(nullptr),
        _mCapacity(0),
        _mCommitted(0),
        _mIndex(0)
    { }

    /**
     * \brief Destructor that frees the allocated memory for the arena.
     */
    ~Arena();

    /**
     * \brief Returns the current index in the arena, which represents the total amount of memory allocated so far.
     * \return The current index in the arena.
     */
    usize index() const {
        return _mIndex;
    }

    /**
     * \brief Returns the total capacity of the arena, which represents the total amount of memory that can be allocated before running out of space.
     * \return The total capacity of the arena.
     */
    usize capacity() const {
        return _mCapacity;
    }

    /**
     * \brief Retrieves a pointer to a specific offset in the arena, allowing for direct access to previously allocated memory.
     * \param offset The offset from the start of the arena to retrieve the pointer for.
     * \param outPtr Pointer to a variable that will receive the pointer to the specified offset in the arena.
     * \return A Result indicating success or failure of the operation. If the offset is out of bounds, it returns nbl::error::OUT_OF_BOUNDS.
     */
    inline Result get(usize offset, void** outPtr) const {
        if(offset >= _mIndex) {
            return nbl::error::OUT_OF_BOUNDS;
        }
        *outPtr = static_cast<char*>(_pData) + offset;
        return Result::SUCCESS;
    }

    /**
     * \brief Returns a pointer to the current position in the arena where the next allocation will occur. This allows for direct access to the memory that will be allocated next.
     * \return A pointer to the current position in the arena.
     */
    inline void* current() const {
        return static_cast<char*>(_pData) + _mIndex;
    }

    /**
     * \brief Returns a pointer to the beginning of the arena, which represents the start of the allocated memory block.
     * \return A pointer to the beginning of the arena.
     */
    inline void* begin() const {
        return _pData;
    }

    /**
     * \brief Allocates a block of memory of the specified size in the arena and returns a pointer to it. The allocated memory is contiguous with any previously allocated memory in the arena.
     * \param size The size of the block of memory to allocate, in bytes.
     * \param outPtr Pointer to a variable that will receive the pointer to the allocated block of memory in the arena.
     * \return A Result indicating success or failure of the allocation. If there is not enough space in the arena to allocate the requested block, it returns nbl::error::OUT_OF_MEMORY.
     */
    Result push(usize size, void** outPtr);

    /** 
     * \brief Allocates a block of memory of the specified size in the arena and copy-constructs an object of type T in that memory using placement new. The allocated memory is contiguous with any previously allocated memory in the arena.
     * \param value The value to construct in the allocated memory.
     * \return A Result indicating success or failure of the allocation and construction. If there is not enough space in the arena to allocate the requested block, it returns nbl::error::OUT_OF_MEMORY.
     */
    template<typename T>
    inline Result pushValue(const T& value) {
        T* ptr;
        NBL_CHECK(push(sizeof(T), reinterpret_cast<void**>(&ptr)));
        new (ptr) T(value);
        return Result::SUCCESS;
    }

    /** 
     * \brief Allocates a block of memory big enough to hold count objects of type T in the arena. The allocated memory is contiguous with any previously allocated memory in the arena.
     * \param outPtr Pointer to a variable that will receive the pointer to the allocated block of memory in the arena, which can hold count objects of type T.
     * \param count The number of objects of type T that the allocated block of memory should be able to hold. Defaults to 1 if not specified.
     * \return A Result indicating success or failure of the allocation and construction. If there is not enough space in the arena to allocate the requested block, it returns nbl::error::OUT_OF_MEMORY.
     */
    template<typename T>
    inline Result allocate(T** outPtr, usize count = 1) {
        return push(sizeof(T) * count, reinterpret_cast<void**>(outPtr));
    }

    /** 
     * \brief Allocates a block of memory in the arena and copies the contents of the specified string into that memory. The allocated memory is contiguous with any previously allocated memory in the arena.
     * \param str The StringView representing the string to be copied into the allocated memory in the arena.
     * \return A Result indicating success or failure of the allocation and copy operation. If there is not enough space in the arena to allocate the requested block, it returns nbl::error::OUT_OF_MEMORY.
     */
    inline Result pushString(StringView str) {
        void* ptr;
        NBL_CHECK(push(sizeof(char) * str.length, &ptr));
        memcpy(ptr, str.data, sizeof(char) * str.length);
        return Result::SUCCESS;
    }

    /** 
     * \brief Pops a specified number of bytes from the arena, effectively deallocating that memory and making it available for future allocations.
     * \param count The number of bytes to pop from the arena.
     * \return The new index in the arena after popping the specified number of bytes. If the count exceeds the current index, it returns 0 and does not modify the arena.
     */
    usize pop(usize count);

    /** 
     * \brief Implicit conversion operator that allows an Arena to be used as a Writer. This operator constructs a Writer struct with the appropriate implementation for writing to the arena.
     * \return A Writer struct that can be used to write to the arena using the Writer interface.
     */
    operator Writer();

    /** 
     * \brief Static method to create a new arena with the specified capacity. This method allocates the necessary memory for the arena and initializes it.
     * \param capacity The total capacity of the arena, in bytes.
     * \param outArena Pointer to a variable that will receive the pointer to the newly created arena.
     * \return A Result indicating success or failure of the arena creation. If there is not enough memory to create the arena, it returns nbl::error::OUT_OF_MEMORY.
     */
    static Result create(usize capacity, Arena* outArena);

    /**
     * \brief Returns a pointer to a thread-local arena. This allows for efficient memory allocation that is specific to the current thread. The thread-local arena is typically used for temporary allocations that do not need to be shared between threads, and it can help reduce contention and improve performance in multi-threaded applications.
      * \return A pointer to the thread-local arena for the current thread.
     */
    static Arena* threadLocal();

private:

    void* _pData;
    usize _mCapacity;
    usize _mCommitted;
    usize _mIndex;

};


/** 
 * \brief A helper class that represents a frame of allocations within an arena. An ArenaFrame allows for efficient management of temporary allocations within an arena, and it automatically releases all allocations made within the frame when it goes out of scope.
 */
class ArenaFrame {
public:

    /** 
     * \brief Default constructor that initializes the ArenaFrame with no associated arena and zeroed indices.
     */
    ArenaFrame() :
        _pParent(nullptr),
        _pArena(nullptr),
        _mStartIndex(0),
        _mLastIndex(0)
    { }

    /** 
     * \brief Constructor that initializes the ArenaFrame with a specific arena. The frame starts at the current index of the arena, and it will manage allocations made within that arena until the frame goes out of scope.
     * \param arena Pointer to the arena that this frame will manage allocations for.
     */
    ArenaFrame(Arena* arena) :
        _pParent(nullptr),
        _pArena(arena),
        _mStartIndex(arena->index()),
        _mLastIndex(arena->index())
    { }

    explicit ArenaFrame(ArenaFrame* parent) :
        _pParent(parent),
        _pArena(parent->arena()),
        _mStartIndex(parent->_mLastIndex),
        _mLastIndex(parent->_mLastIndex)
    { }

    /** 
     * \brief Destructor that automatically releases all allocations made within the frame. When the frame goes out of scope, it pops all allocations made since the frame was created, effectively resetting the arena to the state it was in when the frame was initialized.
     */
    ~ArenaFrame() {
        if(_pArena && _mLastIndex > _mStartIndex) {
            pop(_mLastIndex - _mStartIndex);
        }
    }

    /** 
     * \brief Returns the current index within the frame, which represents the total amount of memory allocated within the frame so far. This is calculated as the current index of the arena minus the starting index of the frame.
     * \return The current index within the frame.
     */
    inline usize index() const {
        return _pArena->index() - _mStartIndex;
    }

    /** 
     * \brief Returns the total capacity of the frame, which represents the total amount of memory that can be allocated within the frame before reaching the capacity of the arena. This is calculated as the total capacity of the arena minus the starting index of the frame.
     * \return The remaining capacity within the frame.
     */
    inline usize capacity() const {
        return _pArena->capacity() - _mStartIndex;
    }

    /** 
     * \brief Returns a pointer to the arena that this frame is managing.
     * \return A pointer to the arena that this frame is managing.
     */
    inline Arena* arena() const {
        return _pArena;
    }

    /** 
     * \brief Releases a specified number of bytes from the frame. This operation simply bumps the frame's start index forward by the specified number of bytes, effectively "giving back" or "releasing" memory to the frame's parent arena. The memory that is released is no longer owned by the frame, and will not be automatically freed when the frame goes out of scope. 
     * \param bytes The number of bytes to release from the frame.
     * \return A Result indicating success or failure of the release operation. If the specified number of bytes exceeds the current index within the frame, it returns nbl::error::OUT_OF_BOUNDS and does not modify the arena.
     */
    inline Result release(usize bytes) {
        NBL_ASSERT(_parentIndex() == _mLastIndex);
        return releaseUnsafe(bytes);
    }

    /**
     * \brief Releases the specified number of bytes from the frame without asserting that the arena's current index matches the frame's last index. If the specified number of bytes exceeds the current index within the frame, it returns nbl::error::OUT_OF_BOUNDS and does not modify the arena.
      * \param bytes The number of bytes to release from the frame.
     */
    inline Result releaseUnsafe(usize bytes) {
        if(bytes > index()) {
            return nbl::error::OUT_OF_BOUNDS;
        }
        _mStartIndex += bytes;
        _setLastIndex(max(_mLastIndex, _mStartIndex));

        return Result::SUCCESS;
    }

    /**
     * Releases all memory allocated within the frame. This operation simply bumps the frame's start index to be the current index of its parent arena, effectively "giving back" or "releasing" all of its memory to its parent arena. The memory that is released is no longer owned by the frame, and will not be automatically freed when the frame goes out of scope.
      * \return A Result indicating success or failure of the release operation.
     */
    inline void release() {
        NBL_ASSERT(_parentIndex() == _mLastIndex);
        releaseUnsafe();
    }

    /**
     * Releases all memory allocated within the frame without asserting that the arena's current index matches the frame's last index. 
     * \return A Result indicating success or failure of the release operation.
     */
    inline void releaseUnsafe() {

        _setLastIndex(_pArena->index());
        _mStartIndex = _mLastIndex;
    }

    /**
     * \brief Retrieves a pointer to a specific offset within the frame.
     * \param offset The offset from the start of the frame to retrieve the pointer for.
     * \param outPtr Pointer to a variable that will receive the pointer to the specified offset within the frame.
     * \return A Result indicating success or failure of the operation. If the offset is out of bounds (i.e., if it exceeds the current index within the frame), it returns nbl::error::OUT_OF_BOUNDS.
     */
    inline Result get(usize offset, void** outPtr) const {
        return _pArena->get(_mStartIndex + offset, outPtr);
    }

    /** 
     * \brief Returns a pointer to the current position within the frame where the next allocation will occur. This allows for direct access to the memory that will be allocated next within the frame.
     * \return A pointer to the current position within the frame.
     */
    inline void* current() const {
        NBL_ASSERT(_parentIndex() == _mLastIndex);
        return _pArena->current();
    }

    inline void* begin() const {
        NBL_ASSERT(_parentIndex() == _mLastIndex);
        return static_cast<char*>(_pArena->begin()) + _mStartIndex;
    }

    /** 
     * \brief Allocates a block of memory of the specified size within the frame and returns a pointer to it. 
     * \param outPtr Pointer to a variable that will receive the pointer to the allocated block of memory within the frame.
     * \return A Result indicating success or failure of the allocation. If there is not enough space in the arena to allocate the requested block, it returns nbl::error::OUT_OF_MEMORY.
     */
    inline Result push(usize size, void** outPtr) {
        NBL_ASSERT(_parentIndex() == _mLastIndex);
        NBL_CHECK(_pArena->push(size, outPtr));
        _setLastIndex(_pArena->index());
        return Result::SUCCESS;
    }

    /** 
     * \brief Allocates a block of memory of the specified size within the frame and copy-constructs an object of type T in that memory. 
     * \param value The value to construct in the allocated memory.
     * \return A Result indicating success or failure of the allocation and construction. If there is not enough space in the arena to allocate the requested block, it returns nbl::error::OUT_OF_MEMORY.
     */
    template<typename T>
    inline Result pushValue(const T& value) {
        NBL_ASSERT(_parentIndex() == _mLastIndex);
        T* ptr;
        NBL_CHECK(_pArena->pushValue(value));
        _setLastIndex(_pArena->index());
        return Result::SUCCESS;
    }

    /**
     * \brief Allocates a block of memory big enough to hold count objects of type T within the frame. 
     * \param outPtr Pointer to a variable that will receive the pointer to the allocated block of memory within the frame, which can hold count objects of type T.
     * \param count The number of objects of type T that the allocated block of memory should be able to hold. Defaults to 1 if not specified.
     * \return A Result indicating success or failure of the allocation and construction. If there is not enough space in the arena to allocate the requested block, it returns nbl::error::OUT_OF_MEMORY.
     */
    template<typename T>
    inline Result allocate(T** outPtr, usize count = 1) {
        return push(sizeof(T) * count, reinterpret_cast<void**>(outPtr));
    }

    /** 
     * \brief Allocates a block of memory within the frame and copies the contents of the specified string into that memory. 
     * \param str The StringView representing the string to be copied into the allocated memory within the frame.
     * \return A Result indicating success or failure of the allocation and copy operation. If there is not enough space in the arena to allocate the requested block, it returns nbl::error::OUT_OF_MEMORY.
     */
    inline Result pushString(StringView str) {
        NBL_ASSERT(_parentIndex() == _mLastIndex);
        NBL_CHECK(_pArena->pushString(str));
        _setLastIndex(_pArena->index());
        return Result::SUCCESS;
    }

    /** 
     * \brief Pops a specified number of bytes from the frame, effectively deallocating that memory and making it available for future allocations within the frame. The offset for the pop operation is relative to the start index of the frame.
     * \param count The number of bytes to pop from the frame.
     * \return The new index within the frame after popping the specified number of bytes. If the count exceeds the current index within the frame, it returns 0 and does not modify the arena.
     */
    inline usize pop(usize count) {
        NBL_ASSERT(_parentIndex() == _mLastIndex);
        usize newIndex = _pArena->pop(count);
        _setLastIndex(newIndex);
        return newIndex - _mStartIndex;
    }

    /** 
     * \brief Returns a Writer that can be used to write to the frame using the Writer interface. 
     * \return A Writer struct that can be used to write to the frame using the Writer interface.
     */
    inline Writer writer() {
        return Writer(this, _write);
    }

    /** 
     * \brief Implicit conversion operator that allows an ArenaFrame to be used as a Writer.
     * \return A Writer struct that can be used to write to the arena managed by the frame using the Writer interface.
     */
    inline operator Writer() {
        return writer();
    }

private:


    usize _parentIndex() const {
        return _pParent ? _pParent->_mLastIndex : _pArena->index();
    }

    void _setLastIndex(usize index) {
        _mLastIndex = index;
        if(_pParent) {
            _pParent->_setLastIndex(index);
        }
    }

    static Result _write(void* impl, const void* data, usize size, usize* bytesWritten) {
        ArenaFrame* frame = static_cast<ArenaFrame*>(impl);
        NBL_ASSERT(frame->_parentIndex() == frame->_mLastIndex);

        Writer writer = *frame->arena();
        NBL_CHECK(writer.write(data, size, bytesWritten));
        frame->_setLastIndex(frame->arena()->index());

        return Result::SUCCESS;
    }

    ArenaFrame* _pParent;
    Arena* _pArena;
    usize _mStartIndex;
    usize _mLastIndex;

};



}
