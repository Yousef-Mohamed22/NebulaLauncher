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
 * \file Core/Either.hpp
 * \author jwall64
 * \brief Contains a type-safe union of two types
 */

#pragma once

#include <Core/Types.hpp>
#include <Core/Math.hpp>
#include <Core/Assert.hpp>

#include <utility>
#include <new>

namespace nbl {

template<typename L, typename R>
class Either {
public:


    Either() :
        _bLeft(false)
    { }


    Either(const L& value) :
        _bLeft(true)
    {
        new (_mValue) L(value);
    }
    
    Either(const R& value) :
        _bLeft(false)
    {
        new (_mValue) R(value);
    }
    

    Either(L&& value) :
        _bLeft(true)
    {
        new (_mValue) L(std::move(value));
    }

    Either(R&& value) :
        _bLeft(true)
    {
        new (_mValue) R(std::move(value));
    }


    Either(const Either& other) :
        _bLeft(other._bLeft)
    {
        if(_bLeft) {
            new (_mValue) L(*reinterpret_cast<const L*>(other._mValue));
        } else {
            new (_mValue) R(*reinterpret_cast<const R*>(other._mValue));
        }
    }


    Either(Either&& other) :
        _bLeft(other._bLeft)
    {
        if(_bLeft) {
            new (_mValue) L(std::move(*reinterpret_cast<L*>(other._mValue)));
        } else {
            new (_mValue) R(std::move(*reinterpret_cast<R*>(other._mValue)));
        }
    }


    ~Either() {
        if(_bLeft) {
            reinterpret_cast<L*>(_mValue)->~L();
        } else {
            reinterpret_cast<R*>(_mValue)->~R();
        }
    }


    Either& operator=(const Either& other) {
        if(this == &other) return *this;
        this->~Either();

        _bLeft = other._bLeft;
        if(_bLeft) {
            new (_mValue) L(*reinterpret_cast<const L*>(other._mValue));
        } else {
            new (_mValue) R(*reinterpret_cast<const R*>(other._mValue));
        }

        return *this;
    }


    Either& operator=(Either&& other) {
        if(this == &other) return *this;
        this->~Either();

        _bLeft = other._bLeft;
        if(_bLeft) {
            new (_mValue) L(std::move(*reinterpret_cast<L*>(other._mValue)));
        } else {
            new (_mValue) R(std::move(*reinterpret_cast<R*>(other._mValue)));
        }

        return *this;
    }

    Either& operator=(const L& value) {
        this->~Either();
        _bLeft = true;
        new (_mValue) L(value);
        return *this;
    }
    

    Either& operator=(L&& value) {
        this->~Either();
        _bLeft = true;
        new (_mValue) L(std::move(value));
        return *this;
    }

    Either& operator=(const R& value) {
        this->~Either();
        _bLeft = false;
        new (_mValue) R(value);
        return *this;
    }
    

    Either& operator=(R&& value) {
        this->~Either();
        _bLeft = false;
        new (_mValue) R(std::move(value));
        return *this;
    }


    bool hasLeft() const {
        return _bLeft;
    }

    bool hasRight() const {
        return !_bLeft;
    }

    L* left() {
        if(!_bLeft) return nullptr;
        return reinterpret_cast<L*>(_mValue);
    }

    const L* left() const {
        if(!_bLeft) return nullptr;
        return reinterpret_cast<const L*>(_mValue);
    }

    R* right() {
        if(_bLeft) return nullptr;
        return reinterpret_cast<R*>(_mValue);
    }

    const R* right() const {
        if(_bLeft) return nullptr;
        return reinterpret_cast<const R*>(_mValue);
    }

private:
    u8 _mValue[nbl::max(sizeof(L), sizeof(R))];
    bool _bLeft;
};

}
