#include "NblAccountsModel.hpp"

#include "ApplicationContext.hpp"

void NblAccountsModel::refresh() {
    beginResetModel();
    _mData.clear();

    if(_pProvider) {
        _pProvider->enumerateAccountInfo(ApplicationContext::instance()->pluginContext(), [&](const nbl::AccountInfo& info) -> nbl::Result {
            _mData.append({
                QString::fromUtf8(info.userName.data, info.userName.length), 
                QString::fromUtf8(info.icon.data, info.icon.length), 
                info.id, 
                info.iconNearestScaling
            });
            return nbl::Result::SUCCESS;
        });
    }

    endResetModel();
}
