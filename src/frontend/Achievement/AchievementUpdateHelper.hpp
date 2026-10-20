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

namespace nbl::frontend {

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

    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningUpdated)

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
            emit isRunningUpdated();
        }

        _pActiveThread = new QThread(this);
        emit isRunningUpdated();
        AchievementUpdateWorker* worker = new AchievementUpdateWorker(gameId,setId);

        worker->moveToThread(_pActiveThread);

        connect(_pActiveThread, &QThread::started, worker, &AchievementUpdateWorker::run);
        connect(worker, &AchievementUpdateWorker::done, this, &AchievementUpdateRunner::updateComplete);

        connect(worker, &AchievementUpdateWorker::done, _pActiveThread, &QThread::quit);
        connect(worker, &AchievementUpdateWorker::done, worker, &QObject::deleteLater);
        connect(worker, &AchievementUpdateWorker::done, this, [this]() {
            _pActiveThread = nullptr;
            emit isRunningUpdated();
        });
        connect(_pActiveThread, &QThread::finished, _pActiveThread, &QObject::deleteLater);

        _pActiveThread->start();
    }

    bool isRunning() {
        return _pActiveThread != nullptr;
    }

signals:

    void isRunningUpdated();
    void updateComplete(bool shouldReload);

private:
    QThread* _pActiveThread;

};

}
