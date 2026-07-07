#include <Data/Model.hpp>

#include <gtest/gtest.h>

TEST(Tag, AddTag) {
 
    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());
    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000).success());

    bool found = false;
    ASSERT_TRUE(nbl::descendants(&connection, "Action", [&](nbl::StringView tagName, u32 tagColor) -> nbl::Result {
        if(tagName == "Action" && tagColor == 0xFF0000) {
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
    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000).success());

    ASSERT_TRUE(nbl::removeTag(&connection, "Action").success());

    bool found = false;
    ASSERT_TRUE(nbl::descendants(&connection, "Action", [&](nbl::StringView tagName, u32 tagColor) -> nbl::Result {
        if(tagName == "Action" && tagColor == 0xFF0000) {
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
    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000).success());
    ASSERT_TRUE(nbl::addTag(&connection, "Adventure", 0x00FF00).success());

    ASSERT_TRUE(nbl::addParentTag(&connection, "Action", "Adventure").success());

    struct _FoundData {
        bool foundAction = false;
        bool foundAdventure = false;
        void reset() { foundAction = foundAdventure = false; }
    } found;

    auto func = [&](nbl::StringView tagName, u32 tagColor) -> nbl::Result {
        if(tagName == "Adventure" && tagColor == 0x00FF00) {
            found.foundAdventure = true;
        }
        if(tagName == "Action" && tagColor == 0xFF0000) {
            found.foundAction = true;
        }
        return nbl::Result::SUCCESS;
    };


    ASSERT_TRUE(nbl::descendants(&connection, "Action", func).success());

    ASSERT_TRUE(found.foundAction);
    ASSERT_TRUE(found.foundAdventure);


    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, "Adventure", func).success());

    ASSERT_TRUE(found.foundAction);
    ASSERT_TRUE(found.foundAdventure);


    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, "Adventure", func).success());

    ASSERT_FALSE(found.foundAction);
    ASSERT_TRUE(found.foundAdventure);


    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, "Action", func).success());

    ASSERT_TRUE(found.foundAction);
    ASSERT_FALSE(found.foundAdventure);

}

TEST(Tag, RemoveParent) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());
    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000).success());
    ASSERT_TRUE(nbl::addTag(&connection, "Adventure", 0x00FF00).success());

    ASSERT_TRUE(nbl::addParentTag(&connection, "Action", "Adventure").success());
    ASSERT_TRUE(nbl::removeParentTag(&connection, "Action", "Adventure").success());

    struct _FoundData {
        bool foundAction = false;
        bool foundAdventure = false;
    } found;

    ASSERT_TRUE(nbl::descendants(&connection, "Action", [&](nbl::StringView tagName, u32 tagColor) -> nbl::Result {
        if(tagName == "Adventure" && tagColor == 0x00FF00) {
            found.foundAdventure = true;
        }
        if(tagName == "Action" && tagColor == 0xFF0000) {
            found.foundAction = true;
        }
        return nbl::Result::SUCCESS;
    }).success());

    ASSERT_TRUE(found.foundAction);
    ASSERT_FALSE(found.foundAdventure);


    found.foundAction = false;
    found.foundAdventure = false;
    ASSERT_TRUE(nbl::ancestors(&connection, "Adventure", [&](nbl::StringView tagName, u32 tagColor) -> nbl::Result {
        if(tagName == "Adventure" && tagColor == 0x00FF00) {
            found.foundAdventure = true;
        }
        if(tagName == "Action" && tagColor == 0xFF0000) {
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
    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000).success());
    ASSERT_TRUE(nbl::addTag(&connection, "Adventure", 0x00FF00).success());

    ASSERT_TRUE(nbl::addParentTag(&connection, "Action", "Adventure").success());
    ASSERT_FALSE(nbl::addParentTag(&connection, "Adventure", "Action").success());
}


TEST(Tag, DeleteParent) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());
    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000).success());
    ASSERT_TRUE(nbl::addTag(&connection, "Adventure", 0x00FF00).success());

    ASSERT_TRUE(nbl::addParentTag(&connection, "Action", "Adventure").success());
    ASSERT_TRUE(nbl::removeTag(&connection, "Action").success());

    struct _FoundData {
        bool foundAction = false;
        bool foundAdventure = false;
    } found;

    ASSERT_TRUE(nbl::descendants(&connection, "Action", [&](nbl::StringView tagName, u32 tagColor) -> nbl::Result {
        if(tagName == "Adventure" && tagColor == 0x00FF00) {
            found.foundAdventure = true;
        }
        if(tagName == "Action" && tagColor == 0xFF0000) {
            found.foundAction = true;
        }
        return nbl::Result::SUCCESS;
    }).success());

    ASSERT_FALSE(found.foundAction);
    ASSERT_FALSE(found.foundAdventure);

    found.foundAction = false;
    found.foundAdventure = false;
    ASSERT_TRUE(nbl::ancestors(&connection, "Adventure", [&](nbl::StringView tagName, u32 tagColor) -> nbl::Result {
        if(tagName == "Adventure" && tagColor == 0x00FF00) {
            found.foundAdventure = true;
        }
        if(tagName == "Action" && tagColor == 0xFF0000) {
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
    ASSERT_TRUE(nbl::addTag(&connection, "a", 0xFF0000).success());
    ASSERT_TRUE(nbl::addTag(&connection, "b", 0x00FF00).success());
    ASSERT_TRUE(nbl::addTag(&connection, "c", 0x0000FF).success());
    ASSERT_TRUE(nbl::addTag(&connection, "d", 0xFFFFFF).success());
    
    ASSERT_TRUE(nbl::addParentTag(&connection, "b", "a").success());
    ASSERT_TRUE(nbl::addParentTag(&connection, "c", "a").success());
    ASSERT_TRUE(nbl::addParentTag(&connection, "d", "b").success());
    ASSERT_TRUE(nbl::addParentTag(&connection, "d", "c").success());
 
    struct _FoundData {
        bool foundA = false;
        bool foundB = false;
        bool foundC = false;
        bool foundD = false;
        void reset() { foundA = foundB = foundC = foundD = false; }
    } found;

    nbl::TagCallback findFunc = [&](nbl::StringView tagName, u32 tagColor) -> nbl::Result {
        if(tagName == "a" && tagColor == 0xFF0000) { found.foundA = true; }
        if(tagName == "b" && tagColor == 0x00FF00) { found.foundB = true; }
        if(tagName == "c" && tagColor == 0x0000FF) { found.foundC = true; }
        if(tagName == "d" && tagColor == 0xFFFFFF) { found.foundD = true; }
        return nbl::Result::SUCCESS;
    };
    
    // Descendants
    ASSERT_TRUE(nbl::descendants(&connection, "a", findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, "b", findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, "c", findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_TRUE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, "d", findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_TRUE(found.foundC);
    ASSERT_TRUE(found.foundD);

    // Ancestors
    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, "a", findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_TRUE(found.foundC);
    ASSERT_TRUE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, "b", findFunc).success());
    ASSERT_FALSE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_TRUE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, "c", findFunc).success());
    ASSERT_FALSE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_TRUE(found.foundC);
    ASSERT_TRUE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, "d", findFunc).success());
    ASSERT_FALSE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_TRUE(found.foundD);

    
    // Remove parent and check again
    ASSERT_TRUE(nbl::removeParentTag(&connection, "d", "b").success());

    // Descendants
    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, "a", findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, "b", findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, "c", findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_TRUE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, "d", findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_TRUE(found.foundC);
    ASSERT_TRUE(found.foundD);

    // Ancestors
    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, "a", findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_TRUE(found.foundC);
    ASSERT_TRUE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, "b", findFunc).success());
    ASSERT_FALSE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, "c", findFunc).success());
    ASSERT_FALSE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_TRUE(found.foundC);
    ASSERT_TRUE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, "d", findFunc).success());
    ASSERT_FALSE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_TRUE(found.foundD);


    ASSERT_TRUE(nbl::addParentTag(&connection, "d", "b").success());
    ASSERT_TRUE(nbl::removeTag(&connection, "c").success());


    // Descendants
    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, "a", findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, "b", findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, "c", findFunc).success());
    ASSERT_FALSE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::descendants(&connection, "d", findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_TRUE(found.foundD);

    // Ancestors
    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, "a", findFunc).success());
    ASSERT_TRUE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_TRUE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, "b", findFunc).success());
    ASSERT_FALSE(found.foundA);
    ASSERT_TRUE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_TRUE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, "c", findFunc).success());
    ASSERT_FALSE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_FALSE(found.foundD);

    found.reset();
    ASSERT_TRUE(nbl::ancestors(&connection, "d", findFunc).success());
    ASSERT_FALSE(found.foundA);
    ASSERT_FALSE(found.foundB);
    ASSERT_FALSE(found.foundC);
    ASSERT_TRUE(found.foundD);
}

TEST(Tag, RenameTag) {

    nbl::DatabaseConnection connection;
    ASSERT_TRUE(nbl::DatabaseConnection::openInMemory(&connection).success());
    ASSERT_TRUE(nbl::initDatabase(&connection).success());
    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000).success());

    ASSERT_TRUE(nbl::renameTag(&connection, "Action", "Adventure").success());

    bool found = false;
    ASSERT_TRUE(nbl::descendants(&connection, "Adventure", [&](nbl::StringView tagName, u32 tagColor) -> nbl::Result {
        if(tagName == "Adventure" && tagColor == 0xFF0000) {
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
    ASSERT_TRUE(nbl::addTag(&connection, "Action", 0xFF0000).success());

    ASSERT_TRUE(nbl::setTagColor(&connection, "Action", 0x00FF00).success());

    bool found = false;
    ASSERT_TRUE(nbl::descendants(&connection, "Action", [&](nbl::StringView tagName, u32 tagColor) -> nbl::Result {
        if(tagName == "Action" && tagColor == 0x00FF00) {
            found = true;
        }
        return nbl::Result::SUCCESS;
    }).success());

    ASSERT_TRUE(found);
}
