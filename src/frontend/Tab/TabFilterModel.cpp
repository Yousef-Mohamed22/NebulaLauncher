#include "Tab/TabFilterModel.hpp"
#include "ApplicationContext.hpp"

#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>

QVariant nbl::frontend::TabFilterModel::data(const QModelIndex& index, int role) const {

    if(!index.isValid() || index.row() >= _mData.size()) return QVariant();

    FilterRef ref = _mData.at(index.row());

    switch(role) {
        case IdRole:
            return static_cast<qint64>(ref.filterId());
        case NameRole:
            return ref.name();
        case IconRole:
            return ref.icon();
        case ColorizeRole:
            return ref.colorizeIcon();
        case MultiLineRole:
            return ref.multiLine();
        case UseIconsRole:
            return ref.useIcons();
        case ShowTitlesRole:
            return ref.showTitles();
        case ShowInstallationsRole:
            return ref.showInstallations();
        case ShowFilterInfoRole:
            return ref.showFilterInfo();
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> nbl::frontend::TabFilterModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[IconRole] = "icon";
    roles[ColorizeRole] = "colorizeIcon";
    roles[MultiLineRole] = "multiLine";
    roles[ShowTitlesRole] = "showTitles";
    roles[UseIconsRole] = "useIcons";
    roles[ShowInstallationsRole] = "showInstallations";
    roles[ShowFilterInfoRole] = "showFilterInfo";
    return roles;
}


nbl::Result nbl::frontend::TabFilterModel::_fetchPageData(qsizetype limit, qsizetype offset) {

    nbl::DatabaseConnection* db = ApplicationContext::instance()->databaseConnection();
    
    QList<FilterRef> found = {};
    found.reserve(limit);
    NBL_CHECK(nbl::getFiltersForTab(db, _mTabId, offset, limit, [&](const nbl::FilterMeta& meta) -> nbl::Result {
        FilterRef ref(
            meta.filterId, 
            QByteArray(meta.name.data, meta.name.length), 
            QByteArray(meta.icon.data, meta.icon.length),
            meta.flags
        );
        found.append(ref);
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
