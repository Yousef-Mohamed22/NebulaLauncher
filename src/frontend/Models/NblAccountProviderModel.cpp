#include "NblAccountProviderModel.hpp"

#include <Platform/Process.hpp>
#include <Plugin/AccountProvider.hpp>
#include <Plugin/PluginContext.hpp>

#include <qstringview.h>

namespace {

struct {

    nbl::LoginTracker tracker;
    u32 current = 0;
    bool setup = false;
    QByteArray username{};
    QByteArray password{};

    nbl::Result startTracking(void* userData) {
        if(!setup) {
            setup = true;
            NBL_CHECK(nbl::LoginTracker::create(
                [](bool success, nbl::StringView message, void* userData) -> void {
                    NblAccountProviderModel* model = static_cast<NblAccountProviderModel*>(userData);
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


i32 _sThreadProc(void* userData) {

    nbl::PluginContext* ctx = ApplicationContext::instance()->pluginContext();
    const nbl::AccountProvider& prov = ctx->accountProviders().data[_sTracker.current];

    nbl::Result result = prov.basicData.login(
        nbl::StringView(_sTracker.username.data(), _sTracker.username.length()),
        nbl::StringView(_sTracker.password.data(), _sTracker.password.length()),
        &_sTracker.tracker
    );
    _sTracker.username = {};
    _sTracker.password = {};

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
    const nbl::AccountProvider& type = ctx->accountProviders().data[index.row()];

    switch(role) {
        case IdRole:
            return QByteArray(type.id.data, type.id.length);
        case NameRole:
            return QByteArray(type.name.data, type.name.length);
        case MaxAccountsRole:
            return type.maxAccounts;
        case AccountTypeRole:
            return static_cast<u32>(type.type);
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
    const nbl::AccountProvider& type = ctx->accountProviders().data[index];
    _sTracker.current = index;
    _sTracker.username = username.toUtf8();
    _sTracker.password = password.toUtf8();


    nbl::Thread thread{};
    res = nbl::Thread::create(_sThreadProc, this, &thread);

    if(res.error()) {
        ctx->pushNotification(nbl::NotificationType::ERROR, "Login failed", "Failed to spawn a login thread");
        return;
    }
}

void NblAccountProviderModel::loginOAuth(quint32 index) {
    nbl::log(nbl::LogLevel::INFO, "Login oauth");
}

void NblAccountProviderModel::logout(quint32 index, quint32 accountId) {

    nbl::PluginContext* ctx = ApplicationContext::instance()->pluginContext();
    const nbl::AccountProvider& prov = ctx->accountProviders().data[index];

    prov.logout(accountId);
}

NblAccountsModel* NblAccountProviderModel::accounts(quint32 index) {

    nbl::PluginContext* ctx = ApplicationContext::instance()->pluginContext();
    const nbl::AccountProvider& type = ctx->accountProviders().data[index];

    return new NblAccountsModel(&type, this);
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



