#pragma once

#include <Core/String.hpp>
#include <Platform/Mutex.hpp>
#include <functional>

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
    void postStatus(StringView message) {
        auto lock = _mMutex.frameLock();
        if(_bIdle) return;
        _pfnStatus(message, _pUserData);
    }

    bool isTracking() { 
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
};

typedef std::function_ref<Result(const AccountInfo&)> AccountCallback;


struct AccountProvider {

    enum class Type {
        BASIC = 0,
        OAUTH = 1
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
            Result(*pfnLogin)(StringView username, StringView password, LoginTracker* tracker, void* userData);
            Result(*pfnLogout)(u32 accountId, void* userData);
            void* userData;

            Result login(StringView username, StringView password, LoginTracker* tracker) const {
                return pfnLogin(username, password, tracker, userData);
            }
        } basicData;
        struct {
            Result(*pfnFinishLogin)(StringView response, LoginTracker* tracker, void* userData);
            Result(*pfnLogout)(u32 accountId, void* userData);
            void* userData;
            OAuthResponseType responseType;
            StringView clientId;
            StringView reirectUrl;
            Span<StringView> scopes;

            Result finishOAuthLogin(StringView response, LoginTracker* tracker) const {
                return pfnFinishLogin(response, tracker, userData);
            }

        } oAuthData;
    };

    struct {
        Result(*pfnGet)(u32 accountId, AccountInfo* info, void* userData);
        Result(*pfnEnumerate)(AccountCallback callback, void* userData);
        void* userData;
    } accountInfo;

    Result logout(u32 accountId) const {
        NBL_ASSERT(type == Type::BASIC || type == Type::OAUTH);
        if(type == Type::BASIC) return basicData.pfnLogout(accountId, basicData.userData);
        return oAuthData.pfnLogout(accountId, oAuthData.userData);
    }

    Result getAccountInfo(u32 accountId, AccountInfo* info) const {
        return accountInfo.pfnGet(accountId, info, accountInfo.userData);
    }

    Result enumerateAccountInfo(AccountCallback callback) const {
        return accountInfo.pfnEnumerate(callback, accountInfo.userData);
    }
};


}
