#include "DvbAssertions.h"
#include "SQLiteTableDVB.h"
#include "SQLiteDatabase.h"
#include "SQLiteResultSet.h"
#include "SQLiteValue.h"
#include "DvbChannel.h"
#include "DvbManager.h"

SQLiteTableDVB::SQLiteTableDVB(DvbManager* manager) : mManager(manager)
{
    mDB = new SQLiteDatabase();
    mDB->open(DVB_SQLITEDB_PATH);

    if (!_TableExists("versions")) {
        if (!mDB->exec("CREATE TABLE versions(TableName TEXT, Version INT)"))
            return;
    }
}

SQLiteTableDVB::~SQLiteTableDVB()
{
    delete(mDB);
}

bool
SQLiteTableDVB::_TableExists(const char* name)
{
    int count = 0;
    SQLiteResultSet* rs= mDB->query("SELECT count(*) FROM sqlite_master WHERE type='table' and name = ?", SQLText(name));
    if (rs) {
        rs->next();
        count = rs->columnInt(0);
        rs->close(); 
    }
    return (1 == count);
}

int 
SQLiteTableDVB::getTableVersion(const char* name)
{
    int version = 0;
    SQLiteResultSet* rs = mDB->query("SELECT Version FROM versions WHERE TableName = ?", SQLText(name));
    if (rs) {
        rs->next();
        version = rs->columnInt(0);
        rs->close();
    }
    return version;
}

bool 
SQLiteTableDVB::setTableVersion(const char* name, int version)
{
    return mDB->exec("INSERT INTO versions(TableName, Version) VALUES(?,?)", SQLText(name), SQLInt(version));
}
