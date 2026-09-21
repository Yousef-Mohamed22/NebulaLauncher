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
    Q_PROPERTY(NblFieldRef* listTemplate READ listTemplate CONSTANT)
    Q_PROPERTY(NblFieldList* structFields READ structFields CONSTANT)
    Q_PROPERTY(bool horizontal READ horizontal CONSTANT)

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
    NblFieldRef* listTemplate();
    NblFieldList* structFields();
    bool horizontal();

    Q_INVOKABLE void suggest(QString current, QJSValue callback);

    nbl::FieldValue toNbl(QVariant variant, nbl::LinearAllocator* allocator);
    QVariant fromNbl(nbl::FieldValue value);

    const nbl::Field* field();

private:

    const nbl::Field* _pField;

};
