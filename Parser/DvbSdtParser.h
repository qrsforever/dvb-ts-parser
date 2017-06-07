#ifndef __DvbSdtParser__H_
#define __DvbSdtParser__H_

#include "DvbDataParser.h" 
#include "DvbDescriptor.h"
#include "DvbUtils.h"

class DvbSdtElement : public DvbDataParser {
public:
    DvbSdtElement(const uint8_t* data, int32_t size) { 
        _InitSdtElement(data, size);
    }
    ~DvbSdtElement() { 
    }
    void advance() {
        return _InitSdtElement(getData() + getLength(), getSize() - getLength());
    }
    uint16_t getServiceId() {
        return (uint16_t)UtilsGetBits(getData(), 0, 0, 16);
    }
    bool getScrambled() {
        return (bool)UtilsGetBits(getData(), 0, 27, 1); 
    }

    DvbDescriptor descriptors() const;
private:
    void _InitSdtElement(const uint8_t* data, int32_t size);
};

class DvbSectionSDT;
class DvbSdtParser : public DvbDataParser {
public: 
    DvbSdtParser(DvbSectionSDT* section);
    ~DvbSdtParser() {
    }
    uint16_t getOriginalNetworkId() {
        return (uint16_t)UtilsGetBits(getData(), 0, 0, 16);
    }
    DvbSdtElement elements() const;
};

#endif
