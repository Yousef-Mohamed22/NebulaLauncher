#pragma once

#include <qobject.h>
#include <qvariant.h>
#include <QObject>
#include <QtQml>

#include "NblEnumFieldModel.hpp"

#include <Platform/Allocator.hpp>
#include <Plugin/Field.hpp>

class NblFieldList;

class NblFieldRef : public QObject {

    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Obtain from NblFieldList")

    Q_PROPERTY(quint32 type READ type CONSTANT)
    Q_PROPERTY(QString key READ key CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QVariant min READ min CONSTANT)
    Q_PROPERTY(QVariant max READ max CONSTANT)
    Q_PROPERTY(QVariant defaultValue READ defaultValue CONSTANT)
    Q_PROPERTY(NblEnumFieldModel* enumModel READ enumModel CONSTANT)
    Q_PROPERTY(QString accountProvider READ accountProvider CONSTANT)
    Q_PROPERTY(bool showDefaultOption READ showDefaultOption CONSTANT)
    Q_PROPERTY(NblFieldRef* fieldTemplate READ fieldTemplate CONSTANT)
    Q_PROPERTY(NblFieldList* structFields READ structFields CONSTANT)
    Q_PROPERTY(bool horizontal READ horizontal CONSTANT)
    Q_PROPERTY(bool password READ password CONSTANT)
    Q_PROPERTY(QString document READ document CONSTANT)

public:
    
    NblFieldRef(const nbl::Field* field, QObject* parent = nullptr) :
        QObject(parent),
        _pField(field)
    { }

    quint32 type();
    QString key();
    QString name();
    QVariant min();
    QVariant max();
    QVariant defaultValue();
    NblEnumFieldModel* enumModel();
    QString accountProvider();
    bool showDefaultOption();
    NblFieldRef* fieldTemplate();
    NblFieldList* structFields();
    bool horizontal();
    bool password();
    QString document();

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
