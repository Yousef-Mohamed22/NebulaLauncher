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
#include "Field/FieldRef.hpp"
#include "Field/FieldList.hpp"
#include "Field/EnumFieldModel.hpp"
#include "ApplicationContext.hpp"

#include <Core/String.hpp>
#include <Core/Math.hpp>
#include <Plugin/Field.hpp>
#include <Plugin/PluginContext.hpp>
#include <Platform/Allocator.hpp>
#include <Utility/StringBuilder.hpp>

namespace {

QVariant _fieldToQt(const nbl::Field* field, nbl::FieldValue value) {
    switch(field->type) {
        case nbl::FieldType::STRING: {
            QVariantMap out = {};
            out["resolved"] = QString::fromUtf8(value.stringValue.resolved.data(), value.stringValue.resolved.length());
            if(!value.stringValue.raw.empty()) {
                out["raw"] = QString::fromUtf8(value.stringValue.raw.data(), value.stringValue.raw.length());
            } else {
                out["raw"] = QString::fromUtf8(value.stringValue.resolved.data(), value.stringValue.resolved.length());
            }
            return out;
        }
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
        case nbl::FieldType::COLOR:
            return static_cast<quint32>(value.colorValue.rgba);
        case nbl::FieldType::LIST: {
            QVariantList out;
            for(nbl::FieldValue& value : value.compositeValue) {
                out.append(_fieldToQt(field->listData.templateField, value));
            }
            return out;
        }
        case nbl::FieldType::STRUCT: {
            QVariantMap out;
            for(auto i = 0 ; i < nbl::min(field->structData.fields.length(), value.compositeValue.length()) ; i++) {
                QString key = QString::fromUtf8(field->structData.fields[i].key.data(), field->structData.fields[i].key.length());
                out.insert(key, _fieldToQt(&field->structData.fields[i], value.compositeValue[i]));
            }
            for(auto i = nbl::min(field->structData.fields.length(), value.compositeValue.length()) ; i < field->structData.fields.length() ; i++) {
                QString key = QString::fromUtf8(field->structData.fields[i].key.data(), field->structData.fields[i].key.length());
                out.insert(key, _fieldToQt(&field->structData.fields[i], {}));
            }
            return out;
        }
        case nbl::FieldType::ACTION:
        case nbl::FieldType::FIELD_ACTION:
            return QVariant();
    }
    return QVariant();
}

nbl::FieldValue _qtToField(const nbl::Field* field, QVariant value, nbl::LinearAllocator* allocator) {

    switch(field->type) {
        case nbl::FieldType::STRING: {

            nbl::StringView strRaw{};
            nbl::StringView strResolved{};
            if(value.userType() == QMetaType::QString) {

                QByteArray utf = value.toString().toUtf8();
                NBL_ASSERT(nbl::StringBuilder(allocator).append(nbl::StringView(utf.data(), utf.length())).buildNullTerminated(&strResolved).success());
                strRaw = strResolved;
            } else {
                QVariantMap values = value.toMap();

                QByteArray utfRaw = values["raw"].toString().toUtf8();
                QByteArray utfResolved = values["resolved"].toString().toUtf8();
                if(utfRaw.length() > 0) {
                    NBL_ASSERT(nbl::StringBuilder(allocator).append(nbl::StringView(utfRaw.data(), utfRaw.length())).buildNullTerminated(&strRaw).success());
                }
                NBL_ASSERT(nbl::StringBuilder(allocator).append(nbl::StringView(utfResolved.data(), utfResolved.length())).buildNullTerminated(&strResolved).success());
            }
            return { .stringValue = { .resolved = strResolved, .raw = strRaw } };
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
        case nbl::FieldType::COLOR: return { .colorValue = {
            .rgba = value.toUInt()
        }};
        case nbl::FieldType::LIST: {
            QVariantList qtValues = value.toList();
            nbl::FieldValue* values{};
            nbl::Span<nbl::FieldValue> valueSpan{};
            if(qtValues.length() > 0) {
                NBL_ASSERT(allocator->allocate(&values, qtValues.length()).success()); 
        
                usize current = 0;
                for(QVariant var : qtValues) {
                    values[current++] = _qtToField(field->listData.templateField, var, allocator);
                }
                valueSpan = nbl::Span<nbl::FieldValue>(values, current);
            }
            return { .compositeValue = valueSpan };
        };
        case nbl::FieldType::STRUCT: {

            QVariantMap qtValues = value.toMap();
            nbl::FieldValue* values{};
            nbl::Span<nbl::FieldValue> valueSpan{};
            if(field->structData.fields.length() > 0) {
                NBL_ASSERT(allocator->allocate(&values, field->structData.fields.length()).success());

                usize current = 0;
                for(const nbl::Field& field : field->structData.fields) {
                    
                    QString key = QString::fromUtf8(field.key.data(), field.key.length());
                    if(qtValues.contains(key)) {
                        QVariant var = *qtValues.find(key);
                        values[current++] = _qtToField(&field, var, allocator);
                    } else {
                        values[current++] = field.defaultValue;
                    }
                }

                valueSpan = nbl::Span<nbl::FieldValue>(values, current);
            }
            return { .compositeValue = valueSpan };          
        };
        case nbl::FieldType::ACTION:
        case nbl::FieldType::FIELD_ACTION:
            return {};
    }
    return {};
}

}

quint32 nbl::frontend::FieldRef::type() {
    return static_cast<quint32>(_pField->type);
}

QString nbl::frontend::FieldRef::key() {
    return QString::fromUtf8(_pField->key.data(), _pField->key.length());
}

QString nbl::frontend::FieldRef::name() {
    return QString::fromUtf8(_pField->name.data(), _pField->name.length());
}

QVariant nbl::frontend::FieldRef::min() {
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

QVariant nbl::frontend::FieldRef::max() {
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

QVariant nbl::frontend::FieldRef::defaultValue() {
    return _fieldToQt(_pField, _pField->defaultValue);
}

QString nbl::frontend::FieldRef::accountProvider() {
    if(_pField->type != nbl::FieldType::ACCOUNT) return {};
    return QString::fromUtf8(_pField->accountData.accountProvider.data(), _pField->accountData.accountProvider.length());
}
bool nbl::frontend::FieldRef::showDefaultOption() {
    return _pField->type == nbl::FieldType::ACCOUNT && (_pField->flags & nbl::FieldFlag::SHOW_DEFAULT_OPTION);
}

nbl::frontend::FieldRef* nbl::frontend::FieldRef::fieldTemplate() {
    if(_pField->type == nbl::FieldType::LIST) return new FieldRef(_pField->listData.templateField, this);
    if(_pField->type == nbl::FieldType::FIELD_ACTION) return new FieldRef(_pField->fieldActionData.templateField, this);
    return nullptr;
}

nbl::frontend::FieldList* nbl::frontend::FieldRef::structFields() {
    if(_pField->type != nbl::FieldType::STRUCT) return nullptr;
    return new FieldList(_pField->structData.fields, this);

}

bool nbl::frontend::FieldRef::horizontal() {
    if(_pField->type != nbl::FieldType::STRUCT) return false;
    return _pField->flags & nbl::FieldFlag::HORIZONTAL; 
}

bool nbl::frontend::FieldRef::password() {
    if(_pField->type != nbl::FieldType::STRING) return false;
    return _pField->flags & nbl::FieldFlag::PASSWORD; 
}




nbl::frontend::EnumFieldModel* nbl::frontend::FieldRef::enumModel() {
    return new EnumFieldModel(_pField);
}


void nbl::frontend::FieldRef::suggest(QString current, QJSValue value) {
    if(_pField->type != nbl::FieldType::STRING || !value.isCallable()) return;

    nbl::PluginContext* context = ApplicationContext::instance()->pluginContext();

    QByteArray currentUtf = current.toUtf8();
    _pField->stringData.suggest(context, nbl::StringView(currentUtf.data(), currentUtf.length()), [&](nbl::StringView suggestion) {
        QJSValueList args{};
        args.append(QString::fromUtf8(suggestion.data(), suggestion.length()));
        value.call(args);
    });
}

void nbl::frontend::FieldRef::action() {
    nbl::PluginContext* context = ApplicationContext::instance()->pluginContext();
    _pField->actionData.activate(context);
}

void nbl::frontend::FieldRef::fieldAction(QVariant variant) {
    nbl::PluginContext* context = ApplicationContext::instance()->pluginContext();
    nbl::AllocationFrame frame(nbl::Arena::threadLocal());
    _pField->fieldActionData.activate(context, toNbl(variant, &frame));
}

nbl::FieldValue nbl::frontend::FieldRef::toNbl(QVariant variant, nbl::LinearAllocator* allocator) {
    return _qtToField(_pField, variant, allocator);
}

QVariant nbl::frontend::FieldRef::fromNbl(nbl::FieldValue value) {
    return _fieldToQt(_pField, value);
}

const nbl::Field* nbl::frontend::FieldRef::field() {
    return _pField;
}

QVariant nbl::frontend::FieldRef::parse(QByteArray encodedFields, nbl::Span<nbl::Field> fields) {

    nbl::Field dummy {
        .type = nbl::FieldType::STRUCT,
        .structData = { .fields = fields }
    };

    QVariant qtFields = QVariantMap();
    nbl::AllocationFrame frame(nbl::Arena::threadLocal());
    nbl::FieldValue* values;
    if(frame.allocate(&values, fields.length()).error()) return QVariant();

    nbl::JsonElement element;
    if(nbl::parseJson(nbl::StringView(encodedFields.data(), encodedFields.length()), &frame, &element).error()) return QVariant();
    if(element.type() != nbl::JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;

    nbl::Span<nbl::FieldValue> valueSpan(values, fields.length());
    if(nbl::decodeFieldValues(ApplicationContext::instance()->pluginContext(), fields, element.asObject(), &valueSpan, &frame).error()) return QVariant();

    qtFields = nbl::frontend::FieldRef(&dummy).fromNbl({ .compositeValue = valueSpan });

    return qtFields;

}
