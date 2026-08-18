#include "NblFieldList.hpp"

NblFieldList::NblFieldList(nbl::Span<nbl::Field> fields, QObject* parent) :
    QObject(parent),
    _mFields(fields)
{ }

quint32 NblFieldList::count() const { return _mFields.length; }

NblFieldRef* NblFieldList::itemAt(qsizetype index) {

    if(index >= _mFields.length) return nullptr;
    return new NblFieldRef(&_mFields[index]);

}

