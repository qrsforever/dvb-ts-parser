#ifndef __DvbChannel__H_
#define __DvbChannel__H_

#include "DvbEnum.h"

#include <stdint.h>
#include <iostream>
#include <map>
#include <string>
#include <sys/time.h>

#define REMINDER_MAX_COUNT 20

class DvbReminder {
public :
    DvbReminder( ):mBookId(0),mChannelKey(0),mEventId(-1),mPeriodType(0),mStartTime(0),mExtendDes(""){ }
    ~DvbReminder( ) { mExtendDes.clear(); }
    void show();
    enum PeriodType_ {
        eOnce    = 0x0,
        eDaily   = 0x1,
        eWeekly  = 0x2,
        eMonthly = 0x3,
        eYearly  = 0x4,
    };
    int mBookId;
    int mChannelKey;
    int mEventId;
    int mPeriodType; //0x0 - 0x4
    time_t mStartTime;
    std::string  mExtendDes;
};

class DvbEpg;
class Transponder;
class DvbChannel {
public:
    DvbChannel(Transponder* transponder);
    ~DvbChannel();
    int mChannelKey;
    int getTransponderKey();
    ServiceType_e mServiceType; 
	int mProgramMapPid;
    int mTransportStreamId;
    int mOriginalNetworkId;
    int mServiceId;
    ChannelType_e mChannelType;
    std::string mChannelName;
    int getEventsCount() const;
    DvbEpg* getEpg() const { return mEpg; }
    void show(); //for debug
    Transponder* mTransponder;
private:
    DvbEpg* mEpg;
};

#endif
