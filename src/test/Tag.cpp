#include <Data/Model.hpp>

#include <gtest/gtest.h>

TEST(Tag, AddTag) {
 
    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    i64 tId = 0;
    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000, &tId).success());

    bool found = false;
    ASSERT_TRUE(nbl::descendants(&connection, tId, [&](nbl::TagMeta meta) -> nbl::Result {
        if(meta.name == "Action" && meta.color == 0xFF0000) {
            found = true;
        }
        return nbl::Result::SUCCESS;
    }).success());

    ASSERT_TRUE(found);
}

TEST(Tag, RemoveTag) {
 
    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    i64 tId = 0;
    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000, &tId).success());

    ASSERT_TRUE(nbl::removeTag(&connection, tId).success());

    bool found = false;
    ASSERT_TRUE(nbl::descendants(&connection, tId, [&](nbl::TagMeta meta) -> nbl::Result {
        if(meta.name == "Action" && meta.color == 0xFF0000) {
            found = true;
        }
        return nbl::Result::SUCCESS;
    }).success());

    ASSERT_FALSE(found);
}

TEST(Tag, AddParent) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    i64 tId1 = 0;
    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000, &tId1).success());
    i64 tId2 = 0;
    ASSERT_TRUE(nbl::addTag(&connection, "Adventure", 0x00FF00, &tId2).success());

    ASSERT_TRUE(nbl::addParentTag(&connection, tId1, tId2).success());

    struct _FoundData {
        bool foundAction = false;
        bool foundAdventure = false;
        void reset() { foundAction = foundAdventure = false; }
    } found;

    auto func = [&](nbl::TagMeta tag) -> nbl::Result {
        if(tag.name == "Adventure" && tag.color == 0x00FF00) {
            found.foundAdventure = true;
        }
        if(tag.name == "Action" && tag.color == 0xFF0000) {
            found.foundAction = true;
        }
        return nbl::Result::SUCCESS;
    };


    ASSERT_TRUE(nbl::descendants(&connection, tId1, func).success());

    ASSERT_TRUE(found.foundAction);
    ASSERT_TRUE(found.foundAdventure);


    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, tId2, func).success());

    ASSERT_TRUE(found.foundAction);
    ASSERT_TRUE(found.foundAdventure);


    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, tId2, func).success());

    ASSERT_FALSE(found.foundAction);
    ASSERT_TRUE(found.foundAdventure);


    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, tId1, func).success());

    ASSERT_TRUE(found.foundAction);
    ASSERT_FALSE(found.foundAdventure);

}

TEST(Tag, RemoveParent) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    i64 tId1 = 0;
    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000, &tId1).success());
    i64 tId2 = 0;
    ASSERT_TRUE(nbl::addTag(&connection, "Adventure", 0x00FF00, &tId2).success());

    ASSERT_TRUE(nbl::addParentTag(&connection, tId1, tId2).success());
    ASSERT_TRUE(nbl::removeParentTag(&connection, tId1, tId2).success());

    struct _FoundData {
        bool foundAction = false;
        bool foundAdventure = false;
    } found;

    ASSERT_TRUE(nbl::descendants(&connection, tId1, [&](const nbl::TagMeta& tag) -> nbl::Result {
        if(tag.name == "Adventure" && tag.color == 0x00FF00) {
            found.foundAdventure = true;
        }
        if(tag.name == "Action" && tag.color == 0xFF0000) {
            found.foundAction = true;
        }
        return nbl::Result::SUCCESS;
    }).success());

    ASSERT_TRUE(found.foundAction);
    ASSERT_FALSE(found.foundAdventure);


    found.foundAction = false;
    found.foundAdventure = false;
    ASSERT_TRUE(nbl::ancestors(&connection, tId2, [&](const nbl::TagMeta& tag) -> nbl::Result {
        if(tag.name == "Adventure" && tag.color == 0x00FF00) {
            found.foundAdventure = true;
        }
        if(tag.name == "Action" && tag.color == 0xFF0000) {
            found.foundAction = true;
        }
        return nbl::Result::SUCCESS;
    }).success());

    ASSERT_FALSE(found.foundAction);
    ASSERT_TRUE(found.foundAdventure);

}

TEST(Tag, BlockCycles) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());
    i64 actId = 0;
    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000, &actId).success());
    i64 advId = 0;
    ASSERT_TRUE(nbl::addTag(&connection, "Adventure", 0x00FF00, &advId).success());

    ASSERT_TRUE(nbl::addParentTag(&connection, actId, advId).success());
    ASSERT_FALSE(nbl::addParentTag(&connection, advId, actId).success());
}


TEST(Tag, DeleteParent) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());
    i64 actId = 0;
    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000, &actId).success());
    i64 advId = 0;
    ASSERT_TRUE(nbl::addTag(&connection, "Adventure", 0x00FF00, &advId).success());

    ASSERT_TRUE(nbl::addParentTag(&connection, actId, advId).success());
    ASSERT_TRUE(nbl::removeTag(&connection, actId).success());

    struct _FoundData {
        bool foundAction = false;
        bool foundAdventure = false;
    } found;

    ASSERT_TRUE(nbl::descendants(&connection, actId, [&](const nbl::TagMeta& tag) -> nbl::Result {
        if(tag.name == "Adventure" && tag.color == 0x00FF00) {
            found.foundAdventure = true;
        }
        if(tag.name == "Action" && tag.color == 0xFF0000) {
            found.foundAction = true;
        }
        return nbl::Result::SUCCESS;
    }).success());

    ASSERT_FALSE(found.foundAction);
    ASSERT_FALSE(found.foundAdventure);

    found.foundAction = false;
    found.foundAdventure = false;
    ASSERT_TRUE(nbl::ancestors(&connection, advId, [&](const nbl::TagMeta& tag) -> nbl::Result {
        if(tag.name == "Adventure" && tag.color == 0x00FF00) {
            found.foundAdventure = true;
        }
        if(tag.name == "Action" && tag.color == 0xFF0000) {
            found.foundAction = true;
        }
        return nbl::Result::SUCCESS;
    }).success());

    ASSERT_FALSE(found.foundAction);
    ASSERT_TRUE(found.foundAdventure);

}

TEST(Tag, MultipleInheritance) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());
    i64 aId;
    i64 bId;
    i64 cId;
    i64 dId;
    ASSERT_TRUE(nbl::addTag(&connection, "a", 0xFF0000, &aId).success());
    ASSERT_TRUE(nbl::addTag(&connection, "b", 0x00FF00, &bId).success());
    ASSERT_TRUE(nbl::addTag(&connection, "c", 0x0000FF, &cId).success());
    ASSERT_TRUE(nbl::addTag(&connection, "d", 0xFFFFFF, &dId).success());
    
    ASSERT_TRUE(nbl::addParentTag(&connection, bId, aId).success());
    ASSERT_TRUE(nbl::addParentTag(&connection, cId, aId).success());
    ASSERT_TRUE(nbl::addParentTag(&connection, dId, bId).success());
    ASSERT_TRUE(nbl::addParentTag(&connection, dId, cId).success());
 
    struct _FoundData {
        bool foundA = false;
        bool foundB = false;
        bool foundC = false;
        bool foundD = false;
        void reset() { foundA = foundB = foundC = foundD = false; }
    } found;

    nbl::TagCallback findFunc = [&](const nbl::TagMeta& tag) -> nbl::Result {
        if(tag.name == "a" && tag.color == 0xFF0000) { found.foundA = true; }
        if(tag.name == "b" && tag.color == 0x00FF00) { found.foundB = true; }
        if(tag.name == "c" && tag.color == 0x0000FF) { found.foundC = true; }
        if(tag.name == "d" && tag.color == 0xFFFFFF) { found.foundD = true; }
        return nbl::Result::SUCCESS;
    };
    
    // Descendants
    ASSERT_TRUE(nbl::descendants(&connection, aId, findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, bId, findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, cId, findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_TRUE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, dId, findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_TRUE(found.foundC);
    ASSERT_TRUE(found.foundD);

    // Ancestors
    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, aId, findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_TRUE(found.foundC);
    ASSERT_TRUE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, bId, findFunc).success());
    ASSERT_FALSE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_TRUE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, cId, findFunc).success());
    ASSERT_FALSE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_TRUE(found.foundC);
    ASSERT_TRUE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, dId, findFunc).success());
    ASSERT_FALSE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_TRUE(found.foundD);

    
    // Remove parent and check again
    ASSERT_TRUE(nbl::removeParentTag(&connection, dId, bId).success());

    // Descendants
    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, aId, findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, bId, findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, cId, findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_TRUE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, dId, findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_TRUE(found.foundC);
    ASSERT_TRUE(found.foundD);

    // Ancestors
    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, aId, findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_TRUE(found.foundC);
    ASSERT_TRUE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, bId, findFunc).success());
    ASSERT_FALSE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, cId, findFunc).success());
    ASSERT_FALSE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_TRUE(found.foundC);
    ASSERT_TRUE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, dId, findFunc).success());
    ASSERT_FALSE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_TRUE(found.foundD);


    ASSERT_TRUE(nbl::addParentTag(&connection, dId, bId).success());
    ASSERT_TRUE(nbl::removeTag(&connection, cId).success());


    // Descendants
    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, aId, findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, bId, findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, cId, findFunc).success());
    ASSERT_FALSE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, dId, findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_TRUE(found.foundD);

    // Ancestors
    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, aId, findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_TRUE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, bId, findFunc).success());
    ASSERT_FALSE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_TRUE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, cId, findFunc).success());
    ASSERT_FALSE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, dId, findFunc).success());
    ASSERT_FALSE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_TRUE(found.foundD);
}

TEST(Tag, RenameTag) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());
    i64 tId = 0;
    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000, &tId).success());
    ASSERT_TRUE(nbl::renameTag(&connection, tId, "Adventure").success());

    bool found = false;
    ASSERT_TRUE(nbl::descendants(&connection, tId, [&](const nbl::TagMeta& tag) -> nbl::Result {
        if(tag.name == "Adventure" && tag.color == 0xFF0000) {
            found = true;
        }
        return nbl::Result::SUCCESS;
    }).success());

    ASSERT_TRUE(found);
}


TEST(Tag, SetTagColor) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());

    i64 tId = 0;
    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000, &tId).success());

    ASSERT_TRUE(nbl::setTagColor(&connection, tId, 0x00FF00).success());

    bool found = false;
    ASSERT_TRUE(nbl::descendants(&connection, tId, [&](const nbl::TagMeta& tag) -> nbl::Result {
        if(tag.name == "Action" && tag.color == 0x00FF00) {
            found = true;
        }
        return nbl::Result::SUCCESS;
    }).success());

    ASSERT_TRUE(found);
}
