#pragma once

#include <QObject>
#include <QtQml>

class NblGameRef : public QObject {

    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(quint64 gameId READ gameId CONSTANT)
    Q_PROPERTY(

};
