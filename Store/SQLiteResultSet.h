#ifndef _SQLiteResultSet__H_
#define _SQLiteResultSet__H_

#include "SQLiteValue.h"

class SQLiteDatabase;
class SQLiteResultSet {
public:
    bool next();    
    void close();
    /* Routines below return information about a single column of the current result row of a query */
    int columnCount()                   const { return sqlite3_column_count(mStmt); }
    double columnDouble(int idx)        const { return sqlite3_column_double(mStmt, idx); }
    int columnInt(int idx)              const { return sqlite3_column_int(mStmt, idx); }
    sqlite3_int64 columnInt64(int idx)  const { return sqlite3_column_int64(mStmt, idx); }
    const char* columnText(int idx)     const { return (const char*)sqlite3_column_text(mStmt, idx); }

    SQLiteBlobValue columnBlob(int idx) const {
        int length = sqlite3_column_bytes(mStmt, idx);
        const void* bytes = sqlite3_column_blob(mStmt, idx);
        SQLiteBlobValue value = { bytes, length };
        return value;
    }

    int columnBytes(int idx)        const { return sqlite3_column_bytes(mStmt, idx); }
    int columnType(int idx)         const { return sqlite3_column_type(mStmt, idx); }
    const char* columnName(int idx) const { return (const char*)sqlite3_column_name(mStmt, idx); }
private:
    SQLiteResultSet();
    SQLiteResultSet& operator = (const SQLiteResultSet& rhs);
    SQLiteResultSet(const SQLiteResultSet& rhs);
    ~SQLiteResultSet()  {}
    void _closeNeedNoticeDb(bool flag); 

    bool isOpen() const { return mDB; }
    void open(SQLiteDatabase* db)  { mDB = db; }

private:
    friend class SQLiteDatabase;
    SQLiteDatabase* mDB;
    sqlite3_stmt* mStmt;
    bool mCached; /* record whether result set is in cacheResultSets, if false we need delete the result to free resource */
};

#endif
