/*
 * Copyright 2026 jwall64
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Tab/TabFilterModel.hpp"
#include "ApplicationContext.hpp"

#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>
#include <Data/FilterModel.hpp>

QVariant nbl::frontend::TabFilterModel::data(const QModelIndex& index, int role) const {

    if(!index.isValid() || index.row() >= _mData.size()) return QVariant();

    FilterMeta ref = _mData.at(index.row());

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
        case IconIsImageRole:
            return ref.iconIsImage();
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
    roles[IconIsImageRole] = "iconIsImage";
    return roles;
}


nbl::Result nbl::frontend::TabFilterModel::_fetchPageData(qsizetype limit, qsizetype offset) {

    nbl::DatabaseConnection* db = ApplicationContext::instance()->databaseConnection();
    
    QList<FilterMeta> found = {};
    found.reserve(limit);
    NBL_CHECK(nbl::Filter::enumerateForTab(db, _mTabId, offset, limit, [&](const nbl::Filter& meta) -> nbl::Result {
        found.append(FilterMeta(
            meta.filterId, 
            QByteArray(meta.name.data, meta.name.length), 
            inlineIcon(meta.icon, meta.flags & FilterFlag::IMAGE_ICON),
            meta.flags
        ));
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
