#ifndef __DvbAntennaSetup__H_
#define __DvbAntennaSetup__H_

#include "DvbBlindScan.h"
#include <stdint.h>

class DvbBlindScan;
class DvbManager;
class DvbDevice;
class DvbDeviceDVBS;
class TransponderDVBS;
class DvbAntennaSetup : public DvbBlindScan {
public:
    DvbAntennaSetup(std::string name, int tunerCount = 1, int setupType = 0, int searchMode = 0, bool isSearchTP = true);
    ~DvbAntennaSetup();
    virtual int getProgress();
    virtual int parseSection(const uint8_t* data, uint16_t size);
    DiseqcPort_e getCurrentPort() const { return mScanCache._DiseqcPort; }
    int getSetupType() const { return mSetupType; }
protected:
    int autoScan();
    virtual void Run(void* threadID);
private:
    int _ParsePAT(const uint8_t* data, uint16_t size);
    int _ParseNIT(const uint8_t* data, uint16_t size);
    void _ScanOrbitalPostion(TransponderDVBS* transponder);
    /* DiSEqC 类型:
     *     0:不使用 DiSEqC 设备
     *     1:DiSEqC1.0
     *     2:DiSEqC1.1  */
    int mSetupType;
    int mSearchMode;
    bool mIsSearchTP;
    int mTunerCount;
    int mTunerIndex;
    int mPortIndex;
    int mPortCount;
    std::vector<bool> mNitSections;
    std::string mSatLongitude;
    uint16_t mTransportStreamId;
};

#endif
