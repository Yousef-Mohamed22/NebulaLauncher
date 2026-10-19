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
 * \file Plugin/Registry.cpp
 * \author jwall64
 * \brief Implementation of Plugin/Registry.hpp
 */

#include <Plugin/Registry.hpp>

nbl::Result nbl::Registry::Builder::_add(PluginRegistry registry, void* value) {

    if(registry >= PluginRegistry::_END) return nbl::Result::INVALID_ARGUMENT;

    _RegisteredObject* obj;
    NBL_CHECK(_pAllocator->allocate(&obj));

    *obj = _RegisteredObject(registry, value);
    _mTotal++;
    _mCounts[registry.value]++;

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::Registry::Builder::build(LinearAllocator* allocator, Registry* outReg) {

    if(_mTotal == 0) return nbl::Result::SUCCESS;

    void** allocation{};
    allocator->allocate(&allocation, _mTotal);

    usize registryMoved[PluginRegistry::count()]{};
    usize offsets[PluginRegistry::count()]{};

    usize runningOffset = 0;
    for(usize i = 0 ; i < PluginRegistry::count() ; i++) {
        
        outReg->_pStarts[i] = allocation + runningOffset;

        offsets[i] = runningOffset;
        runningOffset += _mCounts[i];
    }

    for(usize i = 0 ; i < _mTotal ; i++) {
        const _RegisteredObject& obj = _pBegin[i];
        usize offset = offsets[obj.registry.value] + registryMoved[obj.registry.value];
        registryMoved[obj.registry.value]++;
        allocation[offset] = obj.value;
    }

    memcpy(outReg->_mCounts, _mCounts, sizeof(_mCounts));

    return nbl::Result::SUCCESS;
}

