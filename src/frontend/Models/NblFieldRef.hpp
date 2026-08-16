#pragma once

#include <qobject.h>
#include <QObject>
#include <QtQml>

#include "NblEnumFieldModel.hpp"

#include <Plugin/Field.hpp>

class NblFieldList;

class NblFieldRef : public QObject {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(quint32 type READ type CONSTANT)
    Q_PROPERTY(QString key READ key CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QVariant min READ min CONSTANT)
    Q_PROPERTY(QVariant max READ max CONSTANT)
    Q_PROPERTY(QVariant defaultValue READ defaultValue CONSTANT)
    Q_PROPERTY(NblFieldRef* listTemplate READ listTemplate CONSTANT)
    Q_PROPERTY(NblFieldList* structFields READ structFields CONSTANT)

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
    NblFieldRef* listTemplate();
    NblFieldList* structFields();

    Q_INVOKABLE NblEnumFieldModel* enumModel();
    Q_INVOKABLE void suggest(QString current, QJSValue callback);

private:

    const nbl::Field* _pField;

};
