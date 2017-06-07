#ifndef __DvbTimeOffsetDescriptor__H_
#define __DvbTimeOffsetDescriptor__H_

#include "DvbDescriptor.h"
#include <iostream>
#include <string>
#include <map>

class DvbTimeOffsetDescriptor : public DvbDescriptor {
public:
    DvbTimeOffsetDescriptor(const DvbDescriptor& descriptor);
    ~DvbTimeOffsetDescriptor() {
        mTimeOffsets.clear();
    }
    typedef struct _TimeOffsetInfo_ {
        uint8_t  nCountryRegionId;
        uint8_t  nOffsetPolarity;
        uint16_t nTimeOffset;     //BCD: 0230 - 02:30
        uint64_t nTimeChange;
        uint16_t nNextOffset;     //BCD: 0130 - 01:30
    }TimeOffsetInfo_t;

    std::map<std::string, TimeOffsetInfo_t> getTimeOffsets() {
        return mTimeOffsets;
    }
    int getOffsetPolar(const char* code) const;
    int getTimeOffset(const char* code) const;
    int getNextOffset(const char* code) const;
    uint64_t getTimeChange(const char* code) const;
private:
    std::map<std::string, TimeOffsetInfo_t> mTimeOffsets;
};

#endif
