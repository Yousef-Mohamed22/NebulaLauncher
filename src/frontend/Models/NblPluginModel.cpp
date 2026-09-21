#include "NblPluginModel.hpp"
#include "ApplicationContext.hpp"
#include "Codec/Json.hpp"
#include "Data/Model.hpp"
#include "Plugin/Field.hpp"
#include "Plugin/PluginContext.hpp"

#include <Plugin/PluginLoader.hpp>
#include <Platform/StringBuilder.hpp>

int NblPluginModel::rowCount(const QModelIndex& parent) const {
    ApplicationContext* ctx = ApplicationContext::instance();
    return ctx->pluginContext()->loadedPlugins().length;
}

QVariant NblPluginModel::data(const QModelIndex& index, int role) const {

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

                nbl::ArenaFrame frame(nbl::Arena::threadLocal());
                nbl::Span<nbl::FieldValue> values;
                NBL_CHECK(frame.allocate(&values.data, plugin.fields.length));

                nbl::JsonElement element;
                NBL_CHECK(nbl::parseJson(nbl::StringReader(fields.fieldsJson), &frame, &element));
                if(element.type != nbl::JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;

                nbl::decodeFieldValues(plugin.fields, element.object, &values, &frame);
                qtFields = NblFieldRef(&dummy).fromNbl({ .compositeValue = values });

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

QHash<int, QByteArray> NblPluginModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[VersionRole] = "version";
    roles[DescriptionRole] = "description";
    roles[FieldCountRole] = "fieldCount";
    roles[FieldValuesRole] = "fieldValues";
    return roles;
}

NblFieldList* NblPluginModel::fields(int row) {
    ApplicationContext* ctx = ApplicationContext::instance();
    const nbl::LoadedPlugin& plugin = ctx->pluginContext()->loadedPlugins()[row];

    return new NblFieldList(plugin.fields, this);
}

void NblPluginModel::saveFields(QVariantMap fields, int row) {

    ApplicationContext* ctx = ApplicationContext::instance();
    const nbl::LoadedPlugin& plugin = ctx->pluginContext()->loadedPlugins()[row];

    nbl::Field dummy {
        .type = nbl::FieldType::STRUCT,
        .structData = { .fields = plugin.fields }
    };

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::FieldValue value = NblFieldRef(&dummy).toNbl(fields, &frame);

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
                nbl::formatPlaceholders(value.compositeValue[i].stringValue.raw, &applied, placeholders, field.stringData.strictPlaceholders);
                NBL_ASSERT(applied.buildNullTerminated(&value.compositeValue[i].stringValue.resolved).success());
            }
        }
    }

    if(plugin.pfnUpdateCallback) {
        plugin.pfnUpdateCallback(plugin.fields, value.compositeValue, plugin.userData);
    }

    nbl::StringBuilder builder(&frame);
    nbl::Result res = nbl::encodeFieldValues(plugin.fields, value.compositeValue, builder);
    if(res.error()) {
        ctx->pluginContext()->pushNotification(nbl::NotificationType::ERROR, "An error occurred", "Unable to save plugin configuration");
        return;
    }
    nbl::StringView json;
    builder.buildNullTerminated(&json);

    nbl::setPluginFields(ctx->databaseConnection(), plugin.meta.id, json, plugin.meta.version);
}
