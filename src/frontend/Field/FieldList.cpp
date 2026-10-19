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
#include "Field/FieldList.hpp"

#include <Plugin/Field.hpp>

nbl::frontend::FieldList::FieldList(nbl::Span<nbl::Field> fields, QObject* parent) :
    QObject(parent),
    _mFields(fields)
{ }

quint32 nbl::frontend::FieldList::count() const { return _mFields.length; }

quint32 nbl::frontend::FieldList::visibleCount() const {
    quint32 out = 0;
    for(const nbl::Field& f : _mFields) {
        if(f.flags & nbl::FieldFlag::HIDDEN) continue;
        out++;
    }
    return out;
}

nbl::frontend::FieldRef* nbl::frontend::FieldList::itemAt(qsizetype index) {

    if(index >= _mFields.length) return nullptr;
    return new FieldRef(&_mFields[index]);

}

nbl::frontend::FieldRef* nbl::frontend::FieldList::visibleItemAt(qsizetype index) {
    quint32 out = 0;
    for(const nbl::Field& f : _mFields) {
        if(out == index) return new FieldRef(&f);
        if(f.flags & nbl::FieldFlag::HIDDEN) continue;
        out++;
    }
    return nullptr;
}

