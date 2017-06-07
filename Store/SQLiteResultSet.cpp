#include "SQLiteResultSet.h"
#include "SQLiteDatabase.h"
#include <stdio.h>
#include <unistd.h>

SQLiteResultSet::SQLiteResultSet() :
    mDB(0), mStmt(0), mCached(false)
{
}

void 
SQLiteResultSet::_closeNeedNoticeDb(bool flag)
{
    if (mStmt && mDB) {
        SQLiteDatabase* db = mDB;
        mDB = 0;
        sqlite3_reset(mStmt);
        if (!mCached) {
            sqlite3_finalize(mStmt);
            mStmt = NULL;
        }
        if (flag)
            db->resultSetDidfinalize(this);
    }
}

void 
SQLiteResultSet::close()
{
    _closeNeedNoticeDb(true);
}

bool 
SQLiteResultSet::next()
{
    bool needRetry = true;
    int rc = SQLITE_OK; 
    for (int idx = 0; needRetry && idx <= 2; idx++) {
        needRetry = false;
        /* sqlite3_step() is called again to retrieve the next row of data, so INSERT UPDATE DELETE only exec once */
        rc = sqlite3_step(mStmt);
        if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc) {
            needRetry = true;
            if (SQLITE_LOCKED == rc) {
                /* The sqlite3_reset() function is called to reset a prepared statement object back to its initial state, ready to be re-executed */
                rc = sqlite3_reset(mStmt);
            }
            usleep(20);
        }
    }
    return (SQLITE_ROW == rc);
}


