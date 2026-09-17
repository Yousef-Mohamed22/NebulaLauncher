#include <Core/Result.hpp>
#include <Core/String.hpp>
#include <functional>

namespace nbl::ra {

Result parseTitle(StringView title, std::function_ref<void(StringView)> tagConsumer, StringView* gameTitle, StringView* subsetName);

Result parseDate(StringView date, u64* outTime);

}
