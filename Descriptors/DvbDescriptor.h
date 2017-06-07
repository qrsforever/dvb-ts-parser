#ifndef __DvbDescriptor__H_
#define __DvbDescriptor__H_ 

#include "DvbDataParser.h"

class DvbDescriptor : public DvbDataParser {
public:
    DvbDescriptor(const uint8_t* data, int32_t size) {
        _InitDescriptor(data, size);
    }
    ~DvbDescriptor() {
    }
    void advance() {
        _InitDescriptor(getData() + getLength(), getSize() - getLength());
    }
    int tag() const; 
private:
    void _InitDescriptor(const uint8_t* data, int32_t size);
};

#endif
