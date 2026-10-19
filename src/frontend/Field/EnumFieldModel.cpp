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
#include "Field/EnumFieldModel.hpp"
#include "ApplicationContext.hpp"

int nbl::frontend::EnumFieldModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
    if(_pField == nullptr) return 0;
    return static_cast<int>(_pField->enumData.count(ApplicationContext::instance()->pluginContext()));
}

QVariant nbl::frontend::EnumFieldModel::data(const QModelIndex& index, int role) const {
    if(_pField == nullptr) return QVariant();

    const nbl::Field& field = *_pField;
    if(field.type != nbl::FieldType::ENUM) return QVariant();

    switch(role) {
        case NameRole: {
            nbl::StringView name = field.enumData.name(ApplicationContext::instance()->pluginContext(), static_cast<u32>(index.row()));
            return QByteArray(name.data, name.length);
        }
        case DisplayNameRole: {
            nbl::StringView name = field.enumData.displayName(ApplicationContext::instance()->pluginContext(), static_cast<u32>(index.row()));
            return QByteArray(name.data, name.length);
        }
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> nbl::frontend::EnumFieldModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[NameRole] = "name";
    roles[DisplayNameRole] = "displayName";
    return roles;
}

quint32 nbl::frontend::EnumFieldModel::indexOf(QByteArray name) {
    nbl::StringView nameView = nbl::StringView(name.data(), name.length());
    if(_pField == nullptr) return 0;
    return _pField->enumData.index(ApplicationContext::instance()->pluginContext(), nameView);
}
