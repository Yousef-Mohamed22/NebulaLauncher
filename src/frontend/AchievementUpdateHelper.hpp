#pragma once

#include <qobject.h>
#include <QObject>
#include <QtQml>
#include <QJSEngine>


class AchievementUpdateWorker : public QObject {

    Q_OBJECT
public:
    AchievementUpdateWorker(qint64 gameId,qint64 setId) :
        _mGameId(gameId),
        _mSetId(setId)
    { }

public slots:

    void run();

signals:
    void done(bool);

private:

    qint64 _mGameId;
    qint64 _mSetId;
};

class AchievementUpdateRunner : public QObject {

    Q_OBJECT
    QML_ELEMENT

public:

    AchievementUpdateRunner(QObject* parent = nullptr) :
        QObject(parent),
        _pActiveThread(nullptr)
    { }

    ~AchievementUpdateRunner() {
        if(_pActiveThread) {
            _pActiveThread->quit();
            _pActiveThread->wait();
            _pActiveThread = nullptr;
        }
    }

    Q_INVOKABLE void update(qint64 gameId, qint64 setId=0) {

        if(_pActiveThread) {
            _pActiveThread->quit();
            _pActiveThread->wait();
            _pActiveThread = nullptr;
        }

        _pActiveThread = new QThread(this);
        AchievementUpdateWorker* worker = new AchievementUpdateWorker(gameId,setId);

        worker->moveToThread(_pActiveThread);

        connect(_pActiveThread, &QThread::started, worker, &AchievementUpdateWorker::run);
        connect(worker, &AchievementUpdateWorker::done, this, &AchievementUpdateRunner::updateComplete);

        connect(worker, &AchievementUpdateWorker::done, _pActiveThread, &QThread::quit);
        connect(worker, &AchievementUpdateWorker::done, worker, &QObject::deleteLater);
        connect(worker, &AchievementUpdateWorker::done, this, [this]() {
            _pActiveThread = nullptr;
        });
        connect(_pActiveThread, &QThread::finished, _pActiveThread, &QObject::deleteLater);

        _pActiveThread->start();
    }

signals:

    void updateComplete(bool shouldReload);

private:
    QThread* _pActiveThread;

};
