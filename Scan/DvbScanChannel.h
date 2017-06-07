#ifndef __DvbScanChannel__H_
#define __DvbScanChannel__H_

#include "DvbScan.h"
#include "DvbEnum.h"
#include "Transponder.h"

#include <iostream>
#include <vector>
#include <map>
#include <string>

class DvbDemuxFilter;
class DvbManager;
class DvbSectionPat;
class DvbSectionSdt;
class DvbScanChannel : public DvbScan {
public:
    DvbScanChannel(std::string name, std::vector<int>& transKeys, bool searchNit = false, ChannelType_e channelType = eChannelAll, ServiceType_e serviceType = eServiceAll);
    ~DvbScanChannel();
    class ScanChannelCache {
    public:
        ScanChannelCache(uint16_t number, uint16_t pid) : 
            _ProgramNumber(number), _ProgramMapPid(pid), 
            _TransportStreamId(0), _OriginalNetworkId(0) {}
        uint16_t _ProgramNumber;
        uint16_t _ProgramMapPid;
        uint16_t _TransportStreamId;
        uint16_t _OriginalNetworkId;
        ChannelType_e _ChannelType;
        ServiceType_e _ServiceType; 
        std::string _ProviderName;
        std::string _ServiceName;
    };
    virtual int parseSection(const uint8_t* data, uint16_t size);
    virtual int getProgress(); 
    std::vector<int> getTransponderKeys() { return mTransponderKeys; }
    std::vector<int> getDvbChannelKeys() { return mDvbChannelKeys; }
    int getChnlsCount(ServiceType_e type);
    int getCurrentTpKey() { return mTransponderKeys[mKeysIndex]; }
protected:
    virtual void Run(void* threadID);
private:
    int _ParsePAT(const uint8_t* data, uint16_t size);
    int _ParseSDT(const uint8_t* data, uint16_t size);
    ChannelType_e mChannelType;
    ServiceType_e mServiceType;
    bool mSearchNetwork;
    std::map<uint16_t, uint16_t> mProgramMapPids;
    std::vector<int>  mTransponderKeys;
    std::vector<int>  mDvbChannelKeys;
    std::vector<bool> mPatSections;
    std::vector<bool> mSdtSections;
    std::vector<ScanChannelCache> mScanChannelsCache;
    int mVideoCount;
    int mAudioCount;
    int mKeysIndex;
};

#endif
