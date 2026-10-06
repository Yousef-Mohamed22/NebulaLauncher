#include "AccountProviderModel.hpp"
#include "ApplicationContext.hpp"

#include <Plugin/AccountProvider.hpp>

int nbl::frontend::AccountProviderModel::rowCount(const QModelIndex& parent) const {
    return ApplicationContext::instance()->pluginContext()->registry<AccountProvider>().length;
}

QVariant nbl::frontend::AccountProviderModel::data(const QModelIndex& index, int role) const {

    ConstSpan<const AccountProvider*> providers = ApplicationContext::instance()->pluginContext()->registry<AccountProvider>();
    if(index.row() >= providers.length) return {};

    const AccountProvider& prov = *providers[index.row()];

    switch(role) {
        case IdRole:
            return QString::fromUtf8(prov.id.data, prov.id.length);
        case NameRole:
            return QString::fromUtf8(prov.name.data, prov.name.length);
        case LoginTextRole:
            return QString::fromUtf8(prov.loginText.data, prov.loginText.length);
        case MaxAccountsRole:
            return prov.maxAccounts;
        default:
            return {};
    }
}

QHash<int, QByteArray> nbl::frontend::AccountProviderModel::roleNames() const {
    QHash<int, QByteArray> out = {};
    out[IdRole] = "id";
    out[NameRole] = "name";
    out[LoginTextRole] = "loginText";
    out[MaxAccountsRole] = "maxAccounts";
    return out;
}

nbl::frontend::AccountModel* nbl::frontend::AccountProviderModel::accountModel(int row) {

    ConstSpan<const AccountProvider*> providers = ApplicationContext::instance()->pluginContext()->registry<AccountProvider>();
    if(row >= providers.length) return nullptr;

    return new AccountModel(providers[row]);

}
