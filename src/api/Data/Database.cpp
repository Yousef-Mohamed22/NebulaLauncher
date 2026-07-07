// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Database.cpp
 * \author jwall64
 * \brief Implementation of Data/Database.hpp using SQLite.
 */

#include <Core/Result.hpp>
#include <Platform/Arena.hpp>
#include <Codec/Json.hpp>
#include <Platform/Logging.hpp>
#include <Platform/StringBuilder.hpp>
#include <Data/Database.hpp>

#include <climits>
#include <sqlite3.h>


namespace {

int _databaseCallback(void* userData, int argc, char** argv, char** colNames) {

    i64 lastInsertRowId = sqlite3_last_insert_rowid(reinterpret_cast<sqlite3*>(userData));

    nbl::DatabaseCallback* callback = static_cast<nbl::DatabaseCallback*>(userData);
    nbl::Result res = (*callback)(argc, const_cast<const char**>(argv), const_cast<const char**>(colNames), lastInsertRowId);
    if(!res.success()) {
        return -1;
    }
    return 0;
}

void _strDistance(sqlite3_context* ctx, int argc, sqlite3_value** argv) {

    nbl::StringView str1 = {};
    nbl::StringView str2 = {};

    if(argc > 0) {
        const unsigned char* data1 = sqlite3_value_text(argv[0]);
        if(data1) {
            str1.length = strlen(reinterpret_cast<const char*>(data1));
            str1.data = reinterpret_cast<const char*>(data1);
        }
    }
    if(argc > 1) {
        const unsigned char* data2 = sqlite3_value_text(argv[1]);
        if(data2) {
            str2.length = strlen(reinterpret_cast<const char*>(data2));
            str2.data = reinterpret_cast<const char*>(data2);
        }
    }
    sqlite3_result_int64(ctx, str1.distance(str2));
}

}



nbl::PreparedStatement::~PreparedStatement() {
    if(_pHandle) {
        sqlite3_finalize(reinterpret_cast<sqlite3_stmt*>(_pHandle));
        _pHandle = nullptr;
    }
}

nbl::Result nbl::PreparedStatement::reset() {
    int rc = sqlite3_reset(reinterpret_cast<sqlite3_stmt*>(_pHandle));
    if(rc != SQLITE_OK) {
        return nbl::error::IO_ERROR;
    }
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::PreparedStatement::bindInt(i32 index, i64 value) {
    int rc = sqlite3_bind_int64(reinterpret_cast<sqlite3_stmt*>(_pHandle), index, value);
    if(rc != SQLITE_OK) {
        nbl::log(nbl::LogLevel::ERROR, "Failed to bind int parameter at index ", index, ": ", sqlite3_errmsg(reinterpret_cast<sqlite3*>(_pHandle)));
        return nbl::error::IO_ERROR;
    }
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::PreparedStatement::bindUint(i32 index, u64 value) {
    int rc = sqlite3_bind_int64(reinterpret_cast<sqlite3_stmt*>(_pHandle), index, value);
    if(rc != SQLITE_OK) {
        nbl::log(nbl::LogLevel::ERROR, "Failed to bind uint parameter at index ", index, ": ", sqlite3_errmsg(reinterpret_cast<sqlite3*>(_pHandle)));
        return nbl::error::IO_ERROR;
    }
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::PreparedStatement::bindFloat(i32 index, f64 value) {
    int rc = sqlite3_bind_double(reinterpret_cast<sqlite3_stmt*>(_pHandle), index, value);
    if(rc != SQLITE_OK) {
        nbl::log(nbl::LogLevel::DEBUG, "Failed to bind float parameter at index ", index, ": ", sqlite3_errmsg(reinterpret_cast<sqlite3*>(_pHandle)));
        return nbl::error::IO_ERROR;
    }
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::PreparedStatement::bindString(i32 index, nbl::StringView value) {

    NBL_ASSERT(value.length <= static_cast<usize>(INT_MAX));

    int rc = sqlite3_bind_text(
        reinterpret_cast<sqlite3_stmt*>(_pHandle), 
        index, 
        value.data, 
        static_cast<int>(value.length), 
        SQLITE_TRANSIENT);

    if(rc != SQLITE_OK) {
        nbl::log(nbl::LogLevel::DEBUG, "Failed to bind string parameter at index ", index, ": ", sqlite3_errmsg(reinterpret_cast<sqlite3*>(_pHandle)));
        return nbl::error::IO_ERROR;
    }
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::PreparedStatement::bindNull(i32 index) {
    int rc = sqlite3_bind_null(reinterpret_cast<sqlite3_stmt*>(_pHandle), index);
    if(rc != SQLITE_OK) {
        nbl::log(nbl::LogLevel::DEBUG, "Failed to bind null parameter at index ", index, ": ", sqlite3_errmsg(reinterpret_cast<sqlite3*>(_pHandle)));
        return nbl::error::IO_ERROR;
    }
    return nbl::Result::SUCCESS;
}


nbl::DatabaseConnection::~DatabaseConnection() {
    if(_pHandle) {
        sqlite3_close_v2(reinterpret_cast<sqlite3*>(_pHandle));
        _pHandle = nullptr;
    }
}


nbl::Result nbl::DatabaseConnection::prepare(nbl::StringView query, PreparedStatement* outStatement) {

    outStatement->~PreparedStatement();

    NBL_ASSERT(query.length <= static_cast<usize>(INT_MAX));
    outStatement->_pConnectionHandle = _pHandle;

    int rc = sqlite3_prepare_v2(
        reinterpret_cast<sqlite3*>(_pHandle), 
        query.data, 
        static_cast<int>(query.length), 
        reinterpret_cast<sqlite3_stmt**>(&outStatement->_pHandle), 
        nullptr);

    if(rc != SQLITE_OK) {
        nbl::log(nbl::LogLevel::ERROR, "Failed to prepare statement: ", sqlite3_errmsg(reinterpret_cast<sqlite3*>(_pHandle)));
        return nbl::error::IO_ERROR;
    }

    return nbl::Result::SUCCESS;
}



nbl::Result nbl::DatabaseConnection::execute(nbl::StringView query) {

    int rc = sqlite3_exec(
        reinterpret_cast<sqlite3*>(_pHandle), 
        query.data, 
        nullptr,
        nullptr,
        nullptr);

    if(rc != SQLITE_OK) {
        nbl::log(nbl::LogLevel::ERROR, "Database execution error: ", sqlite3_errmsg(reinterpret_cast<sqlite3*>(_pHandle)));
        return nbl::error::IO_ERROR;
    }
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::DatabaseConnection::execute(nbl::StringView query, DatabaseCallback callback) {

    int rc = sqlite3_exec(
        reinterpret_cast<sqlite3*>(_pHandle), 
        query.data, 
        _databaseCallback, 
        &callback, 
        nullptr);

    if(rc != SQLITE_OK) {
        nbl::log(nbl::LogLevel::ERROR, "Database execution error: ", sqlite3_errmsg(reinterpret_cast<sqlite3*>(_pHandle)));
        return nbl::error::IO_ERROR;
    }
    return nbl::Result::SUCCESS;
}


nbl::Result nbl::DatabaseConnection::execute(PreparedStatement* query, i64* lastInsertRowId) {

    if(!query->isValid(this)) {
        nbl::log(nbl::LogLevel::ERROR, "Invalid prepared statement for this connection");
        return nbl::error::ILLEGAL_STATE;
    }

    sqlite3_stmt* stmt = reinterpret_cast<sqlite3_stmt*>(query->_pHandle);
    
#ifndef NDEBUG

    char* stmtStr = sqlite3_expanded_sql(stmt);
    StringView stmtView = stmtStr;

    nbl::log(nbl::LogLevel::DEBUG, "Running SQL: ", stmtView);

    sqlite3_free(stmtStr);

#endif

    int rc;
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW) { }
    if(rc != SQLITE_DONE) {
        nbl::log(nbl::LogLevel::ERROR, "Database execution error: (", rc, ") ", sqlite3_errmsg(reinterpret_cast<sqlite3*>(_pHandle)));
        return nbl::error::IO_ERROR;
    }

    if(lastInsertRowId) {
        *lastInsertRowId = sqlite3_last_insert_rowid(reinterpret_cast<sqlite3*>(_pHandle));
    }

    NBL_CHECK(query->reset());
    return nbl::Result::SUCCESS;
}


nbl::Result nbl::DatabaseConnection::execute(PreparedStatement* query, TypedDatabaseCallback callback, i64* lastInsertRowId) {

    if(!query->isValid(this)) {
        nbl::log(nbl::LogLevel::ERROR, "Invalid prepared statement for this connection");
        return nbl::error::ILLEGAL_STATE;
    }

    sqlite3_stmt* stmt = reinterpret_cast<sqlite3_stmt*>(query->_pHandle);
    
#ifndef NDEBUG

    char* stmtStr = sqlite3_expanded_sql(stmt);
    StringView stmtView = stmtStr;

    nbl::log(nbl::LogLevel::DEBUG, "Running SQL: ", stmtView);

    sqlite3_free(stmtStr);

#endif

    int rc;
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {

        const char** columnNames = nullptr;
        DataValue* columnValues = nullptr;

        nbl::ArenaFrame frame(&_mArena);
        int columns = sqlite3_column_count(stmt);

        if(columns > 0) {

            NBL_CHECK(frame.allocate<const char*>(&columnNames, columns));
            NBL_CHECK(frame.allocate<DataValue>(&columnValues, columns));

            for(int i = 0; i < columns; i++) {
                columnNames[i] = sqlite3_column_name(stmt, i);

                int type = sqlite3_column_type(stmt, i);
                switch(type) {
                    case SQLITE_INTEGER:
                        columnValues[i].type = DataType::INTEGER;
                        columnValues[i].intValue = sqlite3_column_int64(stmt, i);
                        break;
                    case SQLITE_FLOAT:
                        columnValues[i].type = DataType::FLOAT;
                        columnValues[i].floatValue = sqlite3_column_double(stmt, i);
                        break;
                    case SQLITE_TEXT:
                        columnValues[i].type = DataType::TEXT;
                        columnValues[i].textValue = StringView(
                            static_cast<usize>(sqlite3_column_bytes(stmt, i)),
                            reinterpret_cast<const char*>(sqlite3_column_text(stmt, i))
                        );
                        break;
                    case SQLITE_NULL:
                        columnValues[i].type = DataType::NULL_TYPE;
                        columnValues[i].textValue = StringView();
                        break;
                    default:
                        nbl::log(nbl::LogLevel::ERROR, "Unknown column data type: ", type);
                        return nbl::error::IO_ERROR;
                }
            }
        }
        
        NBL_CHECK(callback(
                columns,
                columnValues, 
                columnNames, 
                sqlite3_last_insert_rowid(reinterpret_cast<sqlite3*>(_pHandle))
            ));
    }
    if(rc != SQLITE_DONE) {
        nbl::log(nbl::LogLevel::ERROR, "Database execution error: (", rc, ") ", sqlite3_errmsg(reinterpret_cast<sqlite3*>(_pHandle)));
        return nbl::error::IO_ERROR;
    }

    if(lastInsertRowId) {
        *lastInsertRowId = sqlite3_last_insert_rowid(reinterpret_cast<sqlite3*>(_pHandle));
    }

    NBL_CHECK(query->reset());
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::DatabaseConnection::beginTransaction() {

    if(_bInTransaction) return nbl::Result::SUCCESS;

    static nbl::PreparedStatement stmt;
    if(!stmt.isValid(this)) {
        NBL_CHECK(prepare("BEGIN TRANSACTION;", &stmt));
    }
    NBL_CHECK(execute(&stmt));
    _bInTransaction = true;
    return nbl::Result::SUCCESS;
}
nbl::Result nbl::DatabaseConnection::commitTransaction() {

    if(!_bInTransaction) return nbl::Result::SUCCESS;

    static nbl::PreparedStatement stmt;
    if(!stmt.isValid(this)) {
        NBL_CHECK(prepare("COMMIT;", &stmt));
    }
    NBL_CHECK(execute(&stmt));
    _bInTransaction = false;
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::DatabaseConnection::rollbackTransaction() {

    if(!_bInTransaction) return nbl::Result::SUCCESS;

    static nbl::PreparedStatement stmt;
    if(!stmt.isValid(this)) {
        NBL_CHECK(prepare("ROLLBACK;", &stmt));
    }
    return execute(&stmt);
    _bInTransaction = false;
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::DatabaseConnection::doTransaction(nbl::Span<PreparedStatement*> statements, i64* lastInsertRowId) {

    NBL_CHECK(beginTransaction());
    for(PreparedStatement* stmt : statements) {
        NBL_CHECK(execute(stmt, lastInsertRowId));
    }
    return commitTransaction();
}

nbl::Result nbl::DatabaseConnection::open(nbl::StringView path, DatabaseConnection* outConnection) {

    nbl::Arena::create(1024 * 1024, &outConnection->_mArena);

    nbl::ArenaFrame frame(&outConnection->_mArena);
    nbl::StringView nullTerminatedPath;
    NBL_CHECK(nbl::StringBuilder::nullTerminate(path, &nullTerminatedPath, &frame));

    int rc = sqlite3_open_v2(
        nullTerminatedPath.data, 
        reinterpret_cast<sqlite3**>(&outConnection->_pHandle), 
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 
        nullptr);

    if(rc != SQLITE_OK) {
        nbl::log(nbl::LogLevel::ERROR, "Failed to open database: ", sqlite3_errmsg(reinterpret_cast<sqlite3*>(outConnection->_pHandle)));
        return nbl::error::IO_ERROR;
    }

    sqlite3_create_function(
            static_cast<sqlite3*>(outConnection->_pHandle),
            "nbl_str_distance",
            2,
            SQLITE_UTF8,
            nullptr,
            _strDistance,
            nullptr,
            nullptr);

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::DatabaseConnection::openInMemory(DatabaseConnection* outConnection) {

    nbl::Arena::create(1024 * 1024, &outConnection->_mArena);

    int rc = sqlite3_open_v2(
        nullptr,
        reinterpret_cast<sqlite3**>(&outConnection->_pHandle), 
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MEMORY,
        nullptr);

    if(rc != SQLITE_OK) {
        nbl::log(nbl::LogLevel::ERROR, "Failed to open database: ", sqlite3_errmsg(reinterpret_cast<sqlite3*>(outConnection->_pHandle)));
        return nbl::error::IO_ERROR;
    }

    sqlite3_create_function(
            static_cast<sqlite3*>(outConnection->_pHandle),
            "nbl_str_distance",
            2,
            SQLITE_UTF8,
            nullptr,
            _strDistance,
            nullptr,
            nullptr);

    return nbl::Result::SUCCESS;
}

