#ifndef __DvbScan__H__
#define __DvbScan__H__

#include "DvbUtils.h"
#include "Frontend.h"
#include <stdint.h>
#include <iostream>
#include <string>

#define SCAN_EPG_NAME       "ScanEpg"
#define BLIND_SCAN_NAME     "BlindScan"
#define SCAN_CHANNEL_NAME   "ScanChannel"
#define SCAN_DATETIME_NAME  "ScanDateTime"
#define ANTENNA_SETUP_NAME  "AntennaSetup"
#define SCAN_OTA_NAME       "ScanOta"

class DvbManager;
class DvbScan : public BaseThread {
public:
    enum ScanState_e {
        eScanIdle = 0,
        eScanPat = 1,
        eScanCat = 2,
        eScanPmt = 3,
        eScanNit = 4,
        eScanSdt = 5,
        eScanEit = 6,
        eScanTdt = 7,
        eScanTot = 8,
        eScanDct = 9,
        eScanDdt = 10,
        eScanFinish  = 100,
        eScanCancel  = 101,
        eScanSuspend = 102,
    };

    enum ScanErrorCode_e {
        eSectionSuccess    = 0,
        eSectionIncomplete = -1,
        eSectionInvalid    = -2,
        eSectionSame       = -3,
        eSectionNotNeed    = -4,
    };

    virtual ~DvbScan();

    virtual int scanStart();
    virtual int scanStop();
    virtual int getProgress() = 0;
    virtual int parseSection(const uint8_t* data, uint16_t size);

    std::string getScanName() { return mScanName; }
    void setScanName(std::string name) { mScanName = name; }
    void show();
    void setState(ScanState_e state) { mScanState = state; }
    ScanState_e getState() { return mScanState; }
protected:
    DvbScan(std::string& name);
    virtual void Run(void* param) = 0;
    std::string mScanName;
    ScanState_e mScanState;
    DvbManager& mManager;
};
#endif
