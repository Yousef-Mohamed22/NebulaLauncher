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
#include "Plugin/PluginModel.hpp"
#include "ApplicationContext.hpp"

#include <Codec/Json.hpp>
#include <Data/Model.hpp>
#include <Plugin/Field.hpp>
#include <Plugin/PluginContext.hpp>
#include <Plugin/PluginLoader.hpp>
#include <Utility/StringBuilder.hpp>

int nbl::frontend::PluginModel::rowCount(const QModelIndex& parent) const {
    ApplicationContext* ctx = ApplicationContext::instance();
    return ctx->pluginContext()->loadedPlugins().length;
}

QVariant nbl::frontend::PluginModel::data(const QModelIndex& index, int role) const {

    if(index.row() >= rowCount()) return QVariant();
    
    ApplicationContext* ctx = ApplicationContext::instance();
    const nbl::LoadedPlugin& plugin = ctx->pluginContext()->loadedPlugins()[index.row()];

    switch(role) {
        case IdRole:
            return QByteArray(plugin.meta.id.data, plugin.meta.id.length);
        case NameRole:
            return QByteArray(plugin.meta.name.data, plugin.meta.name.length);
        case VersionRole:
            return QByteArray(plugin.meta.version.data, plugin.meta.version.length);
        case DescriptionRole:
            return QByteArray(plugin.meta.description.data, plugin.meta.description.length);
        case FieldCountRole:
            return static_cast<qsizetype>(plugin.fields.length);
        case FieldValuesRole: {
        
            QVariant qtFields = QVariantMap();

            nbl::Field dummy {
                .type = nbl::FieldType::STRUCT,
                .structData = { .fields = plugin.fields }
            };

            if(nbl::getPluginFields(ctx->databaseConnection(), plugin.meta.id, [&](const nbl::PluginFields& fields) -> nbl::Result {

                nbl::AllocationFrame frame(nbl::Arena::threadLocal());
                nbl::Span<nbl::FieldValue> values;
                NBL_CHECK(frame.allocate(&values.data, plugin.fields.length));

                nbl::JsonElement element;
                NBL_CHECK(nbl::parseJson(nbl::StringReader(fields.fieldsJson), &frame, &element));
                if(element.type != nbl::JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;

                nbl::decodeFieldValues(ctx->pluginContext(), plugin.fields, element.object, &values, &frame);
                qtFields = nbl::frontend::FieldRef(&dummy).fromNbl({ .compositeValue = values });

                return nbl::Result::SUCCESS;

            }).error()) {
                nbl::log(nbl::LogLevel::DEBUG, "An error occurred while loading plugin fields");
                return QVariantMap();
            }

            return qtFields;
        }
    }
    return QVariant();
}

QHash<int, QByteArray> nbl::frontend::PluginModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[VersionRole] = "version";
    roles[DescriptionRole] = "description";
    roles[FieldCountRole] = "fieldCount";
    roles[FieldValuesRole] = "fieldValues";
    return roles;
}

nbl::frontend::FieldList* nbl::frontend::PluginModel::fields(int row) {
    ApplicationContext* ctx = ApplicationContext::instance();
    const nbl::LoadedPlugin& plugin = ctx->pluginContext()->loadedPlugins()[row];

    return new nbl::frontend::FieldList(plugin.fields, this);
}

void nbl::frontend::PluginModel::saveFields(QVariantMap fields, int row) {

    ApplicationContext* ctx = ApplicationContext::instance();
    const nbl::LoadedPlugin& plugin = ctx->pluginContext()->loadedPlugins()[row];

    nbl::Field dummy {
        .type = nbl::FieldType::STRUCT,
        .structData = { .fields = plugin.fields }
    };

    nbl::AllocationFrame frame(nbl::Arena::threadLocal());
    nbl::FieldValue value = nbl::frontend::FieldRef(&dummy).toNbl(fields, &frame);

    nbl::Span<nbl::Placeholder> placeholders = {};
    for(usize i = 0 ; i < plugin.fields.length ; i++) {
        const nbl::Field& field = plugin.fields[i];
        if(field.type == nbl::FieldType::STRING) {

            usize numPlaceholders = field.stringData.customPlaceholders.length;
            if(numPlaceholders > 0) {
                NBL_ASSERT(frame.allocate(&placeholders.data, numPlaceholders).success());
                memcpy(placeholders.data, field.stringData.customPlaceholders.data, field.stringData.customPlaceholders.length * sizeof(nbl::Placeholder));
                placeholders.length = field.stringData.customPlaceholders.length;
                nbl::StringBuilder applied(&frame);
                nbl::formatPlaceholders(value.compositeValue[i].stringValue.raw, &applied, placeholders, field.flags & nbl::FieldFlag::STRICT_PLACEHOLDERS);
                NBL_ASSERT(applied.buildNullTerminated(&value.compositeValue[i].stringValue.resolved).success());
            } else {
                value.compositeValue[i].stringValue.resolved = value.compositeValue[i].stringValue.raw;
            }
        }
    }

    if(plugin.pfnUpdateCallback) {
        plugin.pfnUpdateCallback(plugin.fields, value.compositeValue, plugin.userData);
    }

    nbl::StringBuilder builder(&frame);
    nbl::Result res = nbl::encodeFieldValues(ctx->pluginContext(), plugin.fields, value.compositeValue, builder);
    if(res.error()) {
        ctx->pluginContext()->pushNotification(nbl::NotificationType::ERROR, "An error occurred", "Unable to save plugin configuration");
        return;
    }
    nbl::StringView json;
    builder.buildNullTerminated(&json);

    nbl::setPluginFields(ctx->databaseConnection(), plugin.meta.id, json, plugin.meta.version);
}
