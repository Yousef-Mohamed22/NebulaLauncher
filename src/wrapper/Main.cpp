#include <Core/Result.hpp>
#include <Platform/Logging.hpp>
#include <Platform/Process.hpp>
#include <Platform/Time.hpp>
#include <Codec/Json.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>

int main(int argc, char** argv) {

    if(argc != 4) {
        nbl::log(nbl::LogLevel::ERROR, "Usage: ", nbl::StringView(argv[0]), " [iId] [/path/to/nebula.db] [command]");
        return 1;
    }

    i64 iId = 0;
    nbl::JsonNumber num = {};
    nbl::Result res = nbl::parseJsonNumber(nbl::StringReader(argv[1]), &num);

    if(res.error() || num.type == nbl::JsonNumberType::FLOAT) {
        nbl::log(nbl::LogLevel::ERROR, "Unable to parse installation ID!");
        return 1;
    }
    iId = num.asInt();

    {
        nbl::DatabaseConnection conn;
        res = nbl::DatabaseConnection::open(argv[2], &conn);
        if(res.error()) {
            nbl::log(nbl::LogLevel::WARN, "Unable to open the database!");
        } else {
            nbl::markInstallationJustPlayed(&conn, iId);
        }
    }

    u64 startTime = nbl::getCurrentTime();
    nbl::ProcessHandle proc;
    res = nbl::ProcessHandle::run(argv[3], &proc, {
        .outputMode = nbl::ProcessOutputMode::INHERIT,
        .outputRedirectFile = {},
        .killOnParentExit = true
    });
    if(res.error()) {
        nbl::log(nbl::LogLevel::ERROR, "Unable to start the program!");
        return 1;
    }

    i32 exitCode = 0;
    proc.wait(&exitCode);

    if(exitCode != 0) {
        nbl::log(nbl::LogLevel::WARN, "Process exited with code ", exitCode);
    }

    u64 endTime = nbl::getCurrentTime();

    {
        nbl::DatabaseConnection conn;
        res = nbl::DatabaseConnection::open(argv[2], &conn);
        if(res.error()) {
            nbl::log(nbl::LogLevel::WARN, "Unable to open the database!");
        } else {
            nbl::bumpInstallationPlayTime(&conn, iId, endTime - startTime);
        }
    }

    nbl::log(nbl::LogLevel::INFO, "Exiting wrapper...");
    return exitCode;
}
