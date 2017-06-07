#ifndef __SQLiteTableDVBT__H_ 
#define __SQLiteTableDVBT__H_

#include "SQLiteTableDVB.h"

class SQLiteTableDVBT : public SQLiteTableDVB {
public:
    SQLiteTableDVBT(DvbManager* manager);
    ~SQLiteTableDVBT();
    virtual bool load() { return false; }
    virtual bool save(int flag) { return false; }
};

#endif
