#ifndef __Transponder__H_
#define __Transponder__H_

#include "DvbEnum.h"
#include <stdint.h>

class DvbDevice;
class Transponder {
public:
    virtual ~Transponder();
    int mUniqueKey;
    int getDvbDeviceKey(); 
    virtual void show() = 0; 
    DvbDevice* mDvbDevice;
    uint32_t mFrequency;  //Unit:kHz (3400000 - 12750000)
    uint32_t mSymbolRate; //symbols per second
    uint32_t mBandWidth;
    Modulation_e mModulation;
    FecRate_e mFecRate;
protected:
    Transponder(DvbDevice* device);
};

class TransponderDVBC : public Transponder {
public:
    TransponderDVBC(DvbDevice* device) : Transponder(device) { }
    ~TransponderDVBC() { }
    void show() {}
};

class TransponderDVBS : public Transponder {
public:
    TransponderDVBS(DvbDevice* device) : Transponder(device), mIFrequency(0) { }
    ~TransponderDVBS() { }
    uint32_t mIFrequency; //Unit:kHz (950000 - 2150000)
    Polarization_e mPolarization;
    uint32_t IF2DF(uint32_t ifreq);
    uint32_t DF2IF(uint32_t dfreq);
    void show();
};

class TransponderDVBT : public Transponder {
public:
    TransponderDVBT(DvbDevice* device) : Transponder(device) { }
    ~TransponderDVBT() { }
    void show() { }
};

#endif
