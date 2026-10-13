#pragma once 

#include <QObject>
#include <QColor>

namespace nbl::frontend {

struct TagRef {
    qint64 id;
    QString name;
    QColor color;
};

}
