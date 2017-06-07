#ifndef __DvbPatParser__H_
#define __DvbPatParser__H_

#include "DvbDataParser.h" 
#include "DvbDescriptor.h"
#include "DvbUtils.h"

class DvbPatElement : public DvbDataParser {
public:
    DvbPatElement(const uint8_t* data, int32_t size) { 
        _InitPatElement(data, size);
    }
    ~DvbPatElement() { 
    }
    void advance() {
        return _InitPatElement(getData() + getLength(), getSize() - getLength());
    }
    uint16_t getProgramNumber() {
        return (uint16_t)UtilsGetBits(getData(), 0, 0, 16);
    }
    uint16_t getProgramMapPid() {
        return (uint16_t)UtilsGetBits(getData(), 0, 19, 13);
    }
private:
    void _InitPatElement(const uint8_t* data, int32_t size);
};
    
class DvbSectionPAT;
class DvbPatParser : public DvbDataParser {
public: 
    DvbPatParser(DvbSectionPAT* section);
    ~DvbPatParser() {
    } 
    DvbPatElement elements() const; 
};

#endif
