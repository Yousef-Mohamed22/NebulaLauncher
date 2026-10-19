#pragma once

#include <QObject>
#include <QtQml>

#include "Field/EnumFieldModel.hpp"

#include <Platform/Allocator.hpp>
#include <Plugin/Field.hpp>

namespace nbl::frontend {

class FieldList;

class FieldRef : public QObject {

    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Obtain from FieldList")

    Q_PROPERTY(quint32 type READ type CONSTANT)
    Q_PROPERTY(QString key READ key CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QVariant min READ min CONSTANT)
    Q_PROPERTY(QVariant max READ max CONSTANT)
    Q_PROPERTY(QVariant defaultValue READ defaultValue CONSTANT)
    Q_PROPERTY(EnumFieldModel* enumModel READ enumModel CONSTANT)
    Q_PROPERTY(QString accountProvider READ accountProvider CONSTANT)
    Q_PROPERTY(bool showDefaultOption READ showDefaultOption CONSTANT)
    Q_PROPERTY(FieldRef* fieldTemplate READ fieldTemplate CONSTANT)
    Q_PROPERTY(FieldList* structFields READ structFields CONSTANT)
    Q_PROPERTY(bool horizontal READ horizontal CONSTANT)
    Q_PROPERTY(bool password READ password CONSTANT)

public:
    
    FieldRef(const nbl::Field* field, QObject* parent = nullptr) :
        QObject(parent),
        _pField(field)
    { }

    quint32 type();
    QString key();
    QString name();
    QVariant min();
    QVariant max();
    QVariant defaultValue();
    EnumFieldModel* enumModel();
    QString accountProvider();
    bool showDefaultOption();
    FieldRef* fieldTemplate();
    FieldList* structFields();
    bool horizontal();
    bool password();

    Q_INVOKABLE void suggest(QString current, QJSValue callback);

    Q_INVOKABLE void action();
    Q_INVOKABLE void fieldAction(QVariant variant);

    nbl::FieldValue toNbl(QVariant variant, nbl::LinearAllocator* allocator);
    QVariant fromNbl(nbl::FieldValue value);

    const nbl::Field* field();

    static QVariant parse(QByteArray array, nbl::Span<nbl::Field> fields);

private:

    const nbl::Field* _pField;

};

}
