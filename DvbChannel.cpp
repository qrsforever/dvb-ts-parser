#include "DvbAssertions.h"
#include "DvbChannel.h"
#include "DvbUtils.h"
#include "DvbEpg.h"
#include "Transponder.h"

DvbChannel::DvbChannel(Transponder* transponder) : mTransponder(transponder)
{
    mEpg = new DvbEpg(this);
}

DvbChannel::~DvbChannel()
{
    if (mEpg)
        delete(mEpg);
    mEpg = 0;
    mChannelName.clear();
}

int 
DvbChannel::getTransponderKey() 
{ 
    if (mTransponder)
        return mTransponder->mUniqueKey; 
    return -1;
}

int 
DvbChannel::getEventsCount() const
{
    if (mEpg)
        return mEpg->getCount();
    return 0;
}

void 
DvbChannel::show()
{
    LogDvbDebug("DvbChannel--->\n"
        "UniqueKey: %d  ChannelName: %s \tProgramMapPid: %d ServerId: %d TransportStreamId: %d OriginalNetworkId: %d Servertype: %s ChannelType: %s EventsCount: %d\n",
        mChannelKey, mChannelName.c_str(), mProgramMapPid, mServiceId, mTransportStreamId, mOriginalNetworkId, EnumToStr(mServiceType), EnumToStr(mChannelType), mEpg->getCount());
}

void 
DvbReminder::show()
{
    LogDvbDebug("DvbReminder--->\n"
        "BookId: %d ChannelKey: %d StartTime: %s EventId: %d PeriodType: %d ExtendInfo: %s\n", 
        mBookId, mChannelKey, DvbEpg::timeStr(mStartTime).c_str(), mEventId, mPeriodType, mExtendDes.c_str());
}
