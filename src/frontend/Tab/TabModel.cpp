#include "ApplicationContext.hpp"
#include "Tab/TabModel.hpp"
#include "Tab/TabRef.hpp"

#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>


QVariant nbl::frontend::TabModel::data(const QModelIndex& index, int role) const {

    if(!index.isValid() || index.row() >= _mData.size()) return QVariant();

    const TabRef& ref = _mData.at(index.row());

    switch(role) {
        case IdRole:
            return ref.tabId();
        case NameRole:
            return ref.name();
        case IconRole:
            return ref.icon();
        case ColorizeRole:
            return ref.colorizeIcon();
        case IconIsImageRole:
            return ref.iconIsImage();
        case HiddenRole:
            return ref.hidden();
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> nbl::frontend::TabModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[IconRole] = "icon";
    roles[ColorizeRole] = "colorizeIcon";
    roles[IconIsImageRole] = "iconIsImage";
    roles[HiddenRole] = "hidden";
    return roles;
}

nbl::Result nbl::frontend::TabModel::_fetchPageData(qsizetype limit, qsizetype offset) {


    nbl::DatabaseConnection* db = ApplicationContext::instance()->databaseConnection();
    if(db == nullptr) {
        return nbl::Result::INVALID_STATE;
    }
    
    QList<TabRef> found = {};
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
