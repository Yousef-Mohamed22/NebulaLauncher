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


#include "PagedListModel.hpp"

void PagedListModel::loadNextPage() {
    if(!_bAllLoaded && !_bIsLoading) {
        _fetchMore();
    }
}

void PagedListModel::refresh() {

    beginResetModel();

    _bPageRequested = false;
    if(_bAllLoaded) {
        _bAllLoaded = false;
        emit allLoadedChanged();
    }
    if(rowCount() > 0) {
        _clear();
        emit countChanged();
    }
    _fetchMore();

    endResetModel();

}

void PagedListModel::_fetchMore() {

    if(_bAllLoaded || _bIsLoading) return;

    if(_bPaused) {
        _bPageRequested = true;
        return;
    }
    _bPageRequested = false;

    _bIsLoading = true;
    emit isLoadingChanged();   

    usize preCount = rowCount();

    if(_fetchPageData(_mPageSize, preCount).error()) {
        _bAllLoaded = true;
        emit allLoadedChanged();
        _bIsLoading = false;
        emit isLoadingChanged();   
        return;
    }

    usize added = rowCount() - preCount;
    if(added < _mPageSize) {
        _bAllLoaded = true;
        emit allLoadedChanged();
    }
    if(added > 0) {
        emit countChanged();
    }

    _bIsLoading = false;
    emit isLoadingChanged();   
}
