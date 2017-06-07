#include "DvbAssertions.h"
#include "SQLiteDatabase.h"
#include <unistd.h>

typedef std::map<std::string, SQLiteResultSet*>::iterator CachesIter;

SQLiteDatabase::SQLiteDatabase(): mSQLiteDB(NULL)
{
}

SQLiteDatabase::~SQLiteDatabase()
{
    if (close())
        LogDvbDebug("Close DB success.\n");
}

bool 
SQLiteDatabase::open(const char* filepath)
{
    if (!filepath)
        return false;

    std::string path(filepath);              
    if (path == mSQLiteDBPath)
        return false;

    close();
    mSQLiteDBPath = path;
    if (sqlite3_open(mSQLiteDBPath.c_str(), &mSQLiteDB) != SQLITE_OK) 
        return false;
    return true;
}

void 
SQLiteDatabase::uncacheCachesResultSets()
{
    for (CachesIter iter = mCacheResultSets.begin(); iter != mCacheResultSets.end(); ++iter) {
        SQLiteResultSet* rc = iter->second;
        rc->mCached = false;
        if (!rc->isOpen()) {
            sqlite3_finalize(rc->mStmt);
            delete rc;
        }
    }
    mCacheResultSets.clear();
}

void 
SQLiteDatabase::closeOpenedResultSets()
{
    std::size_t count = mOpenReusltSets.size();
    for (std::size_t idx = 0; idx < count; idx++) {
        SQLiteResultSet* rs = mOpenReusltSets[idx];
        rs->_closeNeedNoticeDb(false);
        if (!rs->mCached)
            delete rs;
    }
    mOpenReusltSets.clear();

    if (mReusedResultSet.isOpen())
        mReusedResultSet._closeNeedNoticeDb(false);
}

void 
SQLiteDatabase::resultSetDidfinalize(SQLiteResultSet* resultSet)
{
    if (resultSet != &mReusedResultSet) {
        size_t count = mOpenReusltSets.size();
        for (std::size_t idx = 0; idx < count; idx++) {
            SQLiteResultSet* rs = mOpenReusltSets[idx];
            if (rs == resultSet) {
                if (!rs->mCached)
                    delete rs;
                mOpenReusltSets.erase(mOpenReusltSets.begin() + idx);
                break;
            }
        }
    }
}

bool 
SQLiteDatabase::close()
{
    closeOpenedResultSets();
    uncacheCachesResultSets();

    if (!mSQLiteDB) 
        return true;

    bool triedFinalizingOpenStatements = false;
    bool needRetry = true;
    int  rc = SQLITE_OK;
    for (int idx = 0; idx <= 2 && needRetry; idx++) {
        needRetry = false;
        rc = sqlite3_close(mSQLiteDB);
        if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc) {
            LogDvbDebug("sqlite3_close : SQLITE_BUSY or SQLITE_LOCKED.\n");
            needRetry = true;
            usleep(20);
            if (!triedFinalizingOpenStatements) {
                triedFinalizingOpenStatements = true;
                sqlite3_stmt *pStmt;
                /* sqlite3_next_stmt:returns a pointer to the next prepared statement 
                 * after Statement associated with the database connection db,
                 * If Statement is NULL then this interface returns a pointer to 
                 * the first prepared statement associated with the database connection */
                while ((pStmt = sqlite3_next_stmt(mSQLiteDB, 0x00)) !=0) {
                    /* The application must finalize every prepared statement in order to avoid resource leaks */
                    sqlite3_finalize(pStmt);
                }
            }
        }
    }

    if (SQLITE_OK == rc) {
        mSQLiteDB = NULL;
        return true;
    }
    return false;
}

bool 
SQLiteDatabase::cacheSQL(const char* s)
{
    std::string sql(s);
    CachesIter iter = mCacheResultSets.find(sql);
    if ((iter != mCacheResultSets.end()) && (iter->first == sql))
        return false;

    sqlite3_stmt* pStmt = stmtPrepare(s); 
    if (!pStmt)
        return false;

    SQLiteResultSet* resultSet = new SQLiteResultSet;
    resultSet->mStmt = pStmt;
    resultSet->mCached = true;
    mCacheResultSets.insert(iter, std::make_pair(sql, resultSet));
    return true;
}

bool 
SQLiteDatabase::uncacheSQL(const char* s)
{
    if (mCacheResultSets.empty())
        return false;

    std::string sql(s);
    CachesIter iter = mCacheResultSets.find(sql);
    if ((iter == mCacheResultSets.end()) || (iter->first != sql))
        return false;

    SQLiteResultSet* rc = iter->second;
    rc->mCached = false;
    if (!rc->isOpen()) {
        sqlite3_finalize(rc->mStmt);
        delete rc;
    }
    mCacheResultSets.erase(iter);
    return true;
}

void 
SQLiteDatabase::bindValue(const SQLiteValue& value, int idx, sqlite3_stmt* stmt)
{
    switch (value.type()) {
        case SQLValueType_Null:
            sqlite3_bind_null(stmt, idx);
            break;

        case SQLValueType_Int:
            sqlite3_bind_int(stmt, idx, value.intValue());
            break;

        case SQLValueType_Double:
            sqlite3_bind_double(stmt, idx, value.doubleValue());
            break;

        case SQLValueType_Int64:
            sqlite3_bind_int64(stmt, idx, value.int64Value());
            break;

        case SQLValueType_Text:
            /* SQLITE_STATIC:  SQLite assumes that the information is in static, unmanaged space and does not need to be freed */
            sqlite3_bind_text(stmt, idx, value.textValue(), -1, SQLITE_STATIC);
            break;

        case SQLValueType_Blob:
            sqlite3_bind_blob(stmt, idx, value.blobBytes(), value.blobLength(), SQLITE_STATIC);
            break;

        default:
            break;
    }
}

sqlite3_stmt* 
SQLiteDatabase::stmtPrepare(const char* sql)
{
    if (!mSQLiteDB) 
        return NULL;

    sqlite3_stmt* pStmt = NULL; 
    bool needRetry = true;
    int  rc = SQLITE_OK;

    for (int idx = 0; idx <= 2&& needRetry; idx++) {
        needRetry = false;
        rc = sqlite3_prepare_v2(mSQLiteDB, sql, -1, &pStmt, 0);
        if (SQLITE_BUSY == rc || SQLITE_LOCKED == rc) {
            needRetry = true;
            usleep(20); 
        }
    }

    if (rc != SQLITE_OK) {
        LogDvbError("Please Check the SQL Statement[%s]! errorcode[%d]\n", sql, rc);
        sqlite3_finalize(pStmt);
        return NULL;
    }
    return pStmt;
}

SQLiteResultSet* 
SQLiteDatabase::findClosedCache(const char* sql)
{
    if (mCacheResultSets.empty()) 
        return NULL;

    std::string key(sql);
    CachesIter iter = mCacheResultSets.find(key);
    if ((iter != mCacheResultSets.end()) && (iter->first == sql)) {
        if (!iter->second->isOpen())
            return iter->second;
    }
    return NULL;
}

void 
SQLiteDatabase::bindValues(const SQLiteValue* values, int length, sqlite3_stmt* stmt)
{
    /*  returns the index of the largest (rightmost tokens: "?", "?NNN", ":AAA", "$AAA", or "@AAA") parameter */
    int queryCount = sqlite3_bind_parameter_count(stmt);
    int count = std::min(queryCount, length);
    for (int idx = 0; idx < count; idx++) 
        bindValue(values[idx], idx + 1, stmt);
}

bool 
SQLiteDatabase::exec(const char* sql)
{
    return exec(sql, NULL, 0);
}

bool 
SQLiteDatabase::exec(const char* sql, const SQLiteValue& v0)
{
    SQLiteValue values[] = { v0 };
    return exec(sql, values, sizeof(values) / sizeof(values[0]));
}

bool 
SQLiteDatabase::exec(const char* sql, const SQLiteValue& v0, const SQLiteValue& v1)
{
    SQLiteValue values[] = { v0, v1 };
    return exec(sql, values, sizeof(values) / sizeof(values[0]));
}

bool 
SQLiteDatabase::exec(const char* sql, const SQLiteValue* values, int length)
{
    SQLiteResultSet resultSet;
    SQLiteResultSet* pResultSet = findClosedCache(sql);
    if (!pResultSet) {
        resultSet.mStmt = stmtPrepare(sql);
        pResultSet = &resultSet;
    }
    pResultSet->open(this);
    if (!(pResultSet->mStmt))
        return false;
    bindValues(values, length, pResultSet->mStmt);
    pResultSet->next(); /* sqlite3_step */
    pResultSet->_closeNeedNoticeDb(false);
    return true;
}

SQLiteResultSet* 
SQLiteDatabase::query(const char* sql)
{
    return query(sql, NULL, 0);
}

SQLiteResultSet* 
SQLiteDatabase::query(const char* sql, const SQLiteValue& v0)
{
    SQLiteValue values[] = { v0 };
    return query(sql, values, sizeof(values) / sizeof(values[0]));
}

SQLiteResultSet* 
SQLiteDatabase::query(const char* sql, const SQLiteValue& v0, const SQLiteValue& v1)
{
    SQLiteValue values[] = { v0, v1 };
    return query(sql, values, sizeof(values) / sizeof(values[0]));
}

SQLiteResultSet* 
SQLiteDatabase::query(const char* sql, const SQLiteValue* values, int length)
{
    SQLiteResultSet* pResultSet = findClosedCache(sql);
    if (pResultSet)
        mOpenReusltSets.push_back(pResultSet);
    else if (mReusedResultSet.isOpen()) {
        pResultSet = new SQLiteResultSet();
        pResultSet->mStmt = stmtPrepare(sql);
        mOpenReusltSets.push_back(pResultSet);
    } else {
        pResultSet = &mReusedResultSet;
        pResultSet->mStmt = stmtPrepare(sql);
    }
    if (!(pResultSet->mStmt))
        return NULL;
    bindValues(values, length, pResultSet->mStmt);
    pResultSet->open(this);
    return pResultSet;
}

int 
SQLiteDatabase::load(const char* filepath)
{
    //Using file store
	int rc;  
	sqlite3 *fileDB;  
	sqlite3_backup *pBackup;  

	rc = sqlite3_open(filepath, &fileDB);  
	if (SQLITE_OK == rc) {  
		pBackup = sqlite3_backup_init(mSQLiteDB,"main",fileDB,"main");  
		if(pBackup) {  
			sqlite3_backup_step(pBackup,-1);  
			sqlite3_backup_finish(pBackup);  
		}  
		rc = sqlite3_errcode(mSQLiteDB);  
	}  
	sqlite3_close(fileDB);  
    return rc;
}

int 
SQLiteDatabase::save(const char* filepath)
{
    //Using file store
	int rc;  
	sqlite3 *fileDB;  
	sqlite3_backup *pBackup;  

	rc = sqlite3_open(filepath, &fileDB);  
	if(SQLITE_OK == rc) {  
		pBackup = sqlite3_backup_init(fileDB,"main",mSQLiteDB,"main");  
		if(pBackup) {  
			sqlite3_backup_step(pBackup,-1);  
			sqlite3_backup_finish(pBackup);  
		}  
		rc = sqlite3_errcode(fileDB);  
	}  
	sqlite3_close(fileDB);  
    return rc;
}
