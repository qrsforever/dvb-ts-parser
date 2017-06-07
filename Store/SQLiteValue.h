#ifndef __SQLiteValue__H_
#define __SQLiteValue__H_

#include "sqlite3/sqlite3.h"

typedef enum {
    SQLValueType_Null,
    SQLValueType_Double,
    SQLValueType_Text,
    SQLValueType_Blob,
    SQLValueType_Int,
    SQLValueType_Int64,
} SQLValueType;

struct SQLiteBlobValue {
    const void* bytes;
    int         length;
};

class SQLiteValue {
public:
    bool isNull()   const { return _type == SQLValueType_Null;   }
    bool isBlob()   const { return _type == SQLValueType_Blob;   }
    bool isDouble() const { return _type == SQLValueType_Double; }
    bool isText()   const { return _type == SQLValueType_Text;   }
    bool isInt()    const { return _type == SQLValueType_Int;    }
    bool isInt64()  const { return _type == SQLValueType_Int64;  }

    SQLValueType type()        const { return _type;          }

    const void* blobBytes()    const { return _u.blob.bytes;  }
    int         blobLength()   const { return _u.blob.length; }

    double doubleValue()       const { return _u.double_;     }
    const char* textValue()    const { return _u.text;        }
    int    intValue()          const { return _u.int_;        }
    sqlite3_int64 int64Value() const { return _u.int64_;      }

    SQLiteValue() { };
private:
    SQLiteValue(SQLValueType type) : _type(type) {}

    SQLValueType _type;
    union {
        double      double_;
        const char* text;
        int         int_;
        sqlite3_int64 int64_;
        SQLiteBlobValue blob;
    } _u;

    friend inline SQLiteValue SQLDouble(double);
    friend inline SQLiteValue SQLInt(int);
    friend inline SQLiteValue SQLInt64(sqlite3_int64);
    friend inline SQLiteValue SQLText(const char*);
    friend inline SQLiteValue SQLBlob(const void*, int length);
    friend inline SQLiteValue SQLBlob(const SQLiteBlobValue&);
    friend inline SQLiteValue SQLNull();
};

inline SQLiteValue SQLNull()
{
    return SQLiteValue(SQLValueType_Null);
}


inline SQLiteValue SQLDouble(double v)     
{   
    SQLiteValue tmp(SQLValueType_Double);
    tmp._u.double_ = v;
    return tmp;  
}


inline SQLiteValue SQLInt(int v)
{
    SQLiteValue tmp(SQLValueType_Int);
    tmp._u.int_ = v;
    return tmp;
}


inline SQLiteValue SQLInt64(sqlite3_int64 v)
{
    SQLiteValue tmp(SQLValueType_Int64);
    tmp._u.int64_ = v;
    return tmp;
}


inline SQLiteValue SQLText(const char* v)
{   
    SQLiteValue tmp(SQLValueType_Text);
    tmp._u.text = v;
    return tmp;  
}


inline SQLiteValue SQLBlob(const void* bytes, int length)
{
    SQLiteValue tmp(SQLValueType_Blob);
    tmp._u.blob.bytes = bytes;
    tmp._u.blob.length = length;
    return tmp;
}


inline SQLiteValue SQLBlob(const SQLiteBlobValue& blob)
{
    SQLiteValue tmp(SQLValueType_Blob);
    tmp._u.blob = blob;
    return tmp;
}


#endif
