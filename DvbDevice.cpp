#include "DvbAssertions.h"
#include "DvbDevice.h"

void
DvbDeviceDVBS::show()
{
    LogDvbDebug("DvbDeviceDVBS--->\nDeviceKey[%d] SatName:%s Longitude:%d LnbType:%d LOF1:%d LOGF2:%d ToneMode[%d] DiseqcType[%d] DiseqcPort[%d] MotorType[%d], Fav[%d]\n",
        mDeviceKey, mSateliteName.c_str(), mSatLongitude.c_str(), mLnbType, mLOF1, mLOF2, mToneMode, mDiseqcType, mDiseqcPort, mMotorType, mFavorFlag);
}
