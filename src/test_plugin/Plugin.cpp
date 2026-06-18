#include <Plugin/PluginInitializer.hpp>
#include <Platform/Logging.hpp>

extern "C" i32 initPlugin(nbl::PluginContext *context) {

    nbl::log(nbl::LogLevel::INFO, "Hello from plugin");

    return 0;
}
