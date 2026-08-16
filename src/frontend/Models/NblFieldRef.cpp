#include "NblFieldRef.hpp"
#include <qjsvalue.h>
#include "NblEnumFieldModel.hpp"
#include "NblFieldList.hpp"

namespace {

QVariant _fieldToQt(const nbl::Field* field, nbl::FieldValue value) {
    switch(field->type) {
        case nbl::FieldType::STRING: 
            return QString::fromUtf8(field->defaultValue.stringValue.data, field->defaultValue.stringValue.length);
        case nbl::FieldType::INTEGER:
            return static_cast<qint64>(field->defaultValue.integerValue);
        case nbl::FieldType::FLOAT: 
            return field->defaultValue.floatValue;
        case nbl::FieldType::BOOLEAN: 
            return field->defaultValue.booleanValue;
        case nbl::FieldType::ACCOUNT: 
        case nbl::FieldType::ENUM: 
            return static_cast<quint32>(field->defaultValue.enumValue);
        case nbl::FieldType::DATE: 
            return static_cast<quint64>(field->defaultValue.uintegerValue);
        case nbl::FieldType::LIST: {
            QVariantList out;
            for(nbl::FieldValue& value : value.compositeValue) {
                out.append(_fieldToQt(field->listData.templateField, value));
            }
            return out;
        }
        case nbl::FieldType::STRUCT: {
            QVariantList out;
            for(auto i = 0 ; i < field->structData.fields.length ; i++) {
                out.append(_fieldToQt(&field->structData.fields[i], value.compositeValue[i]));
            }
            return out;
        }
    }
    return QVariant();
}

}

quint32 NblFieldRef::type() {
    return _pField->type;
}

QString NblFieldRef::key() {
    return QString::fromUtf8(_pField->key.data, _pField->key.length);
}

QString NblFieldRef::name() {
    return QString::fromUtf8(_pField->name.data, _pField->name.length);
}

QVariant NblFieldRef::min() {
    switch(_pField->type) {
        case nbl::FieldType::INTEGER: {
            return static_cast<qint64>(_pField->integerData.min);
        }
        case nbl::FieldType::FLOAT: {
            return _pField->floatData.min;
        }
        case nbl::FieldType::LIST: {
            return static_cast<quint32>(_pField->listData.min);
        }
        default:
            return QVariant();
    }
}

QVariant NblFieldRef::max() {
    switch(_pField->type) {
        case nbl::FieldType::INTEGER: {
            return static_cast<qint64>(_pField->integerData.max);
        }
        case nbl::FieldType::FLOAT: {
            return _pField->floatData.max;
        }
        case nbl::FieldType::LIST: {
            return static_cast<quint32>(_pField->listData.max);
        }
        default:
            return QVariant();
    }
}

QVariant NblFieldRef::defaultValue() {
    return _fieldToQt(_pField, _pField->defaultValue);
}

NblFieldRef* NblFieldRef::listTemplate() {
    if(_pField->type != nbl::FieldType::LIST) return nullptr;
    return new NblFieldRef(_pField->listData.templateField, this);
}

NblFieldList* NblFieldRef::structFields() {
    if(_pField->type != nbl::FieldType::STRUCT) return nullptr;
    return new NblFieldList(_pField->structData.fields, this);

}


NblEnumFieldModel* NblFieldRef::enumModel() {

    return new NblEnumFieldModel(this);
}


void NblFieldRef::suggest(QString current, QJSValue value) {
    if(_pField->type != nbl::FieldType::STRING || !value.isCallable()) return;

    QByteArray currentUtf = current.toUtf8();
    _pField->stringData.suggest(nbl::StringView(currentUtf.data(), currentUtf.length()), [&](nbl::StringView suggestion) {
        QJSValueList args{};
        args.append(QString::fromUtf8(suggestion.data, suggestion.length));
        value.call(args);
    });

}
