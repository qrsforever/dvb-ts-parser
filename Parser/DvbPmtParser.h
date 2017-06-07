#ifndef __DvbPmtParser__H_
#define __DvbPmtParser__H_

#include "DvbDataParser.h" 
#include "DvbDescriptor.h"
#include "DvbUtils.h"

class DvbPmtElement : public DvbDataParser {
public:
    DvbPmtElement(const uint8_t* data, int32_t size) { 
        _InitPmtElement(data, size);
    }
    ~DvbPmtElement() { 
    }
    void advance() {
        return _InitPmtElement(getData() + getLength(), getSize() - getLength());
    }
    uint8_t getStreamType() const;
    uint16_t getElementaryPid() const;
    DvbDescriptor descriptors() const;
private:
    void _InitPmtElement(const uint8_t* data, int32_t size);
};

class DvbSectionPMT;
class DvbPmtParser : public DvbDataParser {
public: 
    DvbPmtParser(DvbSectionPMT* section);
    ~DvbPmtParser() {
    }
    uint16_t getPcrPid() const;
    DvbDescriptor descriptors() const;
    DvbPmtElement elements() const;
private:
    uint16_t mDescriptorLength;
};

#endif
