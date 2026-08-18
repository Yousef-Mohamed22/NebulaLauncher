#pragma once

#include <QObject>
#include <QtQml>

#include <Plugin/Field.hpp>

#include "NblFieldRef.hpp"

class NblFieldList : public QObject {

    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Obtain from ApplicationContext")

    Q_PROPERTY(qsizetype count READ count CONSTANT)

public:

    NblFieldList(nbl::Span<nbl::Field> fields, QObject* parent = nullptr);

    quint32 count() const;

    Q_INVOKABLE NblFieldRef* itemAt(qsizetype index);

private:

    nbl::Span<nbl::Field> _mFields;

};
