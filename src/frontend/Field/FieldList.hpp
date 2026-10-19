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
