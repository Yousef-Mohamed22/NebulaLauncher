#include "NblPagedListModel.hpp"

void NblPagedListModel::loadNextPage() {
    if(!_mAllLoaded && !_mIsLoading) {
        _fetchMore();
    }
}

void NblPagedListModel::refresh() {

    beginResetModel();

    if(_mAllLoaded) {
        _mAllLoaded = false;
        emit allLoadedChanged();
    }
    if(rowCount() > 0) {
        _clear();
        emit countChanged();
    }
    _fetchMore();

    endResetModel();

}

void NblPagedListModel::_fetchMore() {
 
    if(_mAllLoaded || _mIsLoading) return;
    _mIsLoading = true;
    emit isLoadingChanged();   

    usize preCount = rowCount();

    if(_fetchPageData(_mPageSize, preCount).error()) {
        _mAllLoaded = true;
        emit allLoadedChanged();
        _mIsLoading = false;
        emit isLoadingChanged();   
        return;
    }

    usize added = rowCount() - preCount;
    if(added < _mPageSize) {
        _mAllLoaded = true;
        emit allLoadedChanged();
    }
    if(added > 0) {
        emit countChanged();
    }

    _mIsLoading = false;
    emit isLoadingChanged();   
}
