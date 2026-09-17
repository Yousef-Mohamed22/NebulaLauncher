#include <Plugin/PluginInitContext.hpp>
#include <Plugin/InstallationType.hpp>
#include <Plugin/PluginContext.hpp>
#include <Plugin/AccountProvider.hpp>
#include <Plugin/MetadataProvider.hpp>
#include <Platform/Allocator.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>
#include <Codec/Json.hpp>
#include <Plugin/Field.hpp>

bool nbl::PluginInitContext::_verifyId(nbl::StringView id) {
    for(char c : id) {
        if((c < 'a' || c > 'z') && (c < '0' || c > '9') && c != '_') {
            return false;
        }
    }
    return true;
}

nbl::Result nbl::PluginInitContext::create(Registry::Builder* builder, LoadedPlugin* plugin, PluginContext* context, PluginInitContext* outCtx) {

    outCtx->_pBuilder = builder;
    outCtx->_pPlugin = plugin;
    outCtx->_pContext = context;

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::PluginInitContext::getStoredFieldsObject(JsonObject* outValues, StringView* outVersion, LinearAllocator* allocator) {
    return nbl::getPluginFields(_pContext->databaseConnection(), getMeta()->id, [&](const PluginFields& fields) -> nbl::Result {
        JsonElement e;
        NBL_CHECK(nbl::parseJson(StringReader(fields.fieldsJson), allocator, &e));
        if(e.type != JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;
        *outValues = e.object;
        NBL_CHECK(nbl::StringBuilder(allocator).append(fields.version).buildNullTerminated(outVersion));
        return nbl::Result::SUCCESS;
    });
}

nbl::Result nbl::PluginInitContext::getStoredFieldsVersion(StringView* outVersion, LinearAllocator* allocator) {
    return nbl::getPluginFieldsVersion(_pContext->databaseConnection(), getMeta()->id, [&](const PluginFields& fields) -> nbl::Result {
        NBL_CHECK(nbl::StringBuilder(allocator).append(fields.version).buildNullTerminated(outVersion));
        return nbl::Result::SUCCESS;
    });
}

nbl::Result nbl::PluginInitContext::getStoredFieldValues(Span<Field> fields, Span<FieldValue>* outValues, LinearAllocator* allocator) {

    JsonObject obj;
    StringView ver;
    NBL_CHECK(getStoredFieldsObject(&obj, &ver, allocator));

    return decodeFieldValues(_pContext, fields, obj, outValues, allocator);
}

nbl::Result nbl::PluginInitContext::storeFieldValues(Span<Field> fields, Span<FieldValue> values) {

    ArenaFrame frame(Arena::threadLocal());
    NBL_CHECK(encodeFieldValues(_pContext, fields, values, frame.writer()));

    StringView fieldsJson = StringView(static_cast<char*>(frame.begin()), frame.index());
    return nbl::setPluginFields(_pContext->databaseConnection(), getMeta()->id, fieldsJson, getMeta()->version);
}

nbl::Result nbl::PluginInitContext::setFields(Span<Field> fields, void(*pfnUpdateCallback)(Span<Field> fields, Span<FieldValue> newValues, void* userData), void* userData) {

    _pPlugin->fields = fields;
    _pPlugin->pfnUpdateCallback = pfnUpdateCallback;
    _pPlugin->userData = userData;

    return nbl::Result::SUCCESS;
}
