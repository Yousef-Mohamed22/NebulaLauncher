#include <Core/String.hpp>
#include <Plugin/ProcessTracker.hpp>
#include <Plugin/PluginContext.hpp>
#include <Platform/Assert.hpp>


void nbl::ProcessTracker::pushStatus(nbl::StringView status) {
    if(_mState != State::STARTING) return;
    _fStatusConsumer(_mInstallationId, status, {});
}

void nbl::ProcessTracker::pushStatus(nbl::StringView status, f64 progress) {
    if(_mState != State::STARTING) return;
    _fStatusConsumer(_mInstallationId, status, progress);
}

void nbl::ProcessTracker::pushProgress(f64 progress) {
    if(_mState != State::STARTING) return;
    _fStatusConsumer(_mInstallationId, {}, progress);
}


nbl::Result nbl::ProcessTracker::run(PluginContext* context, StringView command, StringView workingDir) {
    NBL_ASSERT(_mState == State::STARTING);

    nbl::Result res = context->runCommand(_mInstallationId, command, workingDir, &_mHandle);
    if(res.error()) {
        _mState = State::FAILED;
        return res;
    }

    _mState = State::RUNNING;
    return nbl::Result::SUCCESS;
}

void nbl::ProcessTracker::check() {
    if(_mState == State::RUNNING && !_mHandle.isRunning()) {
        _mState = State::COMPLETE;
    }
}

void nbl::ProcessTracker::terminate() {
    if(_mState != State::RUNNING) return;
    _mHandle.terminate();
}

void nbl::ProcessTracker::kill() {
    if(_mState != State::RUNNING) return;
    _mHandle.kill();
    _mState = State::KILLED;
}

nbl::Result nbl::ProcessTracker::create(i64 installationId, void(*statusConsumer)(i64, Optional<StringView>, Optional<f64>), ProcessTracker* outTracker) {
    if(outTracker->_mState != State::NOT_STARTED) return nbl::Result::INVALID_ARGUMENT;
    outTracker->_mInstallationId = installationId;
    outTracker->_mState = State::STARTING;
    outTracker->_fStatusConsumer = statusConsumer;
    return nbl::Result::SUCCESS;
}
