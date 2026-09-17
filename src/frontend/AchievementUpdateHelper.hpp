#pragma once

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

    Q_INVOKABLE void update(qint64 gameId, qint64 setId=0) {

        QThread* thread = new QThread(this);
        AchievementUpdateWorker* worker = new AchievementUpdateWorker(gameId,setId);

        worker->moveToThread(thread);

        connect(thread, &QThread::started, worker, &AchievementUpdateWorker::run);
        connect(worker, &AchievementUpdateWorker::done, this, &AchievementUpdateRunner::updateComplete);

        connect(worker, &AchievementUpdateWorker::done, thread, &QThread::quit);
        connect(worker, &AchievementUpdateWorker::done, worker, &QObject::deleteLater);
        connect(thread, &QThread::finished, thread, &QObject::deleteLater);

        thread->start();
    }

signals:

    void updateComplete(bool shouldReload);

};
