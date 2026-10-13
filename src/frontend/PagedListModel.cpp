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
