#include <Platform/StringBuilder.hpp>

nbl::Result nbl::StringBuilder::replaceChecked(char oldValue, StringView newValue, usize startIndex) {
        
    if(!_mLastResult.success()) {
        return _mLastResult;
    }
    if(_mLength == 0) {
        _mFrame.releaseUnsafe();
    }

    void* ptr = _mFrame.begin();
    char* stringData = static_cast<char*>(ptr);

    usize stringLength = _mFrame.index();

    usize occurrances;
    for(usize i = startIndex ; i < stringLength; i++) {
        char c = stringData[i];
        if(c == oldValue) {
            occurrances++;
        }
    }

    usize outStringLength = stringLength + (occurrances * (newValue.length - 1));

    if(outStringLength > _mFrame.capacity()) {
        return nbl::error::OUT_OF_MEMORY;
    }

    if(outStringLength > stringLength) {
        usize diff = outStringLength - stringLength;
        NBL_CHECK(_mFrame.push(diff, nullptr));

        for(usize i = startIndex; i < outStringLength; i++) {
            char c = stringData[i];
            if(c == oldValue) {
                memmove(stringData + i + newValue.length, stringData + i + 1, stringLength - i - 1);
                memcpy(stringData + i, newValue.data, newValue.length);
                i += newValue.length - 1;
                stringLength += newValue.length - 1;
            } else {
                stringData[i] = c;
            }
        }

    } else {
        for(usize i = startIndex; i < stringLength; i++) {
            char c = stringData[i];
            if(c == oldValue) {
                stringData[i] = newValue.data[0];
            }
        }
    }

    _mLength = outStringLength;

    return Result::SUCCESS;


}
