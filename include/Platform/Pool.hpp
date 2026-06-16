// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Pool.hpp
 * \author jwall64
 * \brief A simple implementation of a pool allocator that allows for efficient allocation and deallocation of fixed-size objects.
 */

#pragma once

#include <Core/Types.hpp>
#include <Platform/Arena.hpp>

namespace nbl {

template<typename T>
class PoolView;

/**
 * \brief A simple implementation of a pool allocator that allows for efficient allocation and deallocation of fixed-size objects. The pool uses an underlying arena for memory management and maintains a free list to track available slots for allocation.
 */
class Pool {
public:

    /**
     * \brief Default constructor that initializes the pool with an empty arena and a stride of 0. The pool is not usable until it is properly initialized with an arena and a stride.
     */
    Pool() = default;
    
    /**
     * \brief Gets a pointer to the object at the specified index in the pool. 
     * \param index The index of the object to retrieve from the pool.
     * \param outPtr Pointer to a variable that will receive the pointer to the object at the specified index in the pool.
     * \return A Result indicating success or failure of the operation. 
     */
    const Result get(usize index, void** outPtr) const {
        usize byteIndex = index * _mStride;
        usize maxIndex = _mArena.capacity();
        if(byteIndex >= maxIndex) {
            return nbl::error::OUT_OF_BOUNDS;
        }
        return _mArena.get(index * _mStride, outPtr);
    }

    /**
     * \brief Pushes a new object onto the pool and returns a pointer to it. 
     * \param outPtr Pointer to a variable that will receive the pointer to the newly allocated object in the pool.
     * \return A Result indicating success or failure of the operation.
     */
    Result push(void** outPtr) {
        if(_mFirstFree >= static_cast<char*>(_mArena.current()) + _mArena.capacity()) {

            return nbl::error::OUT_OF_MEMORY;

        } else if(_mFirstFree == _mArena.current()) {

            NBL_CHECK(_mArena.push(_mStride, outPtr));
            _mFirstFree = _mArena.current();

        } else if(_mFirstFree < _mArena.current() && _mFirstFree >= _mArena.begin()) {

            *outPtr = _mFirstFree;
            _mFirstFree = *static_cast<void**>(_mFirstFree);
            memset(*outPtr, 0, _mStride);

        } else {

            return nbl::error::UNEXPECTED_ERROR;
        }
        return Result::SUCCESS;
    }

    /**
     * \brief Pops an object from the pool, effectively deallocating it and making that slot available for future allocations. 
     * \param value Pointer to the object that will be popped from the pool. The pointer must point to an object that was previously allocated from the pool.
     * \return A Result indicating success or failure of the operation.
     */
    Result pop(void* ptr) {
        if(ptr < _mArena.begin() || ptr >= _mArena.current()) {
            return nbl::error::OUT_OF_BOUNDS;
        }

        // Walk the free list to make sure it stays sorted
        void* current = _mFirstFree;
        void* prev = ptr;
                
        while(current < ptr) {
            prev = current;
            current = *reinterpret_cast<void**>(current);
        }

        *reinterpret_cast<void**>(prev) = ptr;
        *reinterpret_cast<void**>(ptr) = current;
        if(ptr < _mFirstFree) _mFirstFree = ptr;

        return Result::SUCCESS;
    }

    /**
     * \brief Gets the index of the specified object in the pool. 
     * \param value Pointer to the object whose index is to be retrieved. The pointer must point to an object that is currently allocated in the pool.
     * \param outIndex Pointer to a variable that will receive the index of the specified object in the pool.
     * \return A Result indicating success or failure of the operation. 
     */
    Result index(void* ptr, usize* outIndex) const {
        if(ptr < _mArena.begin() || ptr >= _mArena.current()) {
            return nbl::error::OUT_OF_BOUNDS;
        }
        usize byteIndex = static_cast<char*>(ptr) - static_cast<char*>(_mArena.begin());
        if(byteIndex % _mStride != 0) {
            return nbl::error::ILLEGAL_ARGUMENT;
        }
        *outIndex = byteIndex / _mStride;
        return Result::SUCCESS;
    }

    /**
     * \brief Returns a view of the pool as a PoolView of the specified type T. The stride of the pool must match the size of type T for the view to be valid. If the stride does not match, an assertion will fail.
     * \tparam T The type of the objects in the pool. The size of type T must match the stride of the pool for the view to be valid.
     * \return A PoolView<T> that provides typed access to the objects in the pool. The PoolView allows for getting, pushing, and popping objects of type T from the pool using typed pointers.
     */
    template<typename T>
    PoolView<T> view();

    /**
     * \brief Creates a new pool with the specified stride using the provided arena for memory management. 
     * \param arena Pointer to the Arena that will be used for memory management of the pool.
     * \param stride The size of each object in the pool, in bytes. This determines how much memory is allocated for each slot in the pool.
     * \param outPool Pointer to a variable that will receive the created Pool instance.
     * \return A Result indicating success or failure of the operation. 
     */
    static Result create(Arena* arena, usize stride, Pool* outPool) {

        outPool->_mArena = ArenaFrame(arena);
        outPool->_mStride = nbl::align(stride, sizeof(void*));
        outPool->_mFirstFree = arena->current();
        return Result::SUCCESS;
    }

private:

    usize _firstFree() {
        return reinterpret_cast<usize>(_mFirstFree);
    }

    ArenaFrame _mArena;
    usize _mStride;
    void* _mFirstFree;

    template<typename T>
    friend class PoolView;

};


/**
 * \brief A typed view of a Pool that provides typed access to the objects in the pool. The PoolView allows for getting, pushing, and popping objects of type T from the pool using typed pointers. The stride of the underlying pool must match the size of type T for the view to be valid.
 * \tparam T The type of the objects in the pool. The size of type T must match the stride of the underlying pool for the view to be valid.
 */
template<typename T>
class PoolView {
public:

    /**
     * \brief Gets a pointer to the object at the specified index in the pool view. The index is multiplied by the stride of the underlying pool to calculate the byte offset within the arena. If the index is out of bounds, an error result is returned.
     * \param index The index of the object to retrieve from the pool view.
     * \param out Pointer to a variable that will receive the pointer to the object at the specified index in the pool view.
     * \return A Result indicating success or failure of the operation. If the index is out of bounds, it returns nbl::error::OUT_OF_BOUNDS. If the operation is successful, out will point to the object at the specified index in the pool view.
     */
    const Result get(usize index, T** out) const {
        return _mPool.get(index, reinterpret_cast<void**>(out));
    }

    /**
     * \brief Pushes a new object onto the pool view and returns a pointer to it. The allocated object will be of type T, and the pointer returned will be typed accordingly. If there is not enough memory in the underlying pool to allocate a new slot, an error result is returned.
     * \param outPtr Pointer to a variable that will receive the pointer to the newly allocated object of type T in the pool view.
     * \return A Result indicating success or failure of the operation. If there is not enough memory in the underlying pool to allocate a new slot, it returns nbl::error::OUT_OF_MEMORY. If the operation is successful, outPtr will point to the newly allocated object of type T in the pool view.
     */
    Result push(T** outPtr) {
        NBL_CHECK(_mPool.push(reinterpret_cast<void**>(outPtr)));
        new (*outPtr) T();
        return Result::SUCCESS;
    }

    /**
     * \brief Pops an object from the pool view, effectively deallocating it and making that slot available for future allocations. 
     * \param value Pointer to the object of type T that will be popped from the pool view. The pointer must point to an object that was previously allocated from the pool view, and it will be deallocated and added back to the free list of the underlying pool.
     * \return A Result indicating success or failure of the operation. If the index is out of bounds, it returns nbl::error::OUT_OF_BOUNDS. If the operation is successful, the object at the specified index in the pool view will be deallocated and that slot will be added back to the free list of the underlying pool.
     */
    Result pop(T* value) {
        return _mPool.pop(value);
    }

    /**
     * \brief Gets the index of the specified object in the pool view. 
     * \param value Pointer to the object of type T whose index is to be retrieved. The pointer must point to an object that is currently allocated in the pool view.
     * \param outIndex Pointer to a variable that will receive the index of the specified object in the pool view.
     * \return A Result indicating success or failure of the operation. If the index is out of bounds, it returns nbl::error::OUT_OF_BOUNDS. If the operation is successful, outIndex will be set to the index of the specified object in the pool view.
     */
    Result index(T* value, usize* outIndex) const {
        return _mPool.index(value, outIndex);
    }

    /**
     * \brief Creates a new PoolView of type T from the provided arena and an output variable for the pool. 
     * \param arena Pointer to the Arena that will be used for memory management of the underlying pool.
     * \param outPool Pointer to a variable that will receive the created Pool instance that the PoolView will wrap.
     * \return A PoolView<T> that provides typed access to the objects in the newly created pool. The PoolView allows for getting, pushing, and popping objects of type T from the pool using typed pointers.
     */
    static Result create(Arena* arena, PoolView* outPool) {
        return Pool::create(arena, sizeof(T), &outPool->_mPool);
    }

private:
    Pool _mPool;

    friend class Pool;
    friend class Iterator;

public:

    class Iterator { 
    public:

        bool operator==(const Iterator& other) const {
            return _mIndex == other._mIndex;
        }

        bool operator!=(const Iterator& other) const {
            return _mIndex != other._mIndex;
        }

        Iterator& operator++() {
            T* ptr;
            do {
                _mIndex++;
                if(_pView->get(_mIndex, &ptr).error()) {
                    _mIndex = _pView->_mPool._mArena.index() / _pView->_mPool._mStride;
                    return *this;
                }
            } while(_isFree(ptr));
            return *this;
        }

        Iterator operator++(int) {
            Iterator temp = *this;
            ++(*this);
            return temp;
        }

        T* operator*() const {
            T* ptr;
            if(_pView->get(_mIndex, &ptr).error()) {
                return nullptr;
            }
            return ptr;
        }

    private:
        PoolView* _pView;
        usize _mIndex;
        void* _pNextFree;

        bool _isFree(void* ptr) {
            if(ptr == _pView->_mPool._mArena.current()) {
                return false;
            }
            if(ptr == _pNextFree) {
                _pNextFree = *static_cast<void**>(ptr);
                return true;
            }
            return false;
        }

        friend class PoolView;
    };

    Iterator begin() {
        Iterator it;
        it._pView = this;
        it._mIndex = 0;
        it._pNextFree = _mPool._mFirstFree;

        T* ptr;
        if(get(0, &ptr).error()) {
            return end();
        }

        while(it._isFree(ptr)) {
            it._mIndex++;
            if(get(it._mIndex, &ptr).error()) {
                return end();
            }
        }

        return it;
    }

    Iterator end() {
        Iterator it;
        it._pView = this;
        it._mIndex = _mPool._mArena.index() / _mPool._mStride;
        return it;
    }

};

template<typename T>
PoolView<T> Pool::view() {
    NBL_ASSERT(_mStride == sizeof(T));
    PoolView<T> view;
    view._mPool = *this;
    return view;
}



}
