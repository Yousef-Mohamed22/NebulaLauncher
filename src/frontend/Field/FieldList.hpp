#pragma once

#include <QObject>
#include <QtQml>

#include <Plugin/Field.hpp>

#include "FieldRef.hpp"

namespace nbl::frontend {

class FieldList : public QObject {

    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Obtain from ApplicationContext")

    Q_PROPERTY(qsizetype count READ count CONSTANT)
    Q_PROPERTY(qsizetype visibleCount READ visibleCount CONSTANT)

public:

    FieldList(nbl::Span<nbl::Field> fields, QObject* parent = nullptr);

    quint32 count() const;
    quint32 visibleCount() const;

    Q_INVOKABLE FieldRef* itemAt(qsizetype index);
    Q_INVOKABLE FieldRef* visibleItemAt(qsizetype index);

private:

    nbl::Span<nbl::Field> _mFields;

};

}
