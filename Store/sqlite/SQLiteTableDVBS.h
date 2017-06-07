#ifndef __SQLiteTableDVBS__H_ 
#define __SQLiteTableDVBS__H_

#include "SQLiteTableDVB.h"

class SQLiteTableDVBS : public SQLiteTableDVB {
public:
    SQLiteTableDVBS(DvbManager* manager);
    ~SQLiteTableDVBS();
    virtual bool load();
    virtual bool save(int flag);
};

#endif
