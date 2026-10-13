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

