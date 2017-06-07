#include "DvbAssertions.h"
#include "Transponder.h"
#include "Frontend.h"
#include "DvbUtils.h"
#include "DvbDevice.h"

Transponder::Transponder(DvbDevice* device) : mDvbDevice(device), mFrequency(0), mSymbolRate(0) 
{
}

Transponder::~Transponder()
{
}

int 
Transponder::getDvbDeviceKey()
{
    if (mDvbDevice)
        return mDvbDevice->mDeviceKey;
    return -1;
}

void 
TransponderDVBS::show()
{
    LogDvbDebug("TransponderDVBS--->\n"
        "\tUniqueKey: %d\tIFrequency: %d\tFrequency: %d\tSymbolRate: %d\tmPolarization: %s\tModulation: %s\tFecrate: %s\n", 
        mUniqueKey, mIFrequency, mFrequency, mSymbolRate, EnumToStr(mPolarization), EnumToStr(mModulation), EnumToStr(mFecRate));
}

uint32_t 
TransponderDVBS::IF2DF(uint32_t ifreq)
{
    DvbDeviceDVBS* device = static_cast<DvbDeviceDVBS*>(mDvbDevice);
    switch (device->mLnbType) {
        case eLnbManual   :
            if (eToneOff == device->mToneMode)
                return (device->mLOF1 >= 9750000) ? (device->mLOF1 + ifreq) : (device->mLOF1 - ifreq);
            else 
                return (device->mLOF2 >= 9750000) ? (device->mLOF2 + ifreq) : (device->mLOF2 - ifreq);
        case eLnbStandard : return eToneOff == device->mToneMode ? (5150000 - ifreq) : (5750000 - ifreq);
        case eLnbUniversal: return eToneOff == device->mToneMode ? (9750000 + ifreq) : (10600000 + ifreq);
    }
    return 0;
}

uint32_t 
TransponderDVBS::DF2IF(uint32_t dfreq)
{
    DvbDeviceDVBS* device = static_cast<DvbDeviceDVBS*>(mDvbDevice);
    switch (device->mLnbType) {
        case eLnbManual   :
            if (eToneOff == device->mToneMode)
                return (device->mLOF1 >= 9750000) ? (dfreq - device->mLOF1) : (device->mLOF1 - dfreq);
            else 
                return (device->mLOF2 >= 9750000) ? (dfreq - device->mLOF2) : (device->mLOF2 - dfreq);
        case eLnbStandard : return eToneOff == device->mToneMode ? (5150000 - dfreq) : (5750000 - dfreq);
        case eLnbUniversal: return eToneOff == device->mToneMode ? (dfreq - 9750000) : (dfreq - 10600000);
    }
    return 0;
}
