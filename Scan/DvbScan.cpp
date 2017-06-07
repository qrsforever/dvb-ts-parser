#include "DvbAssertions.h"
#include "DvbScan.h"
#include "DvbManager.h"

DvbScan::DvbScan(std::string& name) : mScanName(name), mScanState(DvbScan::eScanIdle), mManager(getDvbManager())
{
}

DvbScan::~DvbScan()
{
}

int
DvbScan::scanStart()
{
    BaseThread::stop();
    BaseThread::start();
    return 0;
}

int 
DvbScan::scanStop()
{
    return BaseThread::stop();
}

int
DvbScan::parseSection(const uint8_t* data, uint16_t size)
{
    return -1;
}

void 
DvbScan::show()
{
    LogDvbDebug("DvbScan--->\n\tScanName[%s] ScanThreadState[%d]\n", mScanName.c_str(), getThreadState()); 
}
