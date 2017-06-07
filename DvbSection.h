#ifndef __DVBSECTION__H_
#define __DVBSECTION__H_

#include <stdint.h>

const int kSectionHeadLength = 8;
const int kSectionCrcLength = 4;
class DvbSection {
public:
    const uint8_t getTableId() const { return mTableId; }
    const uint8_t getVersionNumber() const { return mVersionNumber; }
    const uint8_t getSectionNumber() const { return mSectionNumber; }
    const uint8_t getLastSectionNumber() const { return mLastSectionNumber; }
    const int16_t getSectionLength() const { return mSectionLength; }
    const uint8_t* getData() const { return mSourceData; }
    const int32_t getSize() const { return mSourceSize; }
    virtual bool isValid() = 0;
protected: 
    DvbSection(const uint8_t* data, uint16_t size);
    ~DvbSection();
    union {
        uint16_t mDataId;
        uint16_t mBouquetId; //BAT
        uint16_t mNetworkId; //NIT
        uint16_t mServiceId; //EIT
        uint16_t mProgramNumber; //PMT
        uint16_t mTransportStreamId; //PAT and SDT
    }mUnion;
    uint8_t  mVersionNumber;
    uint8_t  mSectionNumber;   
    uint8_t  mLastSectionNumber;

    bool _CheckCrc();
    void _DecodeHeader();
    bool isStandardSection() { return (mSourceData[1] & 0x80) != 0; }

    const uint8_t* mSourceData;
    const int32_t  mSourceSize;
    int16_t mSectionLength;
    uint8_t mTableId;
};

class DvbSectionPAT : public DvbSection {
public: 
    enum { eActualTableID = 0x00 };
    DvbSectionPAT(const uint8_t* data, uint16_t size) : DvbSection(data, size) {
    }
    ~DvbSectionPAT() {
    }
    bool isValid() { 
        return eActualTableID == mTableId; 
    }
    uint16_t getTransportStreamId() const { 
        return mUnion.mTransportStreamId; 
    }
};

class DvbSectionPMT : public DvbSection {
public: 
    enum { eActualTableID = 0x02 };
    DvbSectionPMT(const uint8_t* data, uint16_t size) : DvbSection(data, size) {
    }
    ~DvbSectionPMT() {
    }
    bool isValid() { 
        return eActualTableID == mTableId; 
    }
    uint16_t getServiceId() const {
        return mUnion.mServiceId; 
    }
};

class DvbSectionSDT : public DvbSection {
public: 
    enum {
        eActualTableID = 0x42,
        eOtherTableID  = 0x46
    };
    DvbSectionSDT(const uint8_t* data, uint16_t size) : DvbSection(data, size) {
    }
    ~DvbSectionSDT() {
    }
    bool isValid() { 
        return (eActualTableID == mTableId || eOtherTableID == mTableId); 
    }
    uint16_t getTransportStreamId() const { 
        return mUnion.mTransportStreamId; 
    }
};

class DvbSectionNIT : public DvbSection {
public: 
    enum {
        eActualTableID = 0x40,
        eOtherTableID  = 0x41,
    };
    DvbSectionNIT(const uint8_t* data, uint16_t size) : DvbSection(data, size) {
    }
    ~DvbSectionNIT() { 
    }
    bool isValid() { 
        return (eActualTableID == mTableId ) || (eOtherTableID == mTableId); 
    }
    uint16_t getNetworkId() const {
        return mUnion.mNetworkId;
    }
};

class DvbSectionEIT : public DvbSection {
public: 
    enum {
        eActualTableID = 0x4e,
        eOtherTableID  = 0x4f,
        eMaxEITTableID = 0x6F
    };
    DvbSectionEIT(const uint8_t* data, uint16_t size) : DvbSection(data, size) {
    }
    ~DvbSectionEIT() {
    }
    bool isValid() { 
        return (eActualTableID <= mTableId && mTableId <= eMaxEITTableID); 
    }
    uint16_t getServiceId() const { 
        return mUnion.mServiceId; 
    } 
};

class DvbSectionTDT : public DvbSection {
public:
    enum { eActualTableID = 0x70 };
    DvbSectionTDT(const uint8_t* data, uint16_t size) : DvbSection(data, size) {
    }
    ~DvbSectionTDT() {
    }
    bool isValid() { 
        return eActualTableID == mTableId;
    }
    uint64_t getUtcTime() const; 
};

class DvbSectionTOT : public DvbSection {
public:
    enum { eActualTableID = 0x73 };
    DvbSectionTOT(const uint8_t* data, uint16_t size) : DvbSection(data, size) {
    }
    ~DvbSectionTOT() {
    }
    bool isValid() { 
        return  eActualTableID == mTableId; 
    }
    uint64_t getUtcTime() const;
};
#endif
