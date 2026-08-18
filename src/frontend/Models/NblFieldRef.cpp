#include "NblFieldRef.hpp"
#include "NblEnumFieldModel.hpp"
#include "NblFieldList.hpp"

#include <Core/String.hpp>
#include <Core/Math.hpp>
#include <Plugin/Field.hpp>
#include <Platform/Allocator.hpp>
#include <Platform/StringBuilder.hpp>

namespace {

QVariant _fieldToQt(const nbl::Field* field, nbl::FieldValue value) {
    switch(field->type) {
        case nbl::FieldType::STRING: 
            return QString::fromUtf8(value.stringValue.data, value.stringValue.length);
        case nbl::FieldType::INTEGER:
            return static_cast<qint64>(value.integerValue);
        case nbl::FieldType::FLOAT: 
            return value.floatValue;
        case nbl::FieldType::BOOLEAN: 
            return value.booleanValue;
        case nbl::FieldType::ENUM: 
            return static_cast<quint32>(value.enumValue);
        case nbl::FieldType::DATE: 
            return static_cast<quint64>(value.uintegerValue);
        case nbl::FieldType::ACCOUNT:
            if(value.accountValue.isDefault) return -1;
            return static_cast<quint32>(value.accountValue.id);
        case nbl::FieldType::LIST: {
            QVariantList out;
            for(nbl::FieldValue& value : value.compositeValue) {
                out.append(_fieldToQt(field->listData.templateField, value));
            }
            return out;
        }
        case nbl::FieldType::STRUCT: {
            QVariantMap out;
            for(auto i = 0 ; i < nbl::min(field->structData.fields.length, value.compositeValue.length) ; i++) {
                QString key = QString::fromUtf8(field->structData.fields[i].key.data, field->structData.fields[i].key.length);
                out.insert(key, _fieldToQt(&field->structData.fields[i], value.compositeValue[i]));
            }
            for(auto i = nbl::min(field->structData.fields.length, value.compositeValue.length) ; i < field->structData.fields.length ; i++) {
                QString key = QString::fromUtf8(field->structData.fields[i].key.data, field->structData.fields[i].key.length);
                out.insert(key, _fieldToQt(&field->structData.fields[i], {}));
            }
            return out;
        }
    }
    return QVariant();
}

nbl::FieldValue _qtToField(const nbl::Field* field, QVariant value, nbl::LinearAllocator* allocator) {

    switch(field->type) {
        case nbl::FieldType::STRING: {
            QByteArray utf = value.toString().toUtf8();
            nbl::StringView str;
            NBL_ASSERT(nbl::StringBuilder(allocator).append(nbl::StringView(utf.data(), utf.length())).buildNullTerminated(&str).success());
            return { .stringValue = str };
        }
        case nbl::FieldType::INTEGER: return { .integerValue = value.toLongLong() };
        case nbl::FieldType::FLOAT: return { .floatValue = value.toDouble() };
        case nbl::FieldType::BOOLEAN: return { .booleanValue = value.toBool() };
        case nbl::FieldType::ENUM: return { .enumValue = static_cast<u32>(value.toUInt()) };
        case nbl::FieldType::DATE: return { .uintegerValue = value.toULongLong() };
        case nbl::FieldType::ACCOUNT: return { .accountValue = {
            .id = value.toUInt(),
            .isDefault = value.toInt() == -1
        }};
        case nbl::FieldType::LIST: {
            QVariantList values = value.toList();
            nbl::Span<nbl::FieldValue> fieldValues{};
            if(values.length() > 0) {
                NBL_ASSERT(allocator->allocate(&fieldValues.data, values.length()).success()); 
                fieldValues.length = values.length();
        
                usize current = 0;
                for(QVariant var : values) {
                    fieldValues[current++] = _qtToField(field->listData.templateField, var, allocator);
                }
                fieldValues.length = current;
            }
            return { .compositeValue = fieldValues };
        };
        case nbl::FieldType::STRUCT: {

            QVariantMap values = value.toMap();
            nbl::Span<nbl::FieldValue> fieldValues{};
            fieldValues.length = field->structData.fields.length;
            if(fieldValues.length > 0) {
                NBL_ASSERT(allocator->allocate(&fieldValues.data, fieldValues.length).success());

                usize current = 0;
                for(const nbl::Field& field : field->structData.fields) {
                    
                    QString key = QString::fromUtf8(field.key.data, field.key.length);
                    if(values.contains(key)) {
                        QVariant var = *values.find(key);
                        fieldValues[current++] = _qtToField(&field, var, allocator);
                    } else {
                        fieldValues[current++] = field.defaultValue;
                    }
                }
            }
            return { .compositeValue = fieldValues };          
        };
    }
    return {};
}

}

quint32 NblFieldRef::type() {
    return static_cast<quint32>(_pField->type);
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

QString NblFieldRef::accountProvider() {
    if(_pField->type != nbl::FieldType::ACCOUNT) return {};
    return QString::fromUtf8(_pField->accountData.accountProvider.data, _pField->accountData.accountProvider.length);
}
bool NblFieldRef::showDefaultOption() {
    return _pField->type == nbl::FieldType::ACCOUNT && _pField->accountData.showDefaultOption;
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
    return new NblEnumFieldModel(_pField);
}


// QVariant NblFieldRef::value() {
//     return _mValue;
// }
//
// void NblFieldRef::setValue(QVariant value) {
//     _mValue = value;
//     emit valueChanged();
// }


void NblFieldRef::suggest(QString current, QJSValue value) {
    if(_pField->type != nbl::FieldType::STRING || !value.isCallable()) return;

    QByteArray currentUtf = current.toUtf8();
    _pField->stringData.suggest(nbl::StringView(currentUtf.data(), currentUtf.length()), [&](nbl::StringView suggestion) {
        QJSValueList args{};
        args.append(QString::fromUtf8(suggestion.data, suggestion.length));
        value.call(args);
    });

}

nbl::FieldValue NblFieldRef::toNbl(QVariant variant, nbl::LinearAllocator* allocator) {
    return _qtToField(_pField, variant, allocator);
}
QVariant NblFieldRef::fromNbl(nbl::FieldValue value) {
    return _fieldToQt(_pField, value);
}

const nbl::Field* NblFieldRef::field() {
    return _pField;
}

// nbl::FieldValue NblFieldRef::fieldValue(nbl::LinearAllocator* allocator) {
//     return _qtToField(_pField, _mValue, allocator);
// }
//
// void NblFieldRef::loadValue(nbl::FieldValue value) {
//     _mValue = _fieldToQt(_pField, value);
//     emit valueChanged();
// }
