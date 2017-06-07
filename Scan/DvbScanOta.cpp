#include "DvbAssertions.h"
#include "DvbScanOta.h"
#include "DvbDemuxFilter.h"
#include "DvbUtils.h"
#include "DvbSection.h"
#include "DvbNitParser.h"
#include "DvbPatParser.h"
#include "DvbPmtParser.h"
#include "DvbManager.h"
#include "DvbLinkageDescriptor.h"
#include "DvbSatelliteDescriptor.h"

#include "SysSetting.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/epoll.h>

static void OtaShowInfo(int u32Oui, int u32HardwareV1, int u32HardwareV2, int u32SoftVersion, char u8UpdateType)
{
    printf("-----------------------Ota Software Upgrade-------------------\n"
                "\tOui [%08X]"
                "\tHardware [%08X%06X]"
                "\tSoftware [%08X]"
                "\tUpdateType [%02X]\n", 
                u32Oui, u32HardwareV1, u32HardwareV2, u32SoftVersion, u8UpdateType);
}

DvbScanOta::DvbScanOta(std::string name, Hippo::UpgradeManager* upgradeManager) : 
    DvbScan(name), mLastSectionNum(0), mCurrSectionCnt(0), mUpgradeManager(upgradeManager)
{
    LogDvbDebug("contructor\n");
    if (pipe2(mPipeFds, O_NONBLOCK)) {
        mPipeFds[0] = -1;
        mPipeFds[1] = -1;
        LogDvbError("pipe2 error!\n");
    }
    memset(&mUpgradeSource, 0, sizeof(mUpgradeSource));
    sysSettingGetInt("MainFrequency", (int*)&mUpgradeSource.nFrequency, 0);
    sysSettingGetInt("MainPolarization", (int*)&mUpgradeSource.nPolarization, 0);
    sysSettingGetInt("MainSymbolRate", (int*)&mUpgradeSource.nSymbolRate, 0);

    mTunerId = 1;
    mClientOui = 0x0000E0FC;
    mHardwareVersion = 0x59583638373644LL;
    mSoftwareVersion = 0;
}

DvbScanOta::~DvbScanOta() 
{ 
    LogDvbDebug("destructor\n");
    if (mPipeFds[0] < 0)
        close(mPipeFds[0]);
    if (mPipeFds[1] < 0)
        close(mPipeFds[1]);
    for (std::size_t i = 0; i < mDataBlocks.size(); ++i) {
        if (mDataBlocks[i]) {
            delete [](mDataBlocks[i]);
            mDataBlocks[i] = 0;
        }
    }
}

bool 
DvbScanOta::startCheck(int timeout)
{
    if (getState() != eScanIdle) {
        return false;
    }
    return _SendCmd(eFilterNIT, DvbSectionNIT::eActualTableID, eNitPid, timeout);
}

bool 
DvbScanOta::startReceive(int timeout)
{
    if (getState() != eScanIdle) {
        return false;
    }
    return _SendCmd(eFilterPAT, DvbSectionPAT::eActualTableID, ePatPid, timeout);
}

void
DvbScanOta::taskSuspend()
{
    setState(eScanSuspend);
}

bool
DvbScanOta::_SendCmd(uint8_t cmd, uint8_t tid, uint16_t pid, uint16_t timeout)
{
    DvbOtaCmd_s cmd_s;
    cmd_s.nCmdType = cmd;
    cmd_s.nTableId = tid;
    cmd_s.nPsiPid  = pid;
    cmd_s.nTimeout = timeout;
    if (write(mPipeFds[1], (const void*)&cmd_s, sizeof(cmd_s)) < 0) {
        LogDvbError("write cmd { %d, %d, %d } : %s\n", cmd_s.nCmdType, cmd_s.nPsiPid, cmd_s.nTimeout, strerror(errno));
        return false;
    }
    return true;
}

int 
DvbScanOta::_ParseNIT(const uint8_t* data, uint16_t size)
{
    // LogDvbDebug("data = %p, size = %d\n", data, size);
    bool bFindOui = false;
    DvbSectionNIT section(data, size);
    if (!section.isValid())
        return eSectionInvalid;

    if (DvbSectionNIT::eActualTableID != section.getTableId())
        return eSectionNotNeed;

    DvbNitParser parser(&section);
    for (DvbDescriptor descriptor = parser.descriptors(); isRun() && descriptor.isValid();  descriptor.advance()) {
        // LogDvbDebug("descriptor tag : 0x%02x\n", descriptor.tag());
        if (0x4A == descriptor.tag()) {//0x4A: linkage_descriptor
            DvbLinkageDescriptor linkageDescriptor(descriptor);
            // LogDvbDebug("linkage type : 0x%02x len : %d\n", linkageDescriptor.getLinkageType(), linkageDescriptor.getLinkageLength());
            if (0x09 == linkageDescriptor.getLinkageType()) { //System Software Update Service 
                if (linkageDescriptor.getLinkageLength() < 17)
                    continue;
                const uint8_t* pLinkageData = linkageDescriptor.getLinkageData();
                // uint8_t  u8OuiDataLen  = (uint8_t )UtilsGetBits(pLinkageData, 0, 0 , 8 );
                uint32_t u32Oui        = (uint32_t)UtilsGetBits(pLinkageData, 0, 8 , 24);
                // uint8_t  u8SecDataLen  = (uint8_t )UtilsGetBits(pLinkageData, 0, 32, 8 );
                uint8_t  u8UpgradeType = (uint8_t )UtilsGetBits(pLinkageData, 0, 40, 8 );
                uint32_t u32HardwareV1 = (uint32_t)UtilsGetBits(pLinkageData, 0, 48, 32);
                uint32_t u32HardwareV2 = (uint32_t)UtilsGetBits(pLinkageData, 0, 80, 24);
                uint32_t u32SoftVersion= (uint32_t)UtilsGetBits(pLinkageData, 0, 104,32);
                uint64_t u64HardVersion= (uint64_t)u32HardwareV1 << 24 | (uint64_t)u32HardwareV2;
                OtaShowInfo(u32Oui, u32HardwareV1, u32HardwareV2, u32SoftVersion, u8UpgradeType);
                if (mClientOui == u32Oui && mHardwareVersion == u64HardVersion && mSoftwareVersion != u32SoftVersion) {
                    mUpgradeSource.nTransportStreamId = linkageDescriptor.getTransportStreamId();
                    mUpgradeSource.nOriginalNetworkId = linkageDescriptor.getOriginalNetworkId();
                    mUpgradeSource.nServiceId = linkageDescriptor.getServiceId();
                    mUpgradeSource.nUpgradeType = u8UpgradeType;
                    mUpgradeSource.nSoftVersion = u32SoftVersion;
                    bFindOui = true;
                }
            }
        }
    }
    for (DvbNitElement element = parser.elements(); isRun() && bFindOui && element.isValid(); element.advance()) {
        if (mUpgradeSource.nTransportStreamId != element.getTransportStreamId() &&
            mUpgradeSource.nOriginalNetworkId != element.getOriginalNetworkId())
            continue;
        for(DvbDescriptor descriptor = element.descriptors(); isRun() && descriptor.isValid(); descriptor.advance()) {
            if (0x43 == descriptor.tag()) {//0x43: satellite_delivery_system
                DvbSatelliteDescriptor satelliteDescriptor(descriptor);
                mUpgradeSource.nFrequency = satelliteDescriptor.getFrequency();
                mUpgradeSource.nPolarization = satelliteDescriptor.getPolarization();
                mUpgradeSource.nSymbolRate = satelliteDescriptor.getSymbolRate();
                goto END;
            }
        }
    }

END:
    if (bFindOui) {
        // Send mesage to upgrade module.
        mUpgradeManager->sendEmptyMessage(0xff01);
        _SendCmd(eFilterPAT, DvbSectionPAT::eActualTableID, ePatPid, 0); //for debug
        return eSectionSuccess;
    }
    return eSectionNotNeed;
}

int 
DvbScanOta::_ParsePAT(const uint8_t* data, uint16_t size)
{
    // LogDvbDebug("data = %p, size = %d\n", data, size);
    DvbSectionPAT section(data, size);
    if (!section.isValid())
        return eSectionInvalid;

    DvbPatParser parser(&section);
    for (DvbPatElement element = parser.elements(); isRun() && element.isValid(); element.advance()) {
        // LogDvbDebug("ServiceId: %02x, Pid: 0x%02x\n", element.getProgramNumber(), element.getProgramMapPid());
        if (mUpgradeSource.nServiceId == element.getProgramNumber()) {
            _SendCmd(eFilterPMT, DvbSectionPMT::eActualTableID, element.getProgramMapPid(), 500);
            return eSectionSuccess;
        }
    }
    return eSectionNotNeed;
}

int 
DvbScanOta::_ParsePMT(const uint8_t* data, uint16_t size)
{
    // LogDvbDebug("data = %p, size = %d\n", data, size);
    DvbSectionPMT section(data, size);
    if (!section.isValid())
        return eSectionInvalid;

    DvbPmtParser parser(&section);
    for (DvbPmtElement element = parser.elements(); isRun() && element.isValid(); element.advance()) {
        // LogDvbDebug("element stream type 0x%02x\n", element.getStreamType());
        if (0x05 != element.getStreamType()) // 0x05: private_section
            continue;
        for (DvbDescriptor descriptor = element.descriptors(); isRun() && descriptor.isValid(); descriptor.advance()) {
            // LogDvbDebug("descriptor tag: 0x%02x len: %d\n", descriptor.tag(), descriptor.getLength()); 
            if (0x66 != descriptor.tag()) // 0x66: data_broadcast_id_descriptor
                continue;
            if (descriptor.getLength() < 26) //26 = (184 + 8) / 8 + 8(tag) + 8(length)
                continue;
            const uint8_t* pBroadcastData = descriptor.getData();
            uint16_t u16BroadcastId = (uint16_t)UtilsGetBits(pBroadcastData, 0, 16, 16);
            // LogDvbDebug("BroadcastID: 0x04X\n", u16BroadcastId);
            if (0x000A != u16BroadcastId) 
                continue;
            // uint8_t  u8OuiDataLen  = (uint8_t )UtilsGetBits(pBroadcastData, 0, 32, 8 );
            uint32_t u32Oui        = (uint32_t)UtilsGetBits(pBroadcastData, 0, 40, 24);
            // uint8_t  u8SecDataLen  = (uint8_t )UtilsGetBits(pBroadcastData, 0, 80, 8 );
            // uint8_t  u8UpgradeType = (uint8_t )UtilsGetBits(pBroadcastData, 0, 88, 8 );
            uint32_t u32HardwareV1 = (uint32_t)UtilsGetBits(pBroadcastData, 0, 96, 32);
            uint32_t u32HardwareV2 = (uint32_t)UtilsGetBits(pBroadcastData, 0, 128,24);
            uint32_t u32SoftVersion= (uint32_t)UtilsGetBits(pBroadcastData, 0, 152,32);
            uint8_t  u8DownCtrlTid = (uint8_t )UtilsGetBits(pBroadcastData, 0, 184, 8);
            uint64_t u64HardVersion= (uint64_t)u32HardwareV1 << 24 | (uint64_t)u32HardwareV2;
            if (mClientOui == u32Oui && mHardwareVersion == u64HardVersion && mUpgradeSource.nSoftVersion == u32SoftVersion) {
                mUpgradeSource.nElementaryPid  = element.getElementaryPid();
                _SendCmd(eFilterDCT, u8DownCtrlTid, mUpgradeSource.nElementaryPid, 0);
                return eSectionSuccess;
            }
        }
    }
    return eSectionNotNeed;
}

int 
DvbScanOta::_ParseDCT(const uint8_t* data, uint16_t size)
{
    // LogDvbDebug("data = %p, size = %d\n", data, size);
    const uint8_t* pData      = data;
    const uint8_t* pDescData  = 0;
    const uint8_t* pDataEnd   = 0;

    /* ====================================   Dct Head    =========================================== */
    struct _DctHead {                                                     //     ^ 
        uint8_t  u8TableId;                                               //     |
        uint16_t u16SectionLen;                                           //     |
        uint16_t u16TableExtendId;                                        //     |
        uint8_t  u8Version;                                               //     |
        uint8_t  u8SectionNum;                                            //     |
        uint8_t  u8LastSectionNum;                                        //     |
    }DctHead;                                                             // Head Parser
    memset(&DctHead, 0, sizeof(struct _DctHead));                         //     |
    DctHead.u8TableId        = (uint8_t )UtilsGetBits(pData, 0,  0, 8 );  //     |
    DctHead.u16SectionLen    = (uint16_t)UtilsGetBits(pData, 0, 12, 12);  //     |
    DctHead.u16TableExtendId = (uint16_t)UtilsGetBits(pData, 0, 24, 16);  //     |
    DctHead.u8Version        = (uint8_t )UtilsGetBits(pData, 0, 42, 5 );  //     |
    DctHead.u8SectionNum     = (uint8_t )UtilsGetBits(pData, 0, 48, 8 );  //     |
    DctHead.u8LastSectionNum = (uint8_t )UtilsGetBits(pData, 0, 56, 8 );  //     V
    /* ============================================================================================== */

    pData    = pData + 3 + 5; 
    pDataEnd = pData + DctHead.u16SectionLen - 5 - 4;                     /* CRC: 4 Bytes */

    pDescData = pData;

    uint32_t u32Oui         = (uint32_t)UtilsGetBits(pDescData, 0,  0 , 24);
    uint32_t u32HardwareV1  = (uint32_t)UtilsGetBits(pDescData, 0,  24, 32);
    uint32_t u32HardwareV2  = (uint32_t)UtilsGetBits(pDescData, 0,  56, 24);
    uint32_t u32SoftVersion = (uint32_t)UtilsGetBits(pDescData, 0,  80, 32);
    uint8_t  u8DownDataTid  = (uint8_t )UtilsGetBits(pDescData, 0, 112, 8 );
    uint32_t u32DownDataLen = (uint32_t)UtilsGetBits(pDescData, 0, 120, 32);
    uint16_t u16DownLastSec = (uint16_t)UtilsGetBits(pDescData, 0, 152, 16);
    uint64_t u64HardVersion = (uint64_t)u32HardwareV1 << 24 | (uint64_t)u32HardwareV2;
    if (mClientOui == u32Oui && mHardwareVersion == u64HardVersion && mUpgradeSource.nSoftVersion == u32SoftVersion) {
        LogDvbDebug("DownDataLen[%d], LastSection[%d]\n", u32DownDataLen, u16DownLastSec);
        _SendCmd(eFilterDDT, u8DownDataTid, mUpgradeSource.nElementaryPid, 0);
        for (std::size_t i = 0; i < mDataBlocks.size(); ++i) {
            if (mDataBlocks[i]) {
                delete [](mDataBlocks[i]);
                mDataBlocks[i] = 0;
            }
        }
        mDataBlocks.resize(u16DownLastSec, 0);
        mLastSectionNum = u16DownLastSec;
        mCurrSectionCnt = 0;
        return eSectionSuccess;
    }
    return eSectionNotNeed;
}

int 
DvbScanOta::_ParseDDT(const uint8_t* data, uint16_t size)
{
    // LogDvbDebug("data = %p, size = %d\n", data, size);
    if (size < 13) 
        return eSectionInvalid;

    uint16_t u16SectionLen = 0;
    uint16_t u16SectionNum = 0;

    u16SectionLen = (uint16_t)UtilsGetBits(data, 0, 12, 12); 
    if (u16SectionLen < 11)
        return eSectionInvalid;
    u16SectionNum = (uint16_t)UtilsGetBits(data, 8, 0, 16);
    if (mDataBlocks[u16SectionNum])
        return eSectionSame;

    mDataBlocks[u16SectionNum] = new OtaDataBlock(data + 10, u16SectionLen - 11);
    if (!mDataBlocks[u16SectionNum]) {
        LogDvbError("new OtaDataBlock.\n");
        return eSectionIncomplete;
    }

    mCurrSectionCnt++;
    if (mCurrSectionCnt > mLastSectionNum) {
        LogDvbDebug("data receive ok [%d]\n", mCurrSectionCnt);
        FILE* fp = fopen(UPGRADE_PATH, "wb");
        for (std::size_t i = 0; i < mDataBlocks.size(); ++i) {
            if (mDataBlocks[i]) {
                fwrite(mDataBlocks[i]->getData(), mDataBlocks[i]->getSize(), 1, fp);     
                delete mDataBlocks[i];
                mDataBlocks[i] = 0;
            }
        }
        fflush(fp);
        fclose(fp);
        mUpgradeManager->sendEmptyMessage(0xff03);
        return eSectionSuccess;
    }
    LogDvbDebug("CurrSectionCnt[%d]\n", mCurrSectionCnt);
    return eSectionIncomplete;
}

int 
DvbScanOta::parseSection(const uint8_t* data, uint16_t size)
{
    // LogDvbDebug("data = %p, size = %d data[0] = 0x%02x\n", data, size, data[0]);
    if (!data && size <= 0) 
        return eSectionInvalid;
    switch (getState()) {
        case eScanNit: return _ParseNIT(data, size);
        case eScanPat: return _ParsePAT(data, size);
        case eScanPmt: return _ParsePMT(data, size);
        case eScanDct: return _ParseDCT(data, size);
        case eScanDdt: return _ParseDDT(data, size);
        default:
            ;
    }
    return eSectionNotNeed;
}

void
DvbScanOta::Run(void* threadID)
{
    LogDvbDebug("ScanOta thread is running\n");
    setState(eScanIdle);
    DvbOtaCmd_s cmd;
    DvbDemuxFilter* demux = new DvbDemuxFilter(this);
    if (!demux)
        return;
    demux->setParserBand(mManager.getFrontend()->getParserBand(mTunerId));

    uint8_t mask[FILTER_OPTION_LENGTH];
    uint8_t coef[FILTER_OPTION_LENGTH];
    uint8_t excl[FILTER_OPTION_LENGTH];
    memset(mask, 0xff, sizeof(mask));
    memset(coef, 0xff, sizeof(coef));
    memset(excl, 0x00, sizeof(excl));
    mask[0] = 0x00;

    int epollFd = epoll_create(1);
    struct epoll_event eev;
    memset(&eev, 0, sizeof(struct epoll_event));
    eev.events = EPOLLIN;
    eev.data.fd = mPipeFds[0];
    epoll_ctl(epollFd, EPOLL_CTL_ADD, mPipeFds[0], &eev);
    while (isRun() && epoll_wait(epollFd, &eev, 1, -1) > 0) {
        if (eev.data.fd != mPipeFds[0] || eev.events != EPOLLIN)
            continue;
        read(mPipeFds[0], (void*)&cmd, sizeof(cmd));
        LogDvbDebug("pipe cmd[type:%d tableId:0x%02x pid: 0x%02x timeout:%d]\n", cmd.nCmdType, cmd.nTableId, cmd.nPsiPid, cmd.nTimeout);
        coef[0] = cmd.nTableId;
        // demux->setDemuxFilterType(eDemuxTsFilter);
        demux->setPsisiPid(cmd.nPsiPid);
        demux->setTimeout(cmd.nTimeout);
        demux->setFilterOption(mask, coef, excl);
        switch (cmd.nCmdType) {
            case eFilterNIT: setState(eScanNit); break;
            case eFilterPAT: setState(eScanPat); break;
            case eFilterPMT: setState(eScanPmt); break;
            case eFilterDCT: setState(eScanDct); break;
            case eFilterDDT: setState(eScanDdt); break;
            default:
                LogDvbError("read cmd!\n");
        }
        if (DvbDemuxFilter::eTimeout == demux->start()) { //block
            LogDvbDebug("demux ddt timeout!\n");
            mUpgradeManager->sendEmptyMessage(0xff02);
        }
        setState(eScanIdle);
    }
    if (demux)
        delete(demux);
    close(epollFd);
    setState(eScanFinish);
    LogDvbDebug("ScanOta thread is end\n");
}
