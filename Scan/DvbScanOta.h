#ifndef __DvbScanOta__H_
#define __DvbScanOta__H_

#include "DvbScan.h"
#include "Transponder.h"
#include "UpgradeManager.h"

#include <stdint.h>
#include <iostream>
#include <vector>
#include <string>

class DvbScanOta : public DvbScan {
public:
    DvbScanOta(std::string name, Hippo::UpgradeManager* upgradeManager);
    ~DvbScanOta();
    typedef struct DvbOtaCmd_ {
        uint8_t  nCmdType;
        uint8_t  nTableId;
        uint16_t nPsiPid;
        uint16_t nTimeout;
    }DvbOtaCmd_s;
    enum DvbOtaCmdType_ {
       eFilterNIT = 1, 
       eFilterPAT = 2,
       eFilterPMT = 3,
       eFilterDCT = 4,
       eFilterDDT = 5,
    };
    virtual int parseSection(const uint8_t* data, uint16_t size);
    virtual int getProgress() { return mCurrSectionCnt * 100 / (mLastSectionNum + 1); }

    bool startCheck(int timeout = 0);
    bool startReceive(int timeout = 0);
    void taskSuspend();

    typedef struct DvbOtaSource_ {
        uint16_t nTransportStreamId;
        uint16_t nOriginalNetworkId;
        uint16_t nServiceId;
        uint8_t  nUpgradeType;
        uint32_t nSoftVersion;
        uint32_t nFrequency;
        uint8_t  nPolarization; 
        uint32_t nSymbolRate;
        uint16_t nElementaryPid;
    }DvbOtaSource_t;

    DvbOtaSource_t& getDvbOtaSource() { return mUpgradeSource; }

    class OtaDataBlock {
    public:
        OtaDataBlock(const uint8_t* data, uint32_t size) {
            mSize = size;
            if (data && size > 0) {
                mData = new unsigned char[size];
                memcpy(mData, data, size);
            }
        }
        uint8_t* getData() { return mData; }
        uint32_t getSize() { return mSize; }
        ~OtaDataBlock() {
            if (mData)
                delete []mData;
        }
        uint8_t* mData;
        uint32_t mSize;
    };
    std::vector<OtaDataBlock*> mDataBlocks;
private:
    bool _SendCmd(uint8_t cmd, uint8_t tid, uint16_t pid, uint16_t timeout); 
    int _ParseNIT(const uint8_t* data, uint16_t size);
    int _ParsePAT(const uint8_t* data, uint16_t size);
    int _ParsePMT(const uint8_t* data, uint16_t size);
    int _ParseDCT(const uint8_t* data, uint16_t size);
    int _ParseDDT(const uint8_t* data, uint16_t size);
    int mPipeFds[2];
    int mTunerId;
protected:
    virtual void Run(void* threadID);
    uint32_t mClientOui;
    uint64_t mHardwareVersion;
    uint32_t mSoftwareVersion;
    uint16_t mLastSectionNum;
    uint16_t mCurrSectionCnt;
    DvbOtaSource_t mUpgradeSource;
    Hippo::UpgradeManager* mUpgradeManager;
};
#endif
