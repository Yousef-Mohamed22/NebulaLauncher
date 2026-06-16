// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file TagModel.cpp
 * \author jwall64
 * \brief Partial Implementation of Data/Model.hpp. Functions related to tag management.
 */

#include <Core/Array.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>


namespace {


    nbl::Result _processTagQuery(nbl::DatabaseConnection* connection, nbl::PreparedStatement* stmt, nbl::TagCallback callback, void* userData) {

        struct _QueryData {
            nbl::TagCallback callback;
            void* userData;
        };
        _QueryData queryData{ callback, userData };
        return connection->execute(stmt, [](void* userData, i32 columnCount, nbl::DataValue* columnValues, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {

            if(columnCount != 2) {
                return nbl::error::IO_ERROR;
            }
            if(columnValues[0].type != nbl::DataType::TEXT || columnValues[1].type != nbl::DataType::INTEGER) {
                return nbl::error::IO_ERROR;
            }

            _QueryData* data = static_cast<_QueryData*>(userData);
            return data->callback(data->userData, columnValues[0].textValue, columnValues[1].intValue);

        }, &queryData);
    }
}



nbl::Result nbl::descendants(DatabaseConnection* connection, StringView tag, TagCallback callback, void* userData) {

    static nbl::PreparedStatement _descendantsStmt;
    if(!_descendantsStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT name, color FROM tag 
JOIN tag_closure ON tag_closure.descendantId = tag.tId
WHERE tag_closure.ancestorId = (SELECT tId FROM tag WHERE name = ?);)", &_descendantsStmt));
    }


    NBL_CHECK(_descendantsStmt.bindString(1, tag));
    return _processTagQuery(connection, &_descendantsStmt, callback, userData);
}

nbl::Result nbl::ancestors(DatabaseConnection* connection, StringView tag, TagCallback callback, void* userData) {

    static nbl::PreparedStatement _ancestorsStmt;
    if(!_ancestorsStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT name, color FROM tag 
JOIN tag_closure ON tag_closure.ancestorId = tag.tId
WHERE tag_closure.descendantId = (SELECT tId FROM tag WHERE name = ?);)", &_ancestorsStmt));
    }

    NBL_CHECK(_ancestorsStmt.bindString(1, tag));
    return _processTagQuery(connection, &_ancestorsStmt, callback, userData);
}

nbl::Result nbl::addTag(DatabaseConnection* connection, StringView tag, u32 color) {

    static nbl::PreparedStatement _addTagStmt;
    if(!_addTagStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare("INSERT INTO tag (name, color) VALUES (?, ?);", &_addTagStmt));
    }

    static nbl::PreparedStatement _addInitialClosureStmt;
    if(!_addInitialClosureStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare("INSERT INTO tag_closure (ancestorId, descendantId, depth) VALUES (last_insert_rowid(), last_insert_rowid(), 0);", &_addInitialClosureStmt));
    }

    NBL_CHECK(_addTagStmt.bindString(1, tag));
    NBL_CHECK(_addTagStmt.bindUint(2, color));

    NBL_CHECK(connection->doTransaction(nbl::Array<PreparedStatement*,2>{ &_addTagStmt, &_addInitialClosureStmt }));
   
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::addParentTag(DatabaseConnection* connection, StringView parentTag, StringView childTag) {

    // Check to see if the parent is a descendant of the child, which would create a cycle
    static PreparedStatement _checkCycleStmt;
    if(!_checkCycleStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT 1 FROM tag_closure
WHERE ancestorId = (SELECT tId FROM tag WHERE name = ?)
AND descendantId = (SELECT tId FROM tag WHERE name = ?);)", &_checkCycleStmt));
    }
    NBL_CHECK(_checkCycleStmt.bindString(1, childTag));
    NBL_CHECK(_checkCycleStmt.bindString(2, parentTag));

    bool cycleExists = false;
    NBL_CHECK(connection->execute(&_checkCycleStmt, [](void* userData, i32 columnCount, DataValue* columnValues, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {
        bool* cycleExists = static_cast<bool*>(userData);
        *cycleExists = true;
        return nbl::Result::SUCCESS;
    }, &cycleExists));

    if(cycleExists) {
        return nbl::error::ILLEGAL_STATE;
    }

    static nbl::PreparedStatement _addParentClosureStmt;
    if(!_addParentClosureStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO tag_closure
(ancestorId, descendantId, depth)
    SELECT (SELECT tId FROM tag WHERE name = ?), descendantId, depth + 1
    FROM tag_closure
    WHERE ancestorId = (SELECT tId FROM tag WHERE name = ?)
ON CONFLICT(ancestorId, descendantId) DO
	UPDATE SET depth = excluded.depth
	WHERE excluded.depth < depth;
        )", &_addParentClosureStmt));
    }

    NBL_CHECK(_addParentClosureStmt.bindString(1, parentTag));
    NBL_CHECK(_addParentClosureStmt.bindString(2, childTag));

    return connection->execute(&_addParentClosureStmt, nullptr, nullptr);
}

nbl::Result nbl::removeParentTag(DatabaseConnection* connection, StringView parentTag, StringView childTag) {

    static nbl::PreparedStatement _removeParentClosureStmt;

    if(!_removeParentClosureStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
WITH
    child AS (
        SELECT tId
        FROM tag
        WHERE name = ?
    ),
    parent AS (
        SELECT tId
        FROM tag
        WHERE name = ?
    ),
	rIds AS (
		SELECT ancestorId
		FROM tag_closure
		WHERE descendantId = (SELECT tId FROM parent)
	),
	rFrom AS (
		SELECT descendantId
		FROM tag_closure
		WHERE ancestorId = (SELECT tId FROM child)
	)
DELETE FROM tag_closure AS tc
WHERE tc.ancestorId IN rIds
  AND tc.descendantId IN rFrom
  AND NOT EXISTS(
	WITH RECURSIVE hierarchy AS (
	  SELECT tc2.ancestorId
	  FROM tag_closure tc2
	  WHERE tc2.descendantId = tc.descendantId
	    AND tc2.depth = 1
		AND NOT (tc2.descendantId = (SELECT tId FROM child) AND tc2.ancestorId = (SELECT tId FROM parent))
		
	  UNION ALL
	
	  SELECT tc3.ancestorId
	  FROM tag_closure tc3
	  JOIN hierarchy h ON tc3.descendantId = h.ancestorId
	  WHERE tc3.depth = 1
		AND NOT (tc3.descendantId = (SELECT tId FROM child) AND tc3.ancestorId = (SELECT tId FROM parent))
	)
	SELECT ancestorId FROM hierarchy
	WHERE ancestorId = (SELECT tId FROM parent)
  );
        )", &_removeParentClosureStmt));
    } else {
        NBL_CHECK(_removeParentClosureStmt.reset());
    }

    NBL_CHECK(_removeParentClosureStmt.bindString(1, childTag));
    NBL_CHECK(_removeParentClosureStmt.bindString(2, parentTag));

    return connection->execute(&_removeParentClosureStmt, nullptr, nullptr);

}

nbl::Result nbl::removeTag(DatabaseConnection* connection, StringView tag) {

    static nbl::PreparedStatement _removeClosureStmt;
    if(!_removeClosureStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
WITH
    toRemove AS (
        SELECT tId
        FROM tag
        WHERE name = ?
    ),
	ancestors AS (
		SELECT ancestorId
		FROM tag_closure
		WHERE descendantId = (SELECT tId FROM toRemove)
	),
	descendants AS (
		SELECT descendantId
		FROM tag_closure
		WHERE ancestorId = (SELECT tId FROM toRemove)
	)
DELETE FROM tag_closure AS tc
WHERE tc.ancestorId IN ancestors
  AND tc.descendantId IN descendants
  AND NOT EXISTS(
	WITH RECURSIVE hierarchy AS (
	  SELECT tc2.ancestorId
	  FROM tag_closure tc2
	  WHERE tc2.descendantId = tc.descendantId
	    AND tc2.depth = 1
		AND tc2.ancestorId != (SELECT tId FROM toRemove)
		AND tc2.descendantId != (SELECT tId FROM toRemove)
		
	  UNION ALL
	
	  SELECT tc3.ancestorId
	  FROM tag_closure tc3
	  JOIN hierarchy h ON tc3.descendantId = h.ancestorId
	  WHERE tc3.depth = 1
		AND tc3.ancestorId != (SELECT tId FROM toRemove)
		AND tc3.descendantId != (SELECT tId FROM toRemove)
	)
	SELECT 1 FROM hierarchy
	WHERE ancestorId = tc.ancestorId
  );
        )", &_removeClosureStmt));
    }

    static nbl::PreparedStatement _removeTagStmt;
    if(!_removeTagStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare("DELETE FROM tag WHERE name = ?;", &_removeTagStmt));
    } 

    NBL_CHECK(_removeClosureStmt.bindString(1, tag));
    NBL_CHECK(_removeTagStmt.bindString(1, tag));

    return connection->doTransaction(nbl::Array<PreparedStatement*,2>{ &_removeClosureStmt, &_removeTagStmt });
}

nbl::Result nbl::setTagColor(DatabaseConnection* connection, StringView tag, u32 color) {

    static nbl::PreparedStatement _setTagColorStmt;
    if(!_setTagColorStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare("UPDATE tag SET color = ? WHERE name = ?;", &_setTagColorStmt));
    } 

    NBL_CHECK(_setTagColorStmt.bindUint(1, color));
    NBL_CHECK(_setTagColorStmt.bindString(2, tag));

    return connection->execute(&_setTagColorStmt, nullptr, nullptr);
}

nbl::Result nbl::renameTag(DatabaseConnection* connection, StringView oldName, StringView newName) {

    static nbl::PreparedStatement _renameTagStmt;
    if(!_renameTagStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare("UPDATE tag SET name = ? WHERE name = ?;", &_renameTagStmt));
    }

    NBL_CHECK(_renameTagStmt.bindString(1, newName));
    NBL_CHECK(_renameTagStmt.bindString(2, oldName));

    return connection->execute(&_renameTagStmt, nullptr, nullptr);
}
