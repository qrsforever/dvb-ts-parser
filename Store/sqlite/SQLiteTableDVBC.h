#ifndef __SQLiteTableDVBC__H_ 
#define __SQLiteTableDVBC__H_

#include "SQLiteTableDVB.h"

class SQLiteTableDVBC : public SQLiteTableDVB {
public:
    SQLiteTableDVBC(DvbManager* manager);
    ~SQLiteTableDVBC();
    virtual bool load() { return false; }
    virtual bool save(int flag) { return false; }
};

#endif
