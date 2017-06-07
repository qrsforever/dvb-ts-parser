#ifndef __SQLiteTableDVB__H_
#define __SQLiteTableDVB__H_

#include "config/pathConfig.h"

#include <iostream>
#include <list>
#include <map>


#define DVB_SQLITEDB_PATH DEFAULT_MODULE_DVB_DATAPATH"/dvbsqlite3.db"

class SQLiteDatabase;
class SQLiteResultSet;
class DvbManager;
class SQLiteTableDVB {
    friend class SQLiteDatabase;
    friend class SQLiteResultSet;
public :
    ~SQLiteTableDVB();
    int getTableVersion(const char* name);
    bool setTableVersion(const char* name, int version);
    virtual bool load() = 0;
    virtual bool save(int flag) = 0;
protected:
    bool _TableExists(const char* name);
    SQLiteDatabase* mDB;
    DvbManager* mManager;
    SQLiteTableDVB(DvbManager* manager);
};

#endif
