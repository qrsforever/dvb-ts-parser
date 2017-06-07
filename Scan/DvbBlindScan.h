#ifndef __DvbBlindScan__H_
#define __DvbBlindScan__H_

#include "DvbScan.h"
#include <iostream>
#include <vector>
#include <string>

class DvbBlindScan : public DvbScan {
public:
    DvbBlindScan(std::string name, std::vector<int> deviceKeys);
    ~DvbBlindScan();

    struct DvbsTP {
        int nUniqueKey;
        uint32_t nIFrequency;  //Unit:kHz
        uint32_t nSymbolRate; //symbols per second
        Polarization_e nPolarization;
    };
    class BlindScanCache {
    public:
        BlindScanCache() : _Progress(0), _TunerId(0), _DiseqcPort(eDiseqcPortA), _ToneMode(eToneOff) { 
        }
        int _Progress;
        int _TunerId;
        DiseqcPort_e _DiseqcPort;
        ToneMode_e _ToneMode;
        LnbPower_e _LnbPower;
        std::vector<struct DvbsTP> _DvbsTPs;
    };

    virtual int getProgress();
    int scanStop();
    std::vector<int> getDeviceKeys() { return mDeviceKeys; }
    std::vector<int> getTransponderKeys() { return mTransponderKeys; }
protected:
    DvbBlindScan(std::string name) : DvbScan(name), mStartFreq(950000), mStopFreq(1150000), mMinSymRate(20000), mMaxSymRate(40000) {
    }
    virtual void Run(void* threadID);
    void setScanRange(int freqBegin, int freqEnd, int symbolBegin, int symbolEnd);
    void blindScan();
    BlindScanCache mScanCache;
    std::vector<int> mDeviceKeys;
    std::vector<int> mTransponderKeys;
private:
    int mStartFreq;
    int mStopFreq;
    int mKeysIndex;
    int mMinSymRate;
    int mMaxSymRate;
};

#endif
