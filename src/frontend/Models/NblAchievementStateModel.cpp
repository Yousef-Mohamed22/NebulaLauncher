#include <Plugin/AchievementProvider.hpp>

#include "NblAchievementStateModel.hpp"
#include <qvariant.h>

#include "ApplicationContext.hpp"
#include "Data/Model.hpp"

int NblAchievementStateModel::rowCount(const QModelIndex& parent) const {
    return _mData.length();
}

QVariant NblAchievementStateModel::data(const QModelIndex& index, int role) const {

    if(index.row() >= rowCount()) return QVariant();
    
    ApplicationContext* ctx = ApplicationContext::instance();
    const _Item& item = _mData[index.row()];

    switch(role) {
        case IdRole:
            return QString::fromUtf8(item.provider->id.data, item.provider->id.length);
        case NameRole:
            return QString::fromUtf8(item.provider->name.data, item.provider->name.length);
        case StatusRole:
            return static_cast<quint32>(item.status);
        case FieldCountRole:
            if(item.status != nbl::AchievementProviderStatus::ADDED_BY_USER && item.status != nbl::AchievementProviderStatus::ADDED_BY_PROVIDER) return 0;
            return static_cast<quint32>(item.provider->fields.length);
        case FieldValuesRole: {
        
            if(item.status != nbl::AchievementProviderStatus::ADDED_BY_USER && item.status != nbl::AchievementProviderStatus::ADDED_BY_PROVIDER) return QVariant();

            nbl::Field dummy {
                .type = nbl::FieldType::STRUCT,
                .structData = { .fields = item.provider->fields }
            };

            QVariant qtFields = QVariantMap();
            nbl::ArenaFrame frame(nbl::Arena::threadLocal());
            nbl::Span<nbl::FieldValue> values;
            if(frame.allocate(&values.data, item.provider->fields.length).error()) return QVariant();

            nbl::JsonElement element;
            if(nbl::parseJson(nbl::StringReader(nbl::StringView(item.encodedFields.data(), item.encodedFields.length())), &frame, &element).error()) return QVariant();
            if(element.type != nbl::JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;

            if(nbl::decodeFieldValues(ctx->pluginContext(), item.provider->fields, element.object, &values, &frame).error()) return QVariant();
            qtFields = NblFieldRef(&dummy).fromNbl({ .compositeValue = values });

            return qtFields;
        }
    }
    return QVariant();
}


QHash<int, QByteArray> NblAchievementStateModel::roleNames() const {
    QHash<int, QByteArray> roles = {};
    roles[IdRole] = "id";
    roles[NameRole] = "name";
    roles[StatusRole] = "status";
    roles[FieldCountRole] = "fieldCount";
    roles[FieldValuesRole] = "fieldValues";
    return roles;
}


NblFieldList* NblAchievementStateModel::fields(quint32 index) {
    if(index >= _mData.length()) return nullptr;
    return new NblFieldList(_mData[index].provider->fields);
}

void NblAchievementStateModel::_refresh() {

    ApplicationContext* ctx = ApplicationContext::instance();
    nbl::ConstSpan<const nbl::AchievementProvider*> providers = ctx->pluginContext()->registry<nbl::AchievementProvider>();

    _mData.resizeForOverwrite(providers.length);
    qsizetype index = 0;
    for(const nbl::AchievementProvider* provider : providers) {
        _mData[index++] =_Item{
            .provider = provider,
            .status = nbl::AchievementProviderStatus::NOT_ADDED,
            .encodedFields = {}
        };
    }

    nbl::enumerateAchievementStates(ctx->databaseConnection(), _mGameId, [&](auto& state) -> nbl::Result {
        for(_Item& item : _mData) {
            if(item.provider->id == state.provider) {
                item.encodedFields = QByteArray(state.fields.data, state.fields.length);
                item.status = state.status;
                return nbl::Result::SUCCESS;
            }
        }
        return nbl::Result::SUCCESS;
    });

}

