#include <Models/NblAllTabsModel.hpp>
#include <ApplicationContext.hpp>

#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>

#include "NblTabRef.hpp"

QVariant NblAllTabsModel::data(const QModelIndex& index, int role) const {

    if(!index.isValid() || index.row() >= _mData.size()) return QVariant();

    const NblTabRef& ref = _mData.at(index.row());

    switch(role) {
        case IdRole:
            return ref.tabId();
        case NameRole:
            return ref.name();
        case IconRole:
            return ref.icon();
        case ColorizeRole:
            return ref.colorizeIcon();
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> NblAllTabsModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[IconRole] = "icon";
    roles[ColorizeRole] = "colorizeIcon";
    return roles;
}

nbl::Result NblAllTabsModel::_fetchPageData(qsizetype limit, qsizetype offset) {


    nbl::DatabaseConnection* db = ApplicationContext::instance()->databaseConnection();
    if(db == nullptr) {
        return nbl::Result::INVALID_STATE;
    }
    
    QList<NblTabRef> found = {};
    found.reserve(_mPageSize);
    NBL_CHECK(nbl::enumerateTabs(db, offset, limit, [&](const nbl::TabMeta& tab) -> nbl::Result {
        found.emplace_back(tab);
        return nbl::Result::SUCCESS;
    }));

    if(found.size() > 0) {
        beginInsertRows(QModelIndex(), _mData.size(), _mData.size() + found.size() - 1);
        _mData.reserve(found.size());
        for(auto& ref : found) {
            _mData.append(ref);
        }
        endInsertRows();
    }

    return nbl::Result::SUCCESS;
}
