// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file TagModel.cpp
 * \author jwall64
 * \brief Partial Implementation of Data/Model.hpp. Functions related to tag management.
 */

#include <Core/Array.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>
#include "Core/Result.hpp"


namespace {
    nbl::Result _processTagQuery(nbl::DatabaseConnection* connection, nbl::PreparedStatement* stmt, nbl::TagCallback callback) {
        return connection->execute(stmt, [&](i32 columnCount, nbl::DataValue* columnValues, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {
            if(columnCount != 3) {
                return nbl::Result::IO_ERROR;
            }
            if(columnValues[0].type != nbl::DataType::INTEGER || columnValues[1].type != nbl::DataType::TEXT || columnValues[2].type != nbl::DataType::INTEGER) {
                return nbl::Result::IO_ERROR;
            }
            return callback(nbl::TagMeta{
                .id = columnValues[0].intValue,
                .name = columnValues[1].textValue, 
                .color = static_cast<u32>(columnValues[2].uintValue())
            });
        });
    }
}



nbl::Result nbl::getTagMeta(DatabaseConnection* connection, i64 tag, TagCallback callback) {

    static nbl::PreparedStatement _metaStmt;
    if(!_metaStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT tId, name, color FROM tag 
WHERE tId = ?;)", &_metaStmt));
    }


    NBL_CHECK(_metaStmt.bindInt(1, tag));
    return _processTagQuery(connection, &_metaStmt, callback);
}


nbl::Result nbl::descendants(DatabaseConnection* connection, i64 tag, TagCallback callback) {

    static nbl::PreparedStatement _descendantsStmt;
    if(!_descendantsStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT tId, name, color FROM tag 
JOIN tag_closure ON tag_closure.descendantId = tag.tId
WHERE tag_closure.ancestorId = ?;)", &_descendantsStmt));
    }


    NBL_CHECK(_descendantsStmt.bindInt(1, tag));
    return _processTagQuery(connection, &_descendantsStmt, callback);
}

nbl::Result nbl::ancestors(DatabaseConnection* connection, i64 tag, TagCallback callback) {

    static nbl::PreparedStatement _ancestorsStmt;
    if(!_ancestorsStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT tId, name, color FROM tag 
JOIN tag_closure ON tag_closure.ancestorId = tag.tId
WHERE tag_closure.descendantId = ?;)", &_ancestorsStmt));
    }

    NBL_CHECK(_ancestorsStmt.bindInt(1, tag));
    return _processTagQuery(connection, &_ancestorsStmt, callback);
}

nbl::Result nbl::parents(DatabaseConnection* connection, i64 tag, TagCallback callback) {

    static nbl::PreparedStatement _parentsStmt;
    if(!_parentsStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT tId, name, color FROM tag 
JOIN tag_closure ON tag_closure.ancestorId = tag.tId
WHERE tag_closure.descendantId = ? AND depth = 1;)", &_parentsStmt));
    }

    NBL_CHECK(_parentsStmt.bindInt(1, tag));
    return _processTagQuery(connection, &_parentsStmt, callback);
}

nbl::Result nbl::addTag(DatabaseConnection* connection, StringView tag, u32 color, i64* outTagId) {

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

    NBL_CHECK(connection->doTransaction(nbl::Array<PreparedStatement*,2>{ &_addTagStmt, &_addInitialClosureStmt }, outTagId));
   
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::addParentTag(DatabaseConnection* connection, i64 parentTag, i64 childTag) {

    // Check to see if the parent is a descendant of the child, which would create a cycle
    static PreparedStatement _checkCycleStmt;
    if(!_checkCycleStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT 1 FROM tag_closure
WHERE ancestorId = ?
AND descendantId = ?;)", &_checkCycleStmt));
    }
    NBL_CHECK(_checkCycleStmt.bindInt(1, childTag));
    NBL_CHECK(_checkCycleStmt.bindInt(2, parentTag));

    bool cycleExists = false;
    NBL_CHECK(connection->execute(&_checkCycleStmt, [&](i32 columnCount, DataValue* columnValues, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {
        cycleExists = true;
        return nbl::Result::SUCCESS;
    }));

    if(cycleExists) {
        return nbl::Result::INVALID_STATE;
    }

    static nbl::PreparedStatement _addParentClosureStmt;
    if(!_addParentClosureStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
INSERT INTO tag_closure
(ancestorId, descendantId, depth)
    SELECT ?1, descendantId, depth + 1
    FROM tag_closure
    WHERE ancestorId = ?2
ON CONFLICT(ancestorId, descendantId) DO
	UPDATE SET depth = excluded.depth
	WHERE excluded.depth < depth;
        )", &_addParentClosureStmt));
    }

    NBL_CHECK(_addParentClosureStmt.bindInt(1, parentTag));
    NBL_CHECK(_addParentClosureStmt.bindInt(2, childTag));

    return connection->execute(&_addParentClosureStmt);
}

nbl::Result nbl::removeParentTag(DatabaseConnection* connection, i64 parentTag, i64 childTag) {

    static nbl::PreparedStatement _removeParentClosureStmt;

    if(!_removeParentClosureStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
WITH
	rIds AS (
		SELECT ancestorId
		FROM tag_closure
		WHERE descendantId = ?2
	),
	rFrom AS (
		SELECT descendantId
		FROM tag_closure
		WHERE ancestorId = ?1
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
		AND NOT (tc2.descendantId = ?1 AND tc2.ancestorId = ?2)
		
	  UNION ALL
	
	  SELECT tc3.ancestorId
	  FROM tag_closure tc3
	  JOIN hierarchy h ON tc3.descendantId = h.ancestorId
	  WHERE tc3.depth = 1
		AND NOT (tc3.descendantId = ?1 AND tc3.ancestorId = ?2)
	)
	SELECT ancestorId FROM hierarchy
	WHERE ancestorId = ?2
  );
        )", &_removeParentClosureStmt));
    } else {
        NBL_CHECK(_removeParentClosureStmt.reset());
    }

    NBL_CHECK(_removeParentClosureStmt.bindInt(1, childTag));
    NBL_CHECK(_removeParentClosureStmt.bindInt(2, parentTag));

    return connection->execute(&_removeParentClosureStmt);

}

nbl::Result nbl::removeTag(DatabaseConnection* connection, i64 tag) {

    static nbl::PreparedStatement _removeClosureStmt;
    if(!_removeClosureStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
WITH
	ancestors AS (
		SELECT ancestorId
		FROM tag_closure
		WHERE descendantId = ?1
	),
	descendants AS (
		SELECT descendantId
		FROM tag_closure
		WHERE ancestorId = ?1
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
		AND tc2.ancestorId != ?1
		AND tc2.descendantId != ?1
		
	  UNION ALL
	
	  SELECT tc3.ancestorId
	  FROM tag_closure tc3
	  JOIN hierarchy h ON tc3.descendantId = h.ancestorId
	  WHERE tc3.depth = 1
		AND tc3.ancestorId != ?1
		AND tc3.descendantId != ?1
	)
	SELECT 1 FROM hierarchy
	WHERE ancestorId = tc.ancestorId
  );
        )", &_removeClosureStmt));
    }

    static nbl::PreparedStatement _removeTagStmt;
    if(!_removeTagStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare("DELETE FROM tag WHERE tId = ?;", &_removeTagStmt));
    } 

    NBL_CHECK(_removeClosureStmt.bindInt(1, tag));
    NBL_CHECK(_removeTagStmt.bindInt(1, tag));

    return connection->doTransaction(nbl::Array<PreparedStatement*,2>{ &_removeClosureStmt, &_removeTagStmt });
}

nbl::Result nbl::setTagColor(DatabaseConnection* connection, i64 tag, u32 color) {

    static nbl::PreparedStatement _setTagColorStmt;
    if(!_setTagColorStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare("UPDATE tag SET color = ? WHERE tId = ?;", &_setTagColorStmt));
    } 

    NBL_CHECK(_setTagColorStmt.bindUint(1, color));
    NBL_CHECK(_setTagColorStmt.bindInt(2, tag));

    return connection->execute(&_setTagColorStmt);
}

nbl::Result nbl::renameTag(DatabaseConnection* connection, i64 tag, StringView newName) {

    static nbl::PreparedStatement _renameTagStmt;
    if(!_renameTagStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare("UPDATE tag SET name = ?1 WHERE tId = ?2;", &_renameTagStmt));
    }

    NBL_CHECK(_renameTagStmt.bindInt(2, tag));
    NBL_CHECK(_renameTagStmt.bindString(1, newName));

    return connection->execute(&_renameTagStmt);
}

nbl::Result nbl::enumerateTags(DatabaseConnection *connection, u32 limit, u32 offset, TagCallback callback) {

    static nbl::PreparedStatement _enumerateStmt;
    if(!_enumerateStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare("SELECT tId, name, color FROM tag ORDER BY name ASC LIMIT ?1 OFFSET ?2;", &_enumerateStmt));
    }

    NBL_CHECK(_enumerateStmt.bindUint(1, limit));
    NBL_CHECK(_enumerateStmt.bindUint(2, offset));

    return _processTagQuery(connection, &_enumerateStmt, callback);
}


nbl::Result nbl::searchTags(DatabaseConnection *connection, StringView query, u32 limit, u32 offset, TagCallback callback) {

    static nbl::PreparedStatement searchStmt;
    if(!searchStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT tId, name, color FROM tag 
WHERE ?3 IS NULL OR name LIKE '%' || ?3 || '%' 
ORDER BY nbl_str_distance(name, ?3)
LIMIT ?1 OFFSET ?2
        )", &searchStmt));
    }

    NBL_CHECK(searchStmt.bindUint(1, limit));
    NBL_CHECK(searchStmt.bindUint(2, offset));
    NBL_CHECK(searchStmt.bindStringOrNull(3, query));

    return _processTagQuery(connection, &searchStmt, callback);
}


nbl::Result nbl::findTag(DatabaseConnection* connection, StringView name, TagCallback callback) {

    static nbl::PreparedStatement _findStmt;
    if(!_findStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
SELECT tId, name, color FROM tag 
WHERE name = ?1;
        )", &_findStmt));
    }

    NBL_CHECK(_findStmt.bindString(1, name));

    bool found = false;
    nbl::Result res = connection->execute(&_findStmt, [&](i32 columnCount, nbl::DataValue* columnValues, const char** columnNames, i64 lastInsertRowId) -> nbl::Result {
        if(columnCount != 3) {
            return nbl::Result::IO_ERROR;
        }
        if(columnValues[0].type != nbl::DataType::INTEGER || columnValues[1].type != nbl::DataType::TEXT || columnValues[2].type != nbl::DataType::INTEGER) {
            return nbl::Result::IO_ERROR;
        }
        found = true;
        return callback(nbl::TagMeta{
            .id = columnValues[0].intValue,
            .name = columnValues[1].textValue, 
            .color = static_cast<u32>(columnValues[2].uintValue())
        });
    });

    if(!found) return nbl::Result::NOT_FOUND;
    return nbl::Result::SUCCESS;
}


nbl::Result nbl::getTagsFromIds(DatabaseConnection* connection, StringView jsonIds, TagCallback callback) {
    static nbl::PreparedStatement _byIdsStmt;
    if(!_byIdsStmt.isValid(connection)) {
        NBL_CHECK(connection->prepare(R"(
WITH 
tag_names AS (
	SELECT value AS tId
	FROM json_each(?1)
)
SELECT tId, name, color
FROM tag
WHERE tId IN tag_names
ORDER BY name ASC;
        )", &_byIdsStmt));
    }
    
    NBL_CHECK(_byIdsStmt.bindString(1, jsonIds));

    return _processTagQuery(connection, &_byIdsStmt, callback);
}
