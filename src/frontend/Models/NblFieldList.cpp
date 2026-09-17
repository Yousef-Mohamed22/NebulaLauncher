#include "NblFieldList.hpp"
#include "Plugin/Field.hpp"

NblFieldList::NblFieldList(nbl::Span<nbl::Field> fields, QObject* parent) :
    QObject(parent),
    _mFields(fields)
{ }

quint32 NblFieldList::count() const { return _mFields.length; }

quint32 NblFieldList::visibleCount() const {
    quint32 out = 0;
    for(const nbl::Field& f : _mFields) {
        if(f.flags & nbl::FieldFlag::HIDDEN) continue;
        out++;
    }
    return out;
}

NblFieldRef* NblFieldList::itemAt(qsizetype index) {

    if(index >= _mFields.length) return nullptr;
    return new NblFieldRef(&_mFields[index]);

}

NblFieldRef* NblFieldList::visibleItemAt(qsizetype index) {
    quint32 out = 0;
    for(const nbl::Field& f : _mFields) {
        if(out == index) return new NblFieldRef(&f);
        if(f.flags & nbl::FieldFlag::HIDDEN) continue;
        out++;
    }
    return nullptr;
}

