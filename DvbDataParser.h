#ifndef __DvbDataParser__H_
#define __DvbDataParser__H_

#include <stdint.h>

class DvbDataParser {
public:
    DvbDataParser() { 
        mParserData = 0; 
        mParserSize = 0; 
        mDataLength = 0; 
    }
    ~DvbDataParser() { }
    void initData(const uint8_t* data = 0, int32_t len = 0, int32_t size = 0) {
        mParserData = data; 
        mParserSize = size; 
        mDataLength = len; 
    }
    const uint8_t* getData() const {
        return mParserData;
    } 
    int32_t getLength() const { 
        return mDataLength;
    }
    bool isValid() { return mDataLength > 0; }
protected:
    int32_t getSize() const {
        return mParserSize; 
    }
    const uint8_t* mParserData;
    int32_t mParserSize;
    int32_t mDataLength;
};

#endif
