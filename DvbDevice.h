#ifndef __DvbDevice__H_
#define __DvbDevice__H_

#include "DvbEnum.h"
#include <string>
#include <stdint.h>

class DvbDevice {
public:
    virtual ~DvbDevice(){};
    int mDeviceKey;
    int mTunerId;
    virtual void show() = 0;
    virtual int getFavorFlag() { return -1; }
    virtual TransmissionType_e getType() = 0;
protected:
    DvbDevice() : mTunerId(1) {}
private:
    DvbDevice(const DvbDevice& device);
    DvbDevice& operator = (const DvbDevice& device);
};

class DvbDeviceDVBC : public DvbDevice {
public:
    DvbDeviceDVBC()  {}
    ~DvbDeviceDVBC() {}
    void show() { }
    TransmissionType_e getType() { return eTransmissionTypeDVBC; }
    //TODO
};

class DvbDeviceDVBS : public DvbDevice {
public :
    DvbDeviceDVBS() : mFavorFlag(1),
        mLnbType(eLnbUniversal), mLOF1(9750000), mLOF2(10600000),
        mToneMode(eToneOff), mDiseqcType(0), mDiseqcPort(eDiseqcPortA),
        mMotorType(0), mSatLongitude(""), mSateliteName("AutoSatellite") { }
    ~DvbDeviceDVBS() { }
    TransmissionType_e getType() { return eTransmissionTypeDVBS; }
/*
 *  LnbType: Standard(C Band)
 *
 *         +-----------------+         +-----------------+
 *         |3600 ~ 4200 MHz H|         |3600 ~ 4200 MHz V|
 *         +-----------------+         +-----------------+
 *                 |                            |
 *                 v                            v
 *          +------------+               +------------+
 *          |LO: 5150 MHz|               |LO: 5750 MHz|
 *          +------------+               +------------+
 *                 \__                        __/
 *                    \___              _____/
 *                        \_          _/
 *                          \        /
 *                 950 ~ 1550 MHz  1550 ~2150 HMz
 *
 *
 *  LnbType:   Universal(Ku Band)
 *
 *       +-------------------+           +-------------------+
 *       |10700~11700 MHz V/H|           |11700~12750 MHz V/H|
 *       +-------------------+           +-------------------+
 *                |                              |
 *                v                              v
 *          +-----+--------+              +-------------+
 *          | LO: 9750 MHz |              |LO: 10600 MHz|
 *          +--------------+              +-------------+
 *                  \__                         __/
 *                     \__                   __/
 *          22K off       \_               _/       22K on
 *                   950~1950 MHz      1100~2150 MHz
 *
 */
    int getFavorFlag() { return mFavorFlag; }
    int mFavorFlag; //0: all 1: search 2: xml preset sat
    LnbType_e mLnbType; //Manual, Standard(C), Universal(Ku)
    uint32_t mLOF1; //Local Oscillator Frequencies for low band      fix Standard: 5150  Universal:9750
    uint32_t mLOF2; //Local Oscillator Frequencies for hight band    fix Standard: 5750  Universal:10600
    /* 22K 开关: 
     *      0:不使用 22K 开关
     *      1:使用 0
     *      2:使用 22K */
    ToneMode_e mToneMode;
    /* DiSEqC 类型:
     *      0:不使用 DiSEqC 设备
     *      1:DiSEqC1.0
     *      2:DiSEqC1.1  */
    int mDiseqcType;
    /* 马达类型:
     *      0:不使用马达
     *      1:DiSEqC1.2
     *      2:USALS */
    DiseqcPort_e mDiseqcPort;
    int mMotorType;      
    std::string mSatLongitude; //15.5 --> 155
    std::string mSateliteName;
    void show();
};

class DvbDeviceDVBT : public DvbDevice {
public:
    TransmissionType_e getType() { return eTransmissionTypeDVBT; }
    DvbDeviceDVBT()  {}
    ~DvbDeviceDVBT() {}
    void show() { }
    //TODO
};

#endif
