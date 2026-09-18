#include <Plugin/Registry.hpp>
#include "Platform/Logging.hpp"

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
        
        nbl::log(nbl::LogLevel::INFO, "start of ", i, " is ", allocation + runningOffset);
        outReg->_pStarts[i] = allocation + runningOffset;

        offsets[i] = runningOffset;
        runningOffset += _mCounts[i];
    }

    for(usize i = 0 ; i < _mTotal ; i++) {
        const _RegisteredObject& obj = _pBegin[i];
        usize offset = offsets[obj.registry.value] + registryMoved[obj.registry.value];
        registryMoved[obj.registry.value]++;


        nbl::log(nbl::LogLevel::INFO, "Put a ", static_cast<usize>(obj.registry.value), " at ", allocation + offset);
        allocation[offset] = obj.value;
    }

    memcpy(outReg->_mCounts, _mCounts, sizeof(_mCounts));

    return nbl::Result::SUCCESS;
}

