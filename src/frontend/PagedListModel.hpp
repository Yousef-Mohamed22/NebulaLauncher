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

#include <Core/Types.hpp>
#include <Core/Math.hpp>
#include <Core/Result.hpp>

#include <QObject>
#include <QtQml>

class PagedListModel : public QAbstractListModel {

    Q_OBJECT

    Q_PROPERTY(int pageSize READ pageSize WRITE setPageSize NOTIFY pageSizeChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)
    Q_PROPERTY(bool allLoaded READ allLoaded NOTIFY allLoadedChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)

public:

    explicit PagedListModel(QObject* parent = nullptr) :
        QAbstractListModel(parent),
        _mPageSize(10),
        _bIsLoading(false),
        _bAllLoaded(false),
        _bPaused(false),
        _bPageRequested(false)
    { }

    quint64 pageSize() const { return _mPageSize; }
    bool isLoading() const { return _bIsLoading; }
    bool allLoaded() const { return _bAllLoaded; }

    void setPageSize(int size) {
        const quint64 newSize = static_cast<quint64>(nbl::max(1, size));
        if(_mPageSize == newSize) return;

        _mPageSize = newSize;
        emit pageSizeChanged();
    }

    int count() {
        return rowCount();
    }

    bool paused() {
        return _bPaused;
    }

    void setPaused(bool paused) {
        if(_bPaused == paused) return;

        _bPaused = paused;
        emit pausedChanged();

        if(!paused && _bPageRequested) {
            loadNextPage();
        }
    }

    Q_INVOKABLE void loadNextPage();
    Q_INVOKABLE void refresh();

signals:

    void pageSizeChanged();
    void isLoadingChanged();
    void allLoadedChanged();
    void countChanged();
    void pausedChanged();

protected:

    virtual void _clear() = 0;
    virtual nbl::Result _fetchPageData(qsizetype limit, qsizetype offset) = 0;

private:

    virtual void _fetchMore();

    quint64 _mPageSize;
    bool _bIsLoading;
    bool _bAllLoaded;
    bool _bPaused;
    bool _bPageRequested;
};
