#pragma once

#include <Core/String.hpp>
#include <Platform/Mutex.hpp>
#include <functional>
#include "Plugin/PluginContext.hpp"

namespace nbl {

class LoginTracker {
public:

    inline Result startTracking(void* userData) {
        auto lock = _mMutex.frameLock();

        if(!_bIdle) return nbl::Result::INVALID_STATE;
        _bIdle = false;
        _pUserData = userData;
        return nbl::Result::SUCCESS;
    }

    inline void finishTracking(bool success, StringView message) {
        auto lock = _mMutex.frameLock();

        if(_bIdle) return;
        _pfnFinished(success, message, _pUserData);
        _bIdle = true;
    }
    inline void postStatus(StringView message) {
        auto lock = _mMutex.frameLock();
        if(_bIdle) return;
        _pfnStatus(message, _pUserData);
    }

    inline bool isTracking() { 
        auto lock = _mMutex.frameLock();
        return !_bIdle; 
    }

    static Result create(void (*pfnFinished)(bool success, StringView message, void* userData), void (*pfnStatus)(StringView message, void* userData), LoginTracker* outTracker) {
        NBL_CHECK(nbl::Mutex::create(&outTracker->_mMutex));
        outTracker->_pfnFinished = pfnFinished;
        outTracker->_pfnStatus = pfnStatus;
        outTracker->_bIdle = true;
        outTracker->_pUserData = nullptr;
        return nbl::Result::SUCCESS;
    }

private:
    bool _bIdle;
    void* _pUserData;
    void (*_pfnFinished)(bool, StringView, void*);
    void (*_pfnStatus)(StringView, void*);
    Mutex _mMutex;
};


struct AccountInfo {
    StringView userName;
    StringView icon;
    u32 id;
    bool iconNearestScaling;
};

typedef std::function_ref<Result(const AccountInfo&)> AccountCallback;


struct AccountProvider {

    enum class Type {
        BASIC = 0,
        OAUTH = 1,
        TOKEN = 2
    };

    enum class OAuthResponseType {
        NONE = 0,
        CODE = 1,
        TOKEN = 2
    };

    StringView id;
    StringView name;
    Type type;
    u32 maxAccounts;
    union {
        struct {
            Result(*pfnLogin)(PluginContext* context, StringView username, StringView password, LoginTracker* tracker, void* userData);
            Result(*pfnLogout)(PluginContext* context, u32 accountId, void* userData);
            void* userData;
            StringView usernameText = "Username";
            StringView passwordText = "Password";

            Result login(PluginContext* context, StringView username, StringView password, LoginTracker* tracker) const {
                if(!pfnLogin) return nbl::Result::PLUGIN_ERROR;
                return pfnLogin(context, username, password, tracker, userData);
            }
        } basicData;
        struct {
            Result(*pfnFinishLogin)(PluginContext* context, StringView response, StringView codeVerifier, LoginTracker* tracker, void* userData);
            Result(*pfnLogout)(PluginContext* context, u32 accountId, void* userData);
            void* userData;
            OAuthResponseType responseType;
            StringView clientId;
            StringView loginUrl;
            Span<StringView> scopes;
            u16 port;

            Result finishOAuthLogin(PluginContext* context, StringView response, StringView codeVerifier, LoginTracker* tracker) const {
                if(!pfnFinishLogin) return nbl::Result::PLUGIN_ERROR;
                return pfnFinishLogin(context, response, codeVerifier, tracker, userData);
            }

        } oAuthData;
        struct {
            Result(*pfnLogin)(PluginContext* context, StringView token, LoginTracker* tracker, void* userData);
            Result(*pfnLogout)(PluginContext* context, u32 accountId, void* userData);
            void* userData;
            StringView tokenText = "Token";

            Result login(PluginContext* context, StringView token, LoginTracker* tracker) const {
                if(!pfnLogin) return nbl::Result::PLUGIN_ERROR;
                return pfnLogin(context, token, tracker, userData);
            }
        } tokenData;
    };

    struct {
        Result(*pfnGet)(PluginContext* context, u32 accountId, AccountInfo* info, void* userData);
        Result(*pfnEnumerate)(PluginContext* context, AccountCallback callback, void* userData);
        void* userData;
    } accountInfo;

    Result logout(PluginContext* context, u32 accountId) const {
        NBL_ASSERT(type <= Type::TOKEN);
        if(type == Type::BASIC) {
            if(!basicData.pfnLogout) return nbl::Result::PLUGIN_ERROR;
            return basicData.pfnLogout(context, accountId, basicData.userData);
        }
        if(type == Type::OAUTH) {
            if(!oAuthData.pfnLogout) return nbl::Result::PLUGIN_ERROR;
            oAuthData.pfnLogout(context, accountId, basicData.userData);
        }

        if(!tokenData.pfnLogout) return nbl::Result::PLUGIN_ERROR;
        return tokenData.pfnLogout(context, accountId, oAuthData.userData);
    }

    Result getAccountInfo(PluginContext* context, u32 accountId, AccountInfo* info) const {
        if(!accountInfo.pfnGet) return nbl::Result::PLUGIN_ERROR;
        return accountInfo.pfnGet(context, accountId, info, accountInfo.userData);
    }

    Result enumerateAccountInfo(PluginContext* context, AccountCallback callback) const {
        if(!accountInfo.pfnEnumerate) return nbl::Result::PLUGIN_ERROR;
        return accountInfo.pfnEnumerate(context, callback, accountInfo.userData);
    }
};


}
