#pragma once

#include <Core/Types.hpp>
#include <Core/String.hpp>
#include <Core/Result.hpp>
#include <Platform/Allocator.hpp>



namespace nbl {

struct InstallationType;
struct AccountProvider;
struct MetadataProvider;
struct GameProvider;
struct AchievementProvider;

class PluginRegistry {
public:

    enum _Value : usize {
        INSTALLATION_TYPE,
        ACCOUNT_PROVIDER,
        METADATA_PROVIDER,
        GAME_PROVIDER,
        ACHIEVEMENT_PROVIDER,
        _END
    };

    constexpr static usize count() { return _END; }

    template<typename T>
    struct FromClass {
        constexpr static _Value value = _Value::_END;
    };

    PluginRegistry(usize value) : value(static_cast<_Value>(value)) {}
    PluginRegistry(_Value value) : value(value) {}

    _Value value;

    operator _Value() { return value; }
    operator usize() { return static_cast<usize>(value); }
};


#define _CREATE_REGISTRY_TYPE(enumName, className) \
    template<> \
    struct PluginRegistry::FromClass<className> { \
        constexpr static _Value value = _Value::enumName; \
    };

_CREATE_REGISTRY_TYPE(INSTALLATION_TYPE, InstallationType);
_CREATE_REGISTRY_TYPE(ACCOUNT_PROVIDER, AccountProvider);
_CREATE_REGISTRY_TYPE(METADATA_PROVIDER, MetadataProvider);
_CREATE_REGISTRY_TYPE(GAME_PROVIDER, GameProvider);
_CREATE_REGISTRY_TYPE(ACHIEVEMENT_PROVIDER, AchievementProvider);

#undef _CREATE_REGISTRY_TYPE

class Registry {
public:

    template<typename T>
    const T* get(StringView name) const {
        PluginRegistry reg = PluginRegistry::FromClass<T>::value;
        if(reg >= PluginRegistry::_END) return nullptr;

        const T** values = static_cast<const T**>(_pStarts[reg.value]);
        for(usize i = 0 ; i < _mCounts[reg.value] ; i++) {
            if(values[i]->id == name) return values[i];
        }

        return nullptr;
    }

    template<typename T>
    ConstSpan<const T*> get() const {
        PluginRegistry reg = PluginRegistry::FromClass<T>::value;
        if(reg >= PluginRegistry::_END) return {};

        const T** values = static_cast<const T**>(_pStarts[reg.value]);
        return ConstSpan(values, _mCounts[reg.value]);
    }

private:

    void* _pStarts[PluginRegistry::count()];
    usize _mCounts[PluginRegistry::count()];

public:

    class Builder {
    public:

        Builder(LinearAllocator* allocator) :
            _pAllocator(allocator),
            _pBegin(static_cast<_RegisteredObject*>(allocator->end())),
            _mCounts{},
            _mTotal(0)
        { }

        template<typename T>
        Result add(T* value) {
            return _add(PluginRegistry::FromClass<T>::value, value);
        }

        Result build(LinearAllocator* allocator, Registry* outReg);

    private:

        struct _RegisteredObject {
            PluginRegistry registry;
            void* value;
        };

        Result _add(PluginRegistry registry, void* value);

        LinearAllocator* _pAllocator;
        _RegisteredObject* _pBegin;
        usize _mCounts[PluginRegistry::count()];
        usize _mTotal;
    };

};



}
