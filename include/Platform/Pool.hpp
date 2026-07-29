#pragma once

#include <Core/Types.hpp>
#include <Core/Result.hpp>
#include <Platform/Arena.hpp>

#include <cstring>

namespace nbl {

template<typename T>
class Pool {
public:

    Pool() = default;

    ~Pool() noexcept {
        for(T& t : *this) {
            (&t)->~T();
        }
    }

    Result push(T** outPtr) {
        
        T* out = nullptr;
        if(_pFirstFree == _mArena.end()) {
            NBL_CHECK(_mArena.push(sizeof(T), reinterpret_cast<void**>(&out)));
            _pFirstFree = _mArena.end();
        } else {
            void* nextFree = *static_cast<void**>(_pFirstFree);
            out = static_cast<T*>(_pFirstFree);
            _pFirstFree = nextFree;
        }

        memset(out, 0, sizeof(T));
        new(out) T();

        if(outPtr) *outPtr = out;
        return nbl::Result::SUCCESS;
    }
    Result pop(T* value) {

        void* voidValue = static_cast<void*>(value);
        if(voidValue < _mArena.begin() || voidValue >= _mArena.end()) {
            return nbl::Result::OUT_OF_BOUNDS;
        }

        value->~T();
        memset(voidValue, 0, sizeof(T));

        if(_pFirstFree > voidValue) {
            void* nextFree = _pFirstFree;
            _pFirstFree = voidValue;
            *static_cast<void**>(_pFirstFree) = nextFree;
            return nbl::Result::SUCCESS;
        }

        // Keep free list in order, so iterator works easily
        void* current = _pFirstFree;
        void* prev = nullptr;
        while(current < voidValue) {
            prev = current;
            current = *static_cast<void**>(current);
        }

        *static_cast<void**>(prev) = voidValue;
        *static_cast<void**>(voidValue) = current;

        return nbl::Result::SUCCESS;
    }

    const T* get(usize index) const {
        const T* ptr = static_cast<const T*>(_mArena.begin()) + index;
        if(ptr >= _mArena.end()) return nullptr;
        void* currentFree = _pFirstFree;
        while(currentFree <= ptr) {
            if(currentFree == ptr) return nullptr;
            currentFree = *static_cast<void**>(currentFree);
        }
        return ptr;
    }

    T* get(usize index) {
        T* ptr = static_cast<T*>(_mArena.begin()) + index;
        if(ptr >= _mArena.end()) return nullptr;
        void* currentFree = _pFirstFree;
        while(currentFree <= ptr) {
            if(currentFree == ptr) return nullptr;
            currentFree = *static_cast<void**>(currentFree);
        };
        return ptr;
    }

    Result indexOf(void* ptr, usize* index) {
        if(ptr < _mArena.begin() || ptr > _mArena.end()) return nbl::Result::OUT_OF_BOUNDS;

        usize diff = static_cast<u8*>(ptr) - static_cast<u8*>(_mArena.begin());
        *index = diff / sizeof(T);

        return nbl::Result::SUCCESS;
    }

    static Result create(usize numItems, Pool* outPool) {
        NBL_CHECK(nbl::Arena::create(numItems * sizeof(T), &outPool->_mArena));
        outPool->_pFirstFree = outPool->_mArena.end();

        return nbl::Result::SUCCESS;
    }

private:

    Arena _mArena;
    void* _pFirstFree;

    friend class Iterator;
    friend class ConstIterator;

public:

    class Iterator { 
    public:

        T& operator*() {
            NBL_ASSERT(_pCurrent < _pParent->_mArena.end());
            return *_pCurrent;
        }

        T* operator->() {
            NBL_ASSERT(_pCurrent < _pParent->_mArena.end());
            return _pCurrent;
        }

        Iterator& operator++(int) {
            _pCurrent++;
            _skip();
            return *this;
        }

        Iterator operator++() {
            Iterator out = *this;
            _pCurrent++;
            _skip();
            return out;
        }

        bool operator!=(const Iterator& other) const {
            return other._pParent != _pParent || other._pCurrent != _pCurrent;
        }

    private:

        Iterator(void* nextSkip, T* current, Pool* parent) :
            _pNextSkip(nextSkip),
            _pCurrent(current),
            _pParent(parent)
        { }

        void _skip() {
            while(_pCurrent < _pParent->_mArena.end() && _pCurrent == _pNextSkip) {
                _pCurrent++;
                _pNextSkip = *static_cast<void**>(_pNextSkip);
            }
        }

        void* _pNextSkip;
        T* _pCurrent;
        Pool* _pParent;

        friend class Pool;
    };


    class ConstIterator { 
    public:

        const T& operator*() const {
            NBL_ASSERT(_pCurrent < _pParent->_mArena.end());
            return *_pCurrent;
        }

        const T* operator->() const {
            NBL_ASSERT(_pCurrent < _pParent->_mArena.end());
            return _pCurrent;
        }

        ConstIterator& operator++(int) {
            _pCurrent++;
            _skip();
            return *this;
        }

        ConstIterator operator++() {
            Iterator out = *this;
            _pCurrent++;
            _skip();
            return out;
        }

        bool operator!=(const Iterator& other) const {
            return other._pParent != _pParent || other._pCurrent != _pCurrent;
        }

    private:

        ConstIterator(void* nextSkip, T* current, Pool* parent) :
            _pNextSkip(nextSkip),
            _pCurrent(current),
            _pParent(parent)
        { }

        void _skip() {
            while(_pCurrent < _pParent->_mArena.end() && _pCurrent == _pNextSkip) {
                _pCurrent++;
                _pNextSkip = *static_cast<void**>(_pNextSkip);
            }
        }

        void* _pNextSkip;
        T* _pCurrent;
        Pool* _pParent;

        friend class Pool;
    };

    Iterator begin() {
        auto out = Iterator(_pFirstFree, static_cast<T*>(_mArena.begin()), this);
        out._skip();
        return out;
    }
    Iterator end() {
        return Iterator(_mArena.end(), static_cast<T*>(_mArena.end()), this);
    }

    ConstIterator begin() const {
        auto out = ConstIterator(_pFirstFree, static_cast<T*>(_mArena.begin()), this);
        out._skip();
        return out;
    }
    ConstIterator end() const {
        return ConstIterator(_mArena.end(), static_cast<T*>(_mArena.end()), this);
    }

};

}
