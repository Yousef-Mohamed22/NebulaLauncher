#include "NblAccountProviderModel.hpp"

#include <Core/Random.hpp>
#include <Core/Array.hpp>
#include <Core/Stream.hpp>
#include <Platform/Process.hpp>
#include <Platform/Time.hpp>
#include <Platform/Random.hpp>
#include <Plugin/AccountProvider.hpp>
#include <Plugin/PluginContext.hpp>
#include <Network/HttpServer.hpp>
#include <Utility/Hash.hpp>

#include <QDesktopServices>

#include "ApplicationContext.hpp"
#include "Codec/Base64.hpp"

namespace {

void _stopServer();

struct {

    nbl::LoginTracker tracker;
    u32 current = 0;
    bool setup = false;

    struct {
        QByteArray username;
        QByteArray password;
    } basicData;
    struct {
        nbl::OAuthHttpServer server;
        nbl::Array<char, 128> challenge;
    } oAuthData;
    struct {
        QByteArray token;
    } tokenData;

    nbl::Result startTracking(void* userData) {
        if(!setup) {
            setup = true;
            NBL_CHECK(nbl::LoginTracker::create(
                [](bool success, nbl::StringView message, void* userData) -> void {

                    if(!success) {                  
                        nbl::PluginContext* ctx = ApplicationContext::instance()->pluginContext();
                        ctx->pushNotification(nbl::NotificationType::ERROR, "Failed to log in", message);
                    }

                    NblAccountProviderModel* model = static_cast<NblAccountProviderModel*>(userData);
                    _stopServer();
                    emit model->loginFinished(success, QString::fromUtf8(message.data, message.length));
                }, 
                [](nbl::StringView message, void* userData) -> void {
                    NblAccountProviderModel* model = static_cast<NblAccountProviderModel*>(userData);
                    emit model->loginStatus(QString::fromUtf8(message.data, message.length));
                },
                &tracker));
        }
        return tracker.startTracking(userData);
    }

} _sTracker;

void _stopServer() {
    _sTracker.oAuthData.server.close();
}


i32 _sThreadProc(void* userData) {

    nbl::PluginContext* ctx = ApplicationContext::instance()->pluginContext();
    const nbl::AccountProvider* prov = ctx->accountProviders().data[_sTracker.current];

    nbl::Result result = prov->basicData.login(
        nbl::StringView(_sTracker.basicData.username.data(), _sTracker.basicData.username.length()),
        nbl::StringView(_sTracker.basicData.password.data(), _sTracker.basicData.password.length()),
        &_sTracker.tracker
    );
    _sTracker.basicData.username = {};
    _sTracker.basicData.password = {};

    if(result.error()) {
        ctx->pushNotification(nbl::NotificationType::ERROR, "Login Failed", "An error occurred while logging in");
    }

    return 0;
}

nbl::Result _sServerHandler(nbl::StringView path, void* userData) {

    nbl::PluginContext* ctx = ApplicationContext::instance()->pluginContext();
    const nbl::AccountProvider* prov = ctx->accountProviders().data[_sTracker.current];
    
    nbl::StringView codeVerifier(_sTracker.oAuthData.challenge.data, 128);

    if(prov->oAuthData.responseType == nbl::AccountProvider::OAuthResponseType::NONE) {
        _sTracker.tracker.postStatus("Logging in");
        return prov->oAuthData.finishOAuthLogin("", codeVerifier, &_sTracker.tracker);
    }

    QString qpath = QString::fromUtf8(path.data, path.length);
    QStringList parts = qpath.split("?");

    if(parts.length() != 2) {
        _sTracker.tracker.finishTracking(false, "Invalid response");
        return nbl::Result::FORMAT_ERROR;
    }

    QStringList params = parts[1].split("&");
    for(QString param : params) {
        QStringList paramParts = param.split("=");
        if(paramParts.length() != 2) {
            _sTracker.tracker.finishTracking(false, "Invalid response");
            return nbl::Result::FORMAT_ERROR;
        }

        switch(prov->oAuthData.responseType) {
            default:
            case nbl::AccountProvider::OAuthResponseType::CODE: {
                if(paramParts[0] != "code") continue;
                QByteArray arr = paramParts[1].toUtf8();
                nbl::StringView code(arr.data(), arr.length());
                _sTracker.tracker.postStatus("Logging in");
                return prov->oAuthData.finishOAuthLogin(code, codeVerifier, &_sTracker.tracker);
            }
            case nbl::AccountProvider::OAuthResponseType::TOKEN: {
                if(paramParts[0] != "token") continue;
                QByteArray arr = paramParts[1].toUtf8();
                nbl::StringView token(arr.data(), arr.length());
                _sTracker.tracker.postStatus("Logging in");
                return prov->oAuthData.finishOAuthLogin(token, codeVerifier, &_sTracker.tracker);
            }
        }

    }

    _sTracker.tracker.finishTracking(false, "Invalid response");
    return nbl::Result::FORMAT_ERROR;
}

i32 _sTokenThreadProc(void* userData) {

    nbl::PluginContext* ctx = ApplicationContext::instance()->pluginContext();
    const nbl::AccountProvider* prov = ctx->accountProviders().data[_sTracker.current];

    nbl::Result result = prov->tokenData.login(
        nbl::StringView(_sTracker.tokenData.token.data(), _sTracker.tokenData.token.length()),
        &_sTracker.tracker
    );
    _sTracker.tokenData.token = {};

    if(result.error()) {
        ctx->pushNotification(nbl::NotificationType::ERROR, "Login Failed", "An error occurred while logging in");
    }

    return 0;
}

}



int NblAccountProviderModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
    nbl::PluginContext* ctx = ApplicationContext::instance()->pluginContext();
    if(ctx == nullptr) return 0;
    return ctx->accountProviders().length;
}

QVariant NblAccountProviderModel::data(const QModelIndex& index, int role) const {
    if(index.row() >= rowCount()) return QVariant();
    
    nbl::PluginContext* ctx = ApplicationContext::instance()->pluginContext();
    const nbl::AccountProvider* type = ctx->accountProviders().data[index.row()];

    switch(role) {
        case IdRole:
            return QByteArray(type->id.data, type->id.length);
        case NameRole:
            return QByteArray(type->name.data, type->name.length);
        case MaxAccountsRole:
            return type->maxAccounts;
        case AccountTypeRole:
            return static_cast<u32>(type->type);
        case LoggingInRole:
            return _sTracker.tracker.isTracking() && _sTracker.current == static_cast<u32>(index.row());
        default:
            return QVariant();
    }
}

void NblAccountProviderModel::loginBasic(quint32 index, QString username, QString password) {

    nbl::PluginContext* ctx = ApplicationContext::instance()->pluginContext();
    nbl::Result res = _sTracker.startTracking(this);
    if(res.error()) {
        ctx->pushNotification(nbl::NotificationType::ERROR, "Cannot log in", "Already trying to login to another account");
        return;
    }
    _sTracker.current = index;
    _sTracker.basicData.username = username.toUtf8();
    _sTracker.basicData.password = password.toUtf8();


    nbl::Thread thread{};
    res = nbl::Thread::create(_sThreadProc, this, &thread);

    if(res.error()) {
        _sTracker.tracker.finishTracking(false, "Failed to spawn a login thread");
        return;
    }
}

void NblAccountProviderModel::loginOAuth(quint32 index) {

    nbl::PluginContext* ctx = ApplicationContext::instance()->pluginContext();
    nbl::Result res = _sTracker.startTracking(this);
    if(res.error()) {
        ctx->pushNotification(nbl::NotificationType::ERROR, "Cannot log in", "Already trying to login to another account");
        return;
    }
    const nbl::AccountProvider* type = ctx->accountProviders().data[index];
    _sTracker.current = index;
 
    u16 port = type->oAuthData.port;
    if(port == 0) {
        nbl::Random random{};
        random.seed(nbl::getCurrentTime());
        port = 40000 + (random.next() % 10000);
    } 

    nbl::Array<u8,128> challengeBytes;
    res = nbl::secureRandom(challengeBytes); 
    if(res.error()) {
        _sTracker.tracker.finishTracking(false, "Failed to generate a PKCE challenge");
        return;
    }

    res = nbl::encodeBase64(challengeBytes, _sTracker.oAuthData.challenge, nbl::Base64CharacterSet::URL_SAFE);
    if(res.error()) {
        _sTracker.tracker.finishTracking(false, "Failed to generate a PKCE challenge");
        return;
    }

    nbl::Array<u8,32> hash{};
    res = nbl::sha256(nbl::BufferStream(_sTracker.oAuthData.challenge.span()), hash);
    if(res.error()) {
        _sTracker.tracker.finishTracking(false, "Failed to generate a PKCE challenge");
        return;
    }

    nbl::Array<char, 43> hashB64{};
    res = nbl::encodeBase64(hash, hashB64, nbl::Base64CharacterSet::URL_SAFE);
    if(res.error()) {
        _sTracker.tracker.finishTracking(false, "Failed to generate a PKCE challenge");
        return;
    }

    QString challenge = QString::fromUtf8(hashB64.data, hashB64.size());

    QString url = QString::fromUtf8(type->oAuthData.loginUrl.data, type->oAuthData.loginUrl.length);
    url += "?client_id=" + QString::fromUtf8(type->oAuthData.clientId.data, type->oAuthData.clientId.length);
    url += "&code_challenge=" + challenge;
    url += "&code_challenge_method=S256";
    url += "&response_type=";
    switch(type->oAuthData.responseType) {
        default:
        case nbl::AccountProvider::OAuthResponseType::NONE:
            url += "none";
            break;
        case nbl::AccountProvider::OAuthResponseType::CODE:
            url += "code";
            break;
        case nbl::AccountProvider::OAuthResponseType::TOKEN:
            url += "token";
            break;
    }

    url += "&redirect_uri=http://localhost:" + QString::number(static_cast<quint16>(port));
    
    if(type->oAuthData.scopes.length > 0) {
        url += "&scope=";
        for(usize i = 0 ; i < type->oAuthData.scopes.length ; i++) {
            if(i > 0) {
                url += "%20";
            }
            url += QString::fromUtf8(type->oAuthData.scopes[i].data, type->oAuthData.scopes[i].length);
        }
    }

    res = nbl::OAuthHttpServer::create(port, _sServerHandler, &_sTracker.oAuthData.server, &_sTracker.oAuthData.server);
    if(res.error()) {
        _sTracker.tracker.finishTracking(false, "Failed to start an OAuth callback server");
        return;
    }

    _sTracker.tracker.postStatus("Awaiting OAuth response");
    QDesktopServices::openUrl(QUrl(url));

}

void NblAccountProviderModel::loginToken(quint32 index, QString token) { 
    nbl::PluginContext* ctx = ApplicationContext::instance()->pluginContext();
    nbl::Result res = _sTracker.startTracking(this);
    if(res.error()) {
        ctx->pushNotification(nbl::NotificationType::ERROR, "Cannot log in", "Already trying to login to another account");
        return;
    }
    _sTracker.current = index;
    _sTracker.tokenData.token = token.toUtf8();

    nbl::Thread thread{};
    res = nbl::Thread::create(_sTokenThreadProc, this, &thread);

    if(res.error()) {
        _sTracker.tracker.finishTracking(false, "Failed to spawn a login thread");
        return;
    }
}

void NblAccountProviderModel::logout(quint32 index, quint32 accountId) {

    nbl::PluginContext* ctx = ApplicationContext::instance()->pluginContext();
    const nbl::AccountProvider* prov = ctx->accountProviders().data[index];

    prov->logout(accountId);
}

NblAccountsModel* NblAccountProviderModel::accounts(quint32 index) {

    nbl::PluginContext* ctx = ApplicationContext::instance()->pluginContext();
    const nbl::AccountProvider* type = ctx->accountProviders().data[index];

    return new NblAccountsModel(type, this);
}

QHash<int, QByteArray> NblAccountProviderModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[MaxAccountsRole] = "maxAccounts";
    roles[AccountTypeRole] = "accountType";
    roles[LoggingInRole] = "loggingIn";
    return roles;
}



