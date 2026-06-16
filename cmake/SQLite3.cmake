CPMAddPackage(
    NAME SQLite3
    URL "https://sqlite.org/2026/sqlite-amalgamation-3530100.zip"
    URL_HASH SHA3_256=3c07136e4f6b5dd0c395be86455014039597bc65b6851f7111e88f71b6e06114
    DOWNLOAD_ONLY YES
)

if(SQLite3_ADDED)
    add_library(sqlite3_lib STATIC ${SQLite3_SOURCE_DIR}/sqlite3.c)
    target_include_directories(sqlite3_lib INTERFACE ${SQLite3_SOURCE_DIR})
    target_compile_definitions(sqlite3_lib PRIVATE SQLITE_THREADSAFE=1 SQLITE_ENABLE_COLUMN_METADATA)
    set_target_properties(sqlite3_lib PROPERTIES POSITION_INDEPENDENT_CODE ON)

    add_library(SQLite3::SQLite3 ALIAS sqlite3_lib)
endif()
