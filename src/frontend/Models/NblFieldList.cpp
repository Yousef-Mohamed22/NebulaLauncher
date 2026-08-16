#include "NblFieldList.hpp"

quint32 NblFieldList::count() const { return _mFields.length; }

NblFieldRef* NblFieldList::itemAt(qsizetype index) const {

    if(index >= _mFields.length) return nullptr;
    return new NblFieldRef(&_mFields[index]);

}

