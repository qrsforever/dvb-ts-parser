#ifndef __DvbNitParser__H_
#define __DvbNitParser__H_

#include "DvbDataParser.h" 
#include "DvbDescriptor.h"
#include "DvbUtils.h"

class DvbNitElement : public DvbDataParser {
public:
    DvbNitElement(const uint8_t* data, int32_t size) {
        _InitNitElement(data, size);
    }
    ~DvbNitElement() {
    }
    void advance() {
        _InitNitElement(getData() + getLength(), getSize() - getLength());
    }
    uint16_t getTransportStreamId() {
        return (uint16_t)UtilsGetBits(getData(), 0, 0, 16);
    }
    uint16_t getOriginalNetworkId() {
        return (uint16_t)UtilsGetBits(getData(), 0, 16, 16);
    }
    DvbDescriptor descriptors();
private:
    void _InitNitElement(const uint8_t* data, int32_t size);
};

class DvbSectionNIT;
class DvbNitParser : public DvbDataParser {
public: 
    DvbNitParser(DvbSectionNIT* section);
    ~DvbNitParser() {
    }
    DvbDescriptor descriptors() const;
    DvbNitElement elements() const;
private:
    uint16_t mDescriptorLength;
    uint16_t mElementsLength;
};

#endif
