#include "DvbAssertions.h"
#include "DvbBlindScan.h"
#include "FrontendDVBS.h"
#include "DvbDevice.h"
#include "DvbManager.h"
#include "Transponder.h"
#include "DvbScanEpg.h"
#include "DvbEnum.h"
#include <unistd.h>
#include "libzebra.h"

DvbBlindScan::DvbBlindScan(std::string name, std::vector<int> deviceKeys) : DvbScan(name), mDeviceKeys(deviceKeys),
    mStartFreq(950000), mStopFreq(2150000), mKeysIndex(0), mMinSymRate(3000), mMaxSymRate(40000)
{
    LogDvbDebug("contructor\n");
    mManager.delDvbScan(SCAN_EPG_NAME);
    mManager.delDvbScan(SCAN_OTA_NAME);
    //Delete Thransponders
    for (std::size_t i = 0; i < mDeviceKeys.size(); ++i) {
        std::vector<int> tTpsKeys = mManager.getTransponderKeys(mDeviceKeys[i]);
        for (std::size_t j = 0; j < tTpsKeys.size(); ++j)
            mManager.delTransponder(tTpsKeys[i]);
    }
}

DvbBlindScan::~DvbBlindScan()
{
    LogDvbDebug("destructor\n");
    if (mManager.getDvbChannelsCount() > 0) {
        /* DvbScanEpg* scan = new DvbScanEpg(SCAN_EPG_NAME, 1);
         * if (eDvbOk == mManager.addDvbScan(scan))
         *     scan->scanStart();
         * else 
         *     delete(scan); */
    }
    mDeviceKeys.clear();
    mTransponderKeys.clear();
}

int 
DvbBlindScan::scanStop()
{
    DTV_dvbs_TPscanStop(mScanCache._TunerId);
    DvbScan::scanStop();
    return 0;
}

int 
DvbBlindScan::getProgress() 
{ 
    if (getState() == eScanFinish || !mDeviceKeys.size())
        return 100;

    return ((mKeysIndex * 100 + mScanCache._Progress) / mDeviceKeys.size()) / 1.02;
}

void 
DvbBlindScan::setScanRange(int freqBegin, int freqEnd, int symbolBegin, int symbolEnd)
{
    LogDvbDebug("freqBegin[%d] freqEnd[%d] symbolBegin[%d] symbolEnd[%d]\n", freqBegin, freqEnd, symbolBegin, symbolEnd);
    mStartFreq = freqBegin;
    mStopFreq = freqEnd;
    mMinSymRate = symbolBegin;
    mMaxSymRate = symbolEnd;
}

void 
DvbBlindScan::blindScan()
{
    if (!isRun())
        return ;
    FrontendDVBS* frontend = static_cast<FrontendDVBS*>(mManager.getFrontend());
    //Set DiseqcPort
    frontend->setDiseqcPort(mScanCache._TunerId, mScanCache._DiseqcPort);
    //Set ToneMode
    frontend->setToneMode(mScanCache._TunerId, mScanCache._ToneMode);
    //Set LnbPower
    frontend->setLnbPower(mScanCache._TunerId, mScanCache._LnbPower); //no usefull now

    LogDvbDebug("TPscanStart: tunerId[%d] startFreq[%d] endFreq[%d] MinSym[%d] MaxSym[%d]\n", mScanCache._TunerId, mStartFreq, mStopFreq, mMinSymRate, mMaxSymRate);
    DTV_dvbs_TPscanStart(mScanCache._TunerId, mStartFreq, mStopFreq, mMinSymRate, mMaxSymRate);
    while (isRun()) {
        usleep(500000);
        mScanCache._Progress = DTV_dvbs_TPscanGetProgress(mScanCache._TunerId);
        if (100 == mScanCache._Progress) {
            mScanCache._DvbsTPs.clear();
            int resCount = 0;
            DTV_DVBS_TP tpsArray[2*BLIND_SCAN_CHANNEL_TOTAL];
            DTV_dvbs_TPscanGetTPlist(mScanCache._TunerId, tpsArray, 2*BLIND_SCAN_CHANNEL_TOTAL, &resCount);
            for (int j = 0; j < resCount; ++j) {
                struct DvbsTP tp;
                tp.nIFrequency   = tpsArray[j].freq * 10;
                tp.nSymbolRate   = tpsArray[j].symborate;
                tp.nPolarization = static_cast<Polarization_e>(tpsArray[j].polarity);
                tp.nUniqueKey    = UtilsCreateKey(tpsArray[j].freq * 10, tpsArray[j].symborate, tpsArray[j].polarity, 0);
                if (!mManager.getTransponder(tp.nUniqueKey))
                    mScanCache._DvbsTPs.push_back(tp);
            }
            break;
        }
    }
}

void 
DvbBlindScan::Run(void* threadID)
{
    LogDvbDebug("DvbBlindScan thread is running\n");
    setState(eScanIdle);
    for (mKeysIndex = 0; isRun() && mKeysIndex < (int)mDeviceKeys.size(); ++mKeysIndex) {
        DvbDeviceDVBS* device  = static_cast<DvbDeviceDVBS*>(mManager.getDvbDevice(mDeviceKeys[mKeysIndex]));
        if (!device)
            continue;
        mScanCache._TunerId    = device->mTunerId;
        mScanCache._DiseqcPort = device->mDiseqcPort;
        mScanCache._ToneMode   = device->mToneMode;
        mScanCache._Progress   = 0;

        blindScan();
        for (std::size_t j = 0; j < mScanCache._DvbsTPs.size(); ++j) {
            TransponderDVBS* transponder = new TransponderDVBS(device);
            transponder->mIFrequency   = mScanCache._DvbsTPs[j].nIFrequency;
            transponder->mFrequency    = transponder->IF2DF(transponder->mIFrequency);
            transponder->mSymbolRate   = mScanCache._DvbsTPs[j].nSymbolRate;
            transponder->mPolarization = mScanCache._DvbsTPs[j].nPolarization;
            transponder->mUniqueKey    = mScanCache._DvbsTPs[j].nUniqueKey;
            if (eDvbOk != mManager.addTransponder(transponder)) {
                delete(transponder);
                continue;
            }
            mTransponderKeys.push_back(transponder->mUniqueKey);
        }
    }
    if (mTransponderKeys.size() > 0)
        mManager.saveDB(eTransponderDB | eDvbChannelDB);
    setState(eScanFinish);
    LogDvbDebug("DvbBlindScan thread is finished\n");
}
