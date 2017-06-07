#include "DvbAssertions.h"
#include "DvbAntennaSetup.h"
#include "DvbBlindScan.h"
#include "DvbDevice.h"
#include "DvbSection.h"
#include "DvbDemuxFilter.h"
#include "DvbManager.h"
#include "DvbScanEpg.h"
#include "Transponder.h"
#include "DvbNitParser.h"
#include "DvbSatelliteDescriptor.h"

DvbAntennaSetup::DvbAntennaSetup(std::string name, int tunerCount, int setupType, int searchMode, bool isSearchTP) : 
    DvbBlindScan(name), mSetupType(setupType), mSearchMode(searchMode), mIsSearchTP(isSearchTP),
    mTunerCount(tunerCount), mPortCount(1), mSatLongitude("") 
{
    LogDvbDebug("contructor\n");
    mManager.delDvbScan(SCAN_EPG_NAME);
    mManager.delDvbScan(SCAN_DATETIME_NAME);
    mManager.delDvbScan(SCAN_OTA_NAME);
    if (!mIsSearchTP)
        setScanRange(950, 1650, 10, 35);
    if (1 == mSetupType)
        mPortCount = 4;
    mTunerIndex = 0;
    mPortIndex = 0;

    //Delete Devices
    std::vector<int> tDevKeys = mManager.getDvbDeviceKeys(1);
    for (std::size_t i = 0; i < tDevKeys.size(); ++i)
        mManager.delDvbDevice(tDevKeys[i]);
}                        

DvbAntennaSetup::~DvbAntennaSetup()
{
    LogDvbDebug("destructor\n");
    mNitSections.clear();
}

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
 *  LnbType: Universal(Ku Band)
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
 *           22K off      \_               _/        22k on
 *                   950~1950 MHz      1100~2150 MHz
 */

int 
DvbAntennaSetup::_ParsePAT(const uint8_t* data, uint16_t size)
{
    LogDvbDebug("data = %p, size = %d\n", data, size);
    DvbSectionPAT section(data, size);
    if (!section.isValid())
        return eSectionInvalid;
    mTransportStreamId = section.getTransportStreamId();
    return eSectionSuccess;
}

int 
DvbAntennaSetup::_ParseNIT(const uint8_t* data, uint16_t size)
{
    LogDvbDebug("data = %p, size = %d\n", data, size);
    DvbSectionNIT section(data, size);
    if (!section.isValid())
        return eSectionInvalid;

    if (DvbSectionNIT::eActualTableID != section.getTableId())
        return eSectionNotNeed;

    if (mNitSections.empty())
        mNitSections.resize(section.getLastSectionNumber() + 1, false);
    if (mNitSections.at(section.getSectionNumber()))
        return eSectionSame;
    
    DvbNitParser parser(&section);
    for (DvbNitElement element = parser.elements(); isRun() && element.isValid(); element.advance()) {
        if (mTransportStreamId != element.getTransportStreamId())
            continue;
        for(DvbDescriptor descriptor = element.descriptors(); isRun() && descriptor.isValid(); descriptor.advance()) {
            if (0x43 == descriptor.tag()) {//0x43: satellite_delivery_system
                DvbSatelliteDescriptor satelliteDescriptor(descriptor);
                int loc = satelliteDescriptor.getOrbitalPostion();
                if (!satelliteDescriptor.getWestEastFlag())
                    loc *= -1;
                mSatLongitude = UtilsInt2Str(loc);
                return eSectionSuccess;
            }
        }
    }

    mNitSections[section.getSectionNumber()] = true;

    for (std::size_t i = 0; i < mNitSections.size(); ++i) {
        if (!mNitSections.at(i))
            return eSectionIncomplete;
    }
    return eSectionSuccess;
}

int 
DvbAntennaSetup::parseSection(const uint8_t* data, uint16_t size)
{
    LogDvbDebug("ScanState[%d]\n", mScanState); 
    if (!isRun())
        return eSectionSuccess;
    switch (getState()) {
        case eScanPat : return _ParsePAT(data, size);
        case eScanNit : return _ParseNIT(data, size);
        default:
            ;
    }
    return eSectionNotNeed;
}

int 
DvbAntennaSetup::getProgress()
{
    if (getState() == eScanFinish)
        return 100;

    if (eToneOn == mScanCache._ToneMode)
        return ((mTunerIndex * 100 + (mPortIndex * 100 + mScanCache._Progress / 2) / mPortCount) / mTunerCount) / 1.01; //22K On
    return ((mTunerIndex * 100 + (mPortIndex * 100 + 50 + mScanCache._Progress / 2) / mPortCount) / mTunerCount) / 1.01; //22K Off
}

void 
DvbAntennaSetup::_ScanOrbitalPostion(TransponderDVBS* transponder) 
{
    DvbDemuxFilter* filter = new DvbDemuxFilter(this);
    if (!isRun() || !filter)
        return;
    filter->setParserBand(mManager.getFrontend()->getParserBand(transponder->mDvbDevice->mTunerId));
    if (mManager.getFrontend()->tune(transponder->mDvbDevice->mTunerId, transponder)) {
        setState(eScanPat);
        filter->setPsisiPid(ePatPid);
        filter->setTimeout(1500);
        filter->start(); //block

        setState(eScanNit);
        filter->setPsisiPid(eNitPid);
        filter->setTimeout(1500);
        filter->start(); //block
    }
    mNitSections.clear();
    delete(filter);
}

int
DvbAntennaSetup::autoScan()
{
    blindScan();
    if (isRun() && mScanCache._DvbsTPs.size() > 0) {
        DvbDeviceDVBS* device = new DvbDeviceDVBS();
        if (!device)
            return eDvbError;
        device->mFavorFlag  = 1;
        device->mLnbType    = eLnbUniversal;
        device->mLOF1       = 9750000;
        device->mLOF2       = 10600000;
        device->mTunerId    = mScanCache._TunerId;
        device->mDiseqcType = mSetupType;
        device->mDiseqcPort = mScanCache._DiseqcPort;
        device->mToneMode   = mScanCache._ToneMode;
        device->mSateliteName.append(EnumToStr(device->mDiseqcPort));
        device->mSateliteName.append(EnumToStr(device->mToneMode));
        device->mDeviceKey  = UtilsCreateKey(mScanCache._TunerId, mScanCache._DiseqcPort, mScanCache._ToneMode, 0);
        if (eDvbOk != mManager.addDvbDevice(device)) {
            delete(device);
            return eDvbError;
        }
        mDeviceKeys.push_back(device->mDeviceKey);

        if (!mIsSearchTP)
            return eDvbOk;

        for (std::size_t i = 0; isRun() && i < mScanCache._DvbsTPs.size(); ++i) {
            TransponderDVBS* transponder = new TransponderDVBS(device);
            if (!transponder) {
                mDeviceKeys.pop_back();
                mManager.delDvbDevice(device->mDeviceKey);
                return eDvbError;
            }
            transponder->mIFrequency   = mScanCache._DvbsTPs[i].nIFrequency;
            transponder->mFrequency    = transponder->IF2DF(transponder->mIFrequency);
            transponder->mSymbolRate   = mScanCache._DvbsTPs[i].nSymbolRate;
            transponder->mPolarization = mScanCache._DvbsTPs[i].nPolarization;
            transponder->mUniqueKey    = mScanCache._DvbsTPs[i].nUniqueKey;
            if (eDvbOk != mManager.addTransponder(transponder))
                delete(transponder);
            if (0 == mSatLongitude.size() && i < 3) //try 3 times
                _ScanOrbitalPostion(transponder);
        }
        if (mSatLongitude.size() > 0) {
            DvbDeviceDVBS* dev = 0;
            std::vector<int> tDevKeys = mManager.getDvbDeviceKeys(0);
            for (std::size_t i = 0; i < tDevKeys.size(); ++i) {
                dev = static_cast<DvbDeviceDVBS*>(mManager.getDvbDevice(tDevKeys[i]));
                if (dev && !dev->mFavorFlag && dev->mSatLongitude == mSatLongitude) {
                    device->mSateliteName.assign(dev->mSateliteName);
                    device->mSateliteName.append(EnumToStr(device->mDiseqcPort));
                    device->mSateliteName.append(EnumToStr(device->mToneMode));
                }
            }
            device->mSatLongitude = mSatLongitude;
        }
    }
    return eDvbOk;
}

void 
DvbAntennaSetup::Run(void* param)
{
    LogDvbDebug("DvbAntennaSetup thread is running\n");
    setState(eScanIdle);
    for (int i = 0; isRun() && i < mTunerCount; ++i) {
        mTunerIndex = i;
        mScanCache._TunerId = mTunerIndex + 1; //TODO
        for (int j = 0; isRun() && j < mPortCount; ++j) {
            mPortIndex = j;
            mScanCache._DiseqcPort = static_cast<DiseqcPort_e>(mPortIndex);
            mSatLongitude.clear();
            //22K On
            mScanCache._ToneMode = eToneOn;
            mScanCache._Progress = 0;
            autoScan();

            //22K Off
            mScanCache._ToneMode = eToneOff;
            mScanCache._Progress = 0;
            autoScan();
        }
    }
    LogDvbDebug("Setup ok, now save to database\n");
    if (mDeviceKeys.size() > 0)
        mManager.saveDB(eDvbDeviceDB | eTransponderDB);
    setState(eScanFinish);
    LogDvbDebug("DvbAntennaSetup thread is finished\n");
}
