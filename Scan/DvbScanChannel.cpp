#include "DvbAssertions.h"
#include "DvbScanChannel.h"
#include "DvbChannel.h"
#include "DvbScanEpg.h"
#include "DvbSection.h"
#include "DvbPatParser.h"
#include "DvbSdtParser.h"
#include "DvbDemuxFilter.h"
#include "DvbServiceDescriptor.h"
#include "DvbDevice.h"
#include "DvbManager.h"

#include "libzebra.h"

DvbScanChannel::DvbScanChannel(std::string name, std::vector<int>& transKeys, bool searchNit, ChannelType_e channelType, ServiceType_e serviceType) : 
    DvbScan(name), mChannelType(channelType), mServiceType(serviceType), mSearchNetwork(searchNit), 
    mTransponderKeys(transKeys), mVideoCount(0), mAudioCount(0), mKeysIndex(0)
{
    LogDvbDebug("contructor\n");
    mManager.delDvbScan(SCAN_EPG_NAME);
    mManager.delDvbScan(SCAN_OTA_NAME);
    //Delete Channels
    for (std::size_t i = 0; i < mTransponderKeys.size(); ++i) {
        std::vector<int> tChlKeys = mManager.getDvbChannelKeys(eServiceAll, mTransponderKeys[i]);
        for (std::size_t j = 0; j < tChlKeys.size(); ++j)
            mManager.delDvbChannel(tChlKeys[i]);
    }
}

DvbScanChannel::~DvbScanChannel()
{
    LogDvbDebug("destructor\n");
    /* DvbScanEpg* scan = new DvbScanEpg(SCAN_EPG_NAME, 1);
     * if (eDvbOk == mManager.addDvbScan(scan))
     *     scan->scanStart();
     * else 
     *     delete(scan); */
    mTransponderKeys.clear();
    mProgramMapPids.clear();
    mScanChannelsCache.clear();
}

int 
DvbScanChannel::getProgress() 
{ 
    if (eScanFinish == mScanState || !mTransponderKeys.size())
        return 100;
    return (mKeysIndex * 100 / mTransponderKeys.size()) / 1.02;
}

int 
DvbScanChannel::getChnlsCount(ServiceType_e type)
{
    switch (type) {
        case eServiceAll  : return mVideoCount + mAudioCount;
        case eServiceTv   : return mVideoCount;
        case eServiceRadio: return mAudioCount;
        default:
            ;
    }
    return 0;
}

int 
DvbScanChannel::_ParsePAT(const uint8_t* data, uint16_t size)
{
    LogDvbDebug("data = %p, size = %d\n", data, size);

    DvbSectionPAT section(data, size);
    if (!section.isValid())
        return eSectionInvalid;

    if (mPatSections.empty())
        mPatSections.resize(section.getLastSectionNumber() + 1, false);
    if (mPatSections.at(section.getSectionNumber()))
        return eSectionSame;

    DvbPatParser parser(&section);
    for (DvbPatElement element = parser.elements(); element.isValid(); element.advance()) {
        //LogDvbDebug("ProgramNumber[%d]\n", element.getProgramNumber());
        if (0 != element.getProgramNumber())
            mProgramMapPids[element.getProgramNumber()] = element.getProgramMapPid();
    }

    mPatSections[section.getSectionNumber()] = true;

    for (std::size_t i = 0; i < mPatSections.size(); ++i) {
        if (!mPatSections.at(i))
            return eSectionIncomplete;
    }
    return eSectionSuccess;
}

int 
DvbScanChannel::_ParseSDT(const uint8_t* data, uint16_t size)
{
    LogDvbDebug("data = %p, size = %d\n", data, size);

    DvbSectionSDT section(data, size);
    if (!section.isValid())
        return eSectionInvalid;

    if (mSdtSections.empty())
        mSdtSections.resize(section.getLastSectionNumber() + 1, false);
    if (mSdtSections.at(section.getSectionNumber()))
        return eSectionSame;

    DvbSdtParser parser(&section);
    for (DvbSdtElement element = parser.elements(); element.isValid(); element.advance()) {
        if (mProgramMapPids.end() == mProgramMapPids.find(element.getServiceId()))
            continue;
        //LogDvbDebug("ServiceId[%d]\n", element.getServiceId());
        ScanChannelCache cache(element.getServiceId(), mProgramMapPids[element.getServiceId()]);
        cache._TransportStreamId = section.getTransportStreamId();
        cache._OriginalNetworkId = parser.getOriginalNetworkId();
        cache._ChannelType = element.getScrambled() ? eChannelCa : eChannelFta;
        for(DvbDescriptor descriptor = element.descriptors(); descriptor.isValid(); descriptor.advance()) {
            if (0x48 == descriptor.tag()) {//0x48:service descriptor
                DvbServiceDescriptor serviceDescriptor(descriptor);
                switch (serviceDescriptor.getServiceType()) {
                    case 0x02:  /* digital radio sound service */
                    case 0x03:  /* Teletext service */
                    case 0x04:  /* NVOD reference service  */
                    case 0x05:  /* NVOD time-shifted service */
                    case 0x06:  /* mosaic service */
                    case 0x07:  /* FM radio service */
                    case 0x0A:  /* advanced codec digital radio sound service */
                        cache._ServiceType = eServiceRadio;
                        break;
                    default:
                        cache._ServiceType = eServiceTv;
                        break;
                }
                cache._ProviderName = serviceDescriptor.getProviderName();
                cache._ServiceName  = serviceDescriptor.getServiceName();
            }
        }
        //filter 
        if (mChannelType != eChannelAll && mChannelType != cache._ChannelType)
            continue;
        if (mServiceType != eServiceAll && mServiceType != cache._ServiceType)
            continue;
        mScanChannelsCache.push_back(cache);
    }

    mSdtSections[section.getSectionNumber()] = true;

    for (std::size_t i = 0; i < mSdtSections.size(); ++i) {
        if (!mSdtSections.at(i))
            return eSectionIncomplete;
    }
    return eSectionSuccess;
}

int 
DvbScanChannel::parseSection(const uint8_t* data, uint16_t size)
{
    switch (mScanState) {
        case eScanPat : return _ParsePAT(data, size);
        case eScanSdt : return _ParseSDT(data, size);
        default:
            ;
    }
    return eSectionNotNeed;
}

void
DvbScanChannel::Run(void* threadID)
{
    LogDvbDebug("ScanChannel thread is running\n");
    setState(eScanIdle);
    DvbDemuxFilter* demux = new DvbDemuxFilter(this);
    if (!demux)
        return ;
    for (std::size_t i = 0; isRun() && i  < mTransponderKeys.size(); ++i) {
        mKeysIndex = i;
        Transponder* transponder = mManager.getTransponder(mTransponderKeys[i]);
        if (!transponder)
            continue;
        demux->setParserBand(mManager.getFrontend()->getParserBand(transponder->mDvbDevice->mTunerId));
        if (mManager.getFrontend()->tune(transponder->mDvbDevice->mTunerId, transponder)) {
            //Start filter Pat section
            mPatSections.clear();
            setState(eScanPat);
            demux->setPsisiPid(ePatPid);
            demux->setTimeout(3000);
            demux->start(); //block

            if (!isRun())
                break;

            //Start filter Sdt section
            mSdtSections.clear();
            setState(eScanSdt);
            demux->setPsisiPid(eSdtPid);
            demux->setTimeout(3000);
            demux->start(); //block
        }

        //Channel strore;
        for (std::size_t i = 0; i < mScanChannelsCache.size(); ++i) {
            DvbChannel* channel = new DvbChannel(transponder);
            channel->mServiceId         = mScanChannelsCache[i]._ProgramNumber;
            channel->mProgramMapPid     = mScanChannelsCache[i]._ProgramMapPid;
            channel->mServiceType       = mScanChannelsCache[i]._ServiceType;
            channel->mChannelType       = mScanChannelsCache[i]._ChannelType;
            channel->mChannelName       = mScanChannelsCache[i]._ServiceName;
            channel->mTransportStreamId = mScanChannelsCache[i]._TransportStreamId;
            channel->mOriginalNetworkId = mScanChannelsCache[i]._OriginalNetworkId;
            channel->mChannelKey = UtilsCreateKey(channel->mServiceId, channel->mTransportStreamId, channel->mOriginalNetworkId, 0);
            if (eDvbOk != mManager.addDvbChannel(channel)) {
                delete(channel);
                continue;
            }
            if (channel->mServiceType == eServiceRadio)
                mAudioCount++;
            else 
                mVideoCount++;
            mDvbChannelKeys.push_back(channel->mChannelKey);
        }
        mScanChannelsCache.clear();
        mProgramMapPids.clear();
    }
    delete (demux);
    if (mDvbChannelKeys.size() > 0)
        mManager.saveDB(eDvbChannelDB);
    setState(eScanFinish);
    LogDvbDebug("ScanChannel thread is end!\n");
}
