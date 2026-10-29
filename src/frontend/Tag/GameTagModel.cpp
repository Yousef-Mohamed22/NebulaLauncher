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

#include "Tag/GameTagModel.hpp"
#include "ApplicationContext.hpp"

#include <Platform/Logging.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>
#include <Data/TagModel.hpp>

QVariant nbl::frontend::GameTagModel::data(const QModelIndex& index, int role) const {

    if(!index.isValid() || index.row() >= _mData.size()) return QVariant();

    TagMeta ref = _mData.at(index.row());

    switch(role) {
        case IdRole:
            return ref.id;
        case NameRole:
            return ref.name;
        case ColorRole:
            return ref.color;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> nbl::frontend::GameTagModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[ColorRole] = "color";
    return roles;
}


nbl::Result nbl::frontend::GameTagModel::_fetchPageData(qsizetype limit, qsizetype offset) {

    nbl::DatabaseConnection* db = ApplicationContext::instance()->databaseConnection();
    
    QList<TagMeta> found = {};
    found.reserve(limit);
    NBL_CHECK(nbl::Tag::enumerateGameTags(db, _mGameId, [&](const nbl::Tag& tag) -> nbl::Result {
        TagMeta ref(tag.id, QString::fromUtf8(tag.name.data(), tag.name.length()), QColor::fromRgb(tag.color)); 
        found.append(ref);
        return nbl::Result::SUCCESS;
    }))

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
