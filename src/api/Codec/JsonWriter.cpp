#include <Codec/Json.hpp>

#include <cmath>
#include "Platform/Logging.hpp"

nbl::Result nbl::writeJson(Writer writer, JsonElement element) {

    switch(element.type) {
        case(nbl::JsonType::NONE): {
            return writer.writeString("null");
        }
        case(nbl::JsonType::NUMBER): {
            switch(element.number.type) {
                case(nbl::JsonNumberType::SIGNED): {
                    return nbl::writeInt(element.number.intValue, writer);
                }
                case(nbl::JsonNumberType::UNSIGNED): {
                    return nbl::writeUint(element.number.uintValue, writer);
                }
                case(nbl::JsonNumberType::FLOAT): {
                    i32 fpInfo = std::fpclassify(element.number.floatValue);
                    if(fpInfo == FP_INFINITE || fpInfo == FP_NAN) {
                        return writer.writeString("0");
                    }
                    return nbl::writeFloat(element.number.floatValue, writer);
                }
            }
            break;
        }
        case(nbl::JsonType::BOOLEAN): {
            if(element.boolean) {
                return writer.writeString("true");
            } else {
                return writer.writeString("false");
            }
        }
        case(nbl::JsonType::STRING): {
            NBL_CHECK(writer.writeChar('"'));
            NBL_CHECK(writer.writeString(element.string));
            NBL_CHECK(writer.writeChar('"'));
            return nbl::Result::SUCCESS;
        }
        case(nbl::JsonType::ARRAY): {
            NBL_CHECK(writer.writeChar('['));
            usize index = 0;
            for(JsonElement& ele : element.array) {
                if(index++ > 0) {
                    NBL_CHECK(writer.writeChar(','));
                }
                NBL_CHECK(writeJson(writer, ele));
            }
            NBL_CHECK(writer.writeChar(']'));
            return nbl::Result::SUCCESS;
        }
        case(nbl::JsonType::OBJECT): {
            NBL_CHECK(writer.writeChar('{'));
            usize index = 0;
            for(JsonObject::Entry& ent : element.object) {
                if(index++ > 0) {
                    NBL_CHECK(writer.writeChar(','));
                }
                NBL_CHECK(writer.writeChar('"'));
                NBL_CHECK(writer.writeString(ent.key));
                NBL_CHECK(writer.writeString("\":"));
                NBL_CHECK(writeJson(writer, ent.value));
            }
            NBL_CHECK(writer.writeChar('}'));
            return nbl::Result::SUCCESS;
        }
    }

    return nbl::Result::INVALID_ARGUMENT;
}




nbl::JsonWriter::ArrayWriter& nbl::JsonWriter::ArrayWriter::add(JsonElement element) {
    if(_rParent._mLastResult.error()) return *this;

    if(_mNumElements++ > 0) {
        _rParent._mLastResult = _rParent._mSink.writeChar(',');
        if(_rParent._mLastResult.error()) return *this;
    }

    _rParent._mLastResult = writeJson(_rParent._mSink, element);
    return *this;
}

nbl::JsonWriter::ArrayWriter& nbl::JsonWriter::ArrayWriter::writeObject(std::function_ref<void(ObjectWriter& writer)> writer) {

    if(_rParent._mLastResult.error()) return *this;
    
    if(_mNumElements++ > 0) {
        _rParent._mLastResult = _rParent._mSink.writeChar(',');
        if(_rParent._mLastResult.error()) return *this;
    }
    
    _rParent._mLastResult = _rParent._mSink.writeChar('{');
    if(_rParent._mLastResult.error()) return *this;

    auto obj = ObjectWriter(_rParent);
    writer(obj);

    if(_rParent._mLastResult.error()) return *this;
    
    _rParent._mLastResult = _rParent._mSink.writeChar('}');
    return *this;

}

nbl::JsonWriter::ArrayWriter& nbl::JsonWriter::ArrayWriter::writeArray(std::function_ref<void(ArrayWriter& writer)> writer) {
    if(_rParent._mLastResult.error()) return *this;
    
    if(_mNumElements++ > 0) {
        _rParent._mLastResult = _rParent._mSink.writeChar(',');
        if(_rParent._mLastResult.error()) return *this;
    }
    
    _rParent._mLastResult = _rParent._mSink.writeChar('[');
    if(_rParent._mLastResult.error()) return *this;

    auto obj = ArrayWriter(_rParent);
    writer(obj);

    if(_rParent._mLastResult.error()) return *this;
    
    _rParent._mLastResult = _rParent._mSink.writeChar(']');
    return *this;
}

nbl::JsonWriter::ObjectWriter& nbl::JsonWriter::ObjectWriter::add(StringView key, JsonElement element) {
    
    if(_rParent._mLastResult.error()) return *this;
    
    if(_mNumElements++ > 0) {
        _rParent._mLastResult = _rParent._mSink.writeChar(',');
        if(_rParent._mLastResult.error()) return *this;
    }

    _rParent._mLastResult = _rParent._mSink.writeChar('"');
    if(_rParent._mLastResult.error()) return *this;

    _rParent._mLastResult = _rParent._mSink.writeString(key);
    if(_rParent._mLastResult.error()) return *this;

    _rParent._mLastResult = _rParent._mSink.writeString("\":");
    if(_rParent._mLastResult.error()) return *this;

    _rParent._mLastResult = writeJson(_rParent._mSink, element);
    return *this;
}



nbl::JsonWriter::ObjectWriter& nbl::JsonWriter::ObjectWriter::writeObject(StringView key, std::function_ref<void(ObjectWriter& writer)> writer) {

    if(_rParent._mLastResult.error()) return *this;
    
    if(_mNumElements++ > 0) {
        _rParent._mLastResult = _rParent._mSink.writeChar(',');
        if(_rParent._mLastResult.error()) return *this;
    }

    _rParent._mLastResult = _rParent._mSink.writeChar('"');
    if(_rParent._mLastResult.error()) return *this;

    _rParent._mLastResult = _rParent._mSink.writeString(key);
    if(_rParent._mLastResult.error()) return *this;

    _rParent._mLastResult = _rParent._mSink.writeString("\":{");
    if(_rParent._mLastResult.error()) return *this;

    auto obj = ObjectWriter(_rParent);
    writer(obj);

    if(_rParent._mLastResult.error()) return *this;
    
    _rParent._mLastResult = _rParent._mSink.writeChar('}');
    return *this;
}
nbl::JsonWriter::ObjectWriter& nbl::JsonWriter::ObjectWriter::writeArray(StringView key, std::function_ref<void(ArrayWriter& writer)> writer) {
    if(_rParent._mLastResult.error()) return *this;
    
    if(_mNumElements++ > 0) {
        _rParent._mLastResult = _rParent._mSink.writeChar(',');
        if(_rParent._mLastResult.error()) return *this;
    }

    _rParent._mLastResult = _rParent._mSink.writeChar('"');
    if(_rParent._mLastResult.error()) return *this;

    _rParent._mLastResult = _rParent._mSink.writeString(key);
    if(_rParent._mLastResult.error()) return *this;

    _rParent._mLastResult = _rParent._mSink.writeString("\":[");
    if(_rParent._mLastResult.error()) return *this;

    auto obj = ArrayWriter(_rParent);
    writer(obj);

    if(_rParent._mLastResult.error()) return *this;
    
    _rParent._mLastResult = _rParent._mSink.writeChar(']');
    return *this;
}


nbl::Result nbl::JsonWriter::writeElement(nbl::JsonElement element) {
    return nbl::writeJson(_mSink, element);
}

nbl::Result nbl::JsonWriter::writeObject(std::function_ref<void(ObjectWriter& writer)> writer) {

    NBL_CHECK(_mLastResult = _mSink.writeChar('{'));

    auto obj = ObjectWriter(*this);
    writer(obj);

    if(_mLastResult.error()) return _mLastResult;
    NBL_CHECK(_mLastResult = _mSink.writeChar('}'));

    return nbl::Result::SUCCESS;
}
nbl::Result nbl::JsonWriter::writeArray(std::function_ref<void(ArrayWriter& writer)> writer) {
    NBL_CHECK(_mLastResult = _mSink.writeChar('['));

    auto obj = ArrayWriter(*this);
    writer(obj);

    if(_mLastResult.error()) return _mLastResult;
    NBL_CHECK(_mLastResult = _mSink.writeChar(']'));

    return nbl::Result::SUCCESS;
}
