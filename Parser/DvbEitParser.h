#ifndef __DvbEitParser__H_
#define __DvbEitParser__H_ 

#include "DvbDataParser.h" 
#include "DvbDescriptor.h"
#include "DvbUtils.h"

class DvbEitElement : public DvbDataParser {
public:
    DvbEitElement(const uint8_t* data, int32_t size) {
        _InitEitElement(data, size);
    }
    ~DvbEitElement() {
    }
    void advance() {
        _InitEitElement(getData() + getLength(), getSize() - getLength());
    }
    uint16_t getEventId() {
        return (uint16_t)UtilsGetBits(getData(), 0, 0, 16);
    }
    uint64_t getStartTime() const;
    uint64_t getDuration() const;
    uint8_t getRunStatus() {
        return (uint8_t)UtilsGetBits(getData(), 0, 80, 3);
    }
    bool getScrambled() {
        return (bool)UtilsGetBits(getData(), 0, 83, 1);
    }
    DvbDescriptor descriptors() const;
private:
    void _InitEitElement(const uint8_t* data, int32_t size);
};

class DvbSectionEIT;
class DvbEitParser : public DvbDataParser {
public: 
    DvbEitParser(DvbSectionEIT* section);
    ~DvbEitParser() {
    }
    uint16_t getTransportStreamId() {
        return (uint16_t)UtilsGetBits(getData(), 0, 0, 16);
    }
    uint16_t getOriginalNetworkId() {
        return (uint16_t)UtilsGetBits(getData(), 0, 16, 16);
    }
    DvbEitElement elements() const;
};

#endif
