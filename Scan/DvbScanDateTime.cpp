#include "DvbAssertions.h"
#include "DvbScanDateTime.h"
#include "DvbDemuxFilter.h"
#include "DvbSection.h"
#include "DvbTotParser.h"
#include "DvbManager.h"
#include "DvbUtils.h"
#include "DvbDescriptor.h"
#include "DvbTimeOffsetDescriptor.h"

DvbScanDateTime::DvbScanDateTime(std::string name, int id) : DvbScan(name), mTunerId(id)
{
    LogDvbDebug("contructor\n");
}

DvbScanDateTime::~DvbScanDateTime() 
{ 
    LogDvbDebug("destructor\n");
}

int 
DvbScanDateTime::_ParseTDT(const uint8_t* data, uint16_t size)
{
    if (!isRun())
        return eSectionSuccess;

    DvbSectionTDT section(data, size);
    if (!section.isValid())
        return eSectionInvalid;

    mManager.setDvbDateTime(section.getUtcTime(), 0, 0, 0);
    return eSectionSuccess;
}

int 
DvbScanDateTime::_ParseTOT(const uint8_t* data, uint16_t size)
{
    if (!isRun())
        return eSectionSuccess;

    DvbSectionTOT section(data, size);
    if (!section.isValid())
        return eSectionInvalid;

    DvbTotParser parser(&section);
    for (DvbDescriptor descriptor = parser.descriptors(); descriptor.isValid(); descriptor.advance()) {
        if (0x58 == descriptor.tag()) {
            DvbTimeOffsetDescriptor timeDescr(descriptor);
            const char* lan = mManager.getCountryCode();
            mManager.setDvbDateTime(section.getUtcTime(), timeDescr.getTimeOffset(lan), timeDescr.getNextOffset(lan), timeDescr.getTimeChange(lan));
            mManager.sendEmptyMessageDelayed(DvbManager::eDvbMessageTimeSyncOk, 3000);
            break;
        }
    }
    return eSectionSuccess;
}

int 
DvbScanDateTime::parseSection(const uint8_t* data, uint16_t size)
{
    switch (mScanState) {
        case DvbScan::eScanTot : return _ParseTOT(data, size);
        case DvbScan::eScanTdt : return _ParseTDT(data, size);
        default:
            ;
    }
    return eSectionNotNeed;
}

void 
DvbScanDateTime::Run(void* threadID)
{
    LogDvbDebug("DvbScanDateTime thread is running\n");
    setState(eScanIdle);
    DvbDemuxFilter* filter = new DvbDemuxFilter(this);
    if (!filter)
        return ;

    if (!mManager.getFrontend()->isTuned(mTunerId))
        LogDvbWarn("is not Tuned\n");

    setState(eScanTot);
    filter->setParserBand(mManager.getFrontend()->getParserBand(mTunerId));
    filter->setPsisiPid(eTotPid);
    filter->start(); //block
    if (!mManager.isDateTimeSync()) {
        mScanState = DvbScan::eScanTdt; 
        filter->setPsisiPid(eTdtPid);
        filter->start(); //block
    }
    delete(filter);
    setState(eScanFinish);
    LogDvbDebug("DvbScanDateTime thread end\n");
}
