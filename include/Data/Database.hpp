// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Database.hpp
 * \author jwall64
 * \brief Interface for interacting with a database
 */

#pragma once

#include <Core/Types.hpp>
#include <Core/Result.hpp>
#include <Core/String.hpp>
#include <Core/Span.hpp>
#include <Codec/Json.hpp>
#include <Platform/Logging.hpp>

namespace nbl {

/**
 * Enumerates the possible data types for values in the database
 */
enum class DataType {
    NULL_TYPE,
    INTEGER,
    FLOAT,
    TEXT
};

/**
 * A typed-union representing a value obtained from the database
 */
struct DataValue {

    /**
     * The type of data contained in this value
     */
    DataType type;

    /**
     * The actual value
     */
    union {
        i64 intValue;
        f64 floatValue;
        StringView textValue;
    };

    inline u64 uintValue() const {
        if(type != DataType::INTEGER) {
            return 0;
        }
        return static_cast<u64>(intValue);
    }
};


/**
 * A callback called with untyped data from the database. The values might be numbers or null, but have all been cast to strings. 
 */
typedef Result (*DatabaseCallback)(void* userData, i32 columnCount, const char** values, const char** columnNames, i64 lastInsertRowId);

/**
 * A callback called with typed data from the database.
 */
typedef Result (*TypedDatabaseCallback)(void* userData, i32 columnCount, DataValue* values, const char** columnNames, i64 lastInsertRowId);


class DatabaseConnection;

/**
 * \brief Reperesents a reusable prepared statement for a database connection
 */
class PreparedStatement {
public:

    /**
    * \brief Default constructor that creates an empty, invalid prepared statement
    */
    PreparedStatement() = default;

    /**
     * \brief Destructor. Frees the underlying data allocated with the statement 
     */
    ~PreparedStatement();

    /**
     * \brief Binds an int value to the given index of the prepared statement
     * \param index The index to bind the int value to 
     * \param value Tbe int value to bind
     * \return Whether the result of the binding operation was successful
     */
    Result bindInt(i32 index, i64 value);

    /**
     * \brief Binds an unsigned int value to the given index of the prepared statement
     * \param index The index to bind the unsigned int value to 
     * \param value Tbe unsigned int value to bind
     * \return Whether the result of the binding operation was successful
     */
    Result bindUint(i32 index, u64 value);

    /**
     * \brief Binds a float value to the given index of the prepared statement
     * \param index The index to bind the float value to 
     * \param value Tbe float value to bind
     * \return Whether the result of the binding operation was successful
     */
    Result bindFloat(i32 index, f64 value);

    /**
     * \brief Binds a string value to the given index of the prepared statement
     * \param index The index to bind the string value to 
     * \param value Tbe string value to bind
     * \return Whether the result of the binding operation was successful
     */
    Result bindString(i32 index, StringView value);

    /**
     * \brief Binds a string value or null to the given index of the prepared statement, depending on whether the string is empty
     * \param index The index to bind the string or null value to 
     * \param value Tbe string value to bind, or an empty string to bind null
     * \return Whether the result of the binding operation was successful
     */
    Result bindStringOrNull(i32 index, StringView value) {
        if(value.empty()) {
            return bindNull(index);
        } else {
            return bindString(index, value);
        }
    }

    /**
     * \brief Binds a null value to the given index of the prepared statement
     * \param index The index to bind the int value to 
     * \return Whether the result of the binding operation was successful
     */
    Result bindNull(i32 index);

    /**
     * \brief Resets the parameters bound to the statement
     * \return Whether the result of the reset operation was successful
     */
    Result reset();

    /**
     * \brief Checks if this statement is valid for execution on the given database connection
     * \return Whether the statement is valid
     */
    bool isValid(DatabaseConnection* conn) const;

private:
    void* _pConnectionHandle;
    void* _pHandle;

    friend class DatabaseConnection;

};


/**
 * \brief Represents a connection to a database
 */
class DatabaseConnection {
public:

    /**
     * \brief Contstructor that creates a unconnected "connection"
     */
    DatabaseConnection() = default;

    /**
     * \brief Destructor that frees all data related to the database connection, if any
     */
    ~DatabaseConnection();

    // Deleted copy constructor
    DatabaseConnection(const DatabaseConnection&) = delete;

    /**
     * \brief Creates a new prepared statement using the given query
     * \param query The SQL query to parse
     * \param outStatement A pointer to the location to store the prepared statement
     * \return The result of the prepare operation
     */
    Result prepare(StringView query, PreparedStatement* outStatement);

    /**
     * \brief Parses and executes an unprepeared SQL query
     * \param query The SQL query to parse and execute
     * \param callback The function to call for each row of the query's output. May be nullptr
     * \param userData The user data which will be passed into the callback function. May be nullptr
     * \return The result of the prepare operation
     */
    Result execute(StringView query, DatabaseCallback callback, void* userData);

    /**
     * \brief Executes an prepeared SQL query
     * \param query The SQL query to execute
     * \param callback The function to call for each row of the query's output. May be nullptr
     * \param userData The user data which will be passed into the callback function. May be nullptr
     * \return The result of the prepare operation
     */
    Result execute(PreparedStatement* query, TypedDatabaseCallback callback, void* userData, i64* lastInsertRowId = nullptr);

    /**
     * \brief Begins a new transaction. No changes will be committed until commitTransaction() is called
     * \return The result of the begin transaction operation
     */
    Result beginTransaction();

    /**
     * \brief Commits a transaction. All statements since the last beginTransaction() call will be committed to the database. 
     * \return The result of the commit operation
     */
    Result commitTransaction();

    /**
     * \brief Rolls back all changes made since the last call to beginTransaction();
     * \return The result of the rollback operation
     */
    Result rollbackTransaction();

    /**
     * \brief Executes a series of prepared statements within a transaction. If any of the statements fail, no changes will be committed to the database.
     * \return The result of the entire transaction
     */
    Result doTransaction(Span<PreparedStatement*> statements, i64* lastInsertRowId = nullptr);

    /**
     * \brief Checks if the database connection is valid
     * \return Whether the connection is valid
     */
    inline bool isValid() const { return _pHandle != nullptr; }

    /**
     * \brief Opens a new database connection by reading a SQLite file on disk
     * \param path The path to the database file to open
     * \param outConnection A pointer to the connection to initialize
     * \return The result of the open operation
     */
    static Result open(StringView path, DatabaseConnection* outConnection);

    /**
     * \brief Opens a new database connection in memory
     * \param outConnection A pointer to the connection to initialize
     * \return The result of the open operation
     */
    static Result openInMemory(DatabaseConnection* outConnection);

private:

    Arena _mArena;
    void* _pHandle;

    bool _bInTransaction;

    friend class PreparedStatement;

};

inline bool nbl::PreparedStatement::isValid(DatabaseConnection* conn) const {
    return _pHandle != nullptr && _pConnectionHandle == conn->_pHandle;
}


}
