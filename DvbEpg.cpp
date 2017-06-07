#include "DvbAssertions.h"
#include "DvbEpg.h"
#include "DvbChannel.h"
#include <sys/time.h>
#include <time.h>

EpgEvent::EpgEvent() 
{ 
}

EpgEvent::~EpgEvent()
{
    mTexts.clear();
    mRatings.clear();
}

int 
EpgEvent::getRating(std::string& lan)
{
    std::map<std::string, int>::iterator it = mRatings.find(lan);
    if (it != mRatings.end())
        return it->second;

    if ((it = mRatings.begin()) == mRatings.end())
        return 0;
    lan = it->first;
    return it->second;
}

std::string 
EpgEvent::getEventName(std::string& lan)
{
    std::map<std::string, EventText>::iterator it = mTexts.find(lan);
    if (it != mTexts.end())
        return it->second.mEventName;

    if ((it = mTexts.begin()) == mTexts.end())
        return "";
    lan = it->second.mLangCode;
    return it->second.mEventName;
}

std::string 
EpgEvent::getShortText(std::string& lan)
{
    std::map<std::string, EventText>::iterator it = mTexts.find(lan);
    if (it != mTexts.end())
        return it->second.mShortText;

    if ((it = mTexts.begin()) == mTexts.end())
        return "";
    lan = it->second.mLangCode;
    return it->second.mShortText;
}

std::string 
EpgEvent::getExtendText(std::string& lan)
{
    std::map<std::string, EventText>::iterator it = mTexts.find(lan);
    if (it != mTexts.end())
        return it->second.mExtendText;

    if ((it = mTexts.begin()) == mTexts.end())
        return "";
    lan = it->second.mLangCode;
    return it->second.mExtendText;
}

DvbEpg::DvbEpg(DvbChannel* channel) : mChannel(channel), mVersionNumber(-1), mUpdateFlag(false), mMinUpdateTime(0xefffffffffffffffLL), mMaxUpdateTime(0) 
{
    pthread_mutex_init(&mMutex, 0);
}

DvbEpg::~DvbEpg()
{
    std::map<uint32_t, EpgEvent*>::iterator it;
    lock();
    for (it = mEventMap.begin(); it != mEventMap.end(); it++) {
        delete(it->second);
    }
    unlock();
    mEventMap.clear();
    mSTimeMap.clear();

    pthread_mutex_destroy(&mMutex);
}

EpgEvent* 
DvbEpg::getEvent(uint32_t eventId)
{
    if (mEventMap.end() != mEventMap.find(eventId))
        return mEventMap[eventId];
    return 0;
}

bool 
DvbEpg::addEvent(EpgEvent* event)
{
    if (!event) //|| isExist(event->mEventId))
        return false;
    lock();
    time_t stime = event->mSTime;
    mSTimeMap[stime] = event;
    mEventMap[event->mEventId] = event;
    unlock();
    mUpdateFlag = true;
    return true;
}

bool 
DvbEpg::delEvent(uint32_t eventId)
{
    lock();
    std::map<uint32_t, EpgEvent*>::iterator it = mEventMap.find(eventId);
    if (mEventMap.end() != it) {
        EpgEvent* event = it->second;
        mSTimeMap.erase(event->mSTime);
        mEventMap.erase(event->mEventId);
        delete(event);
    }
    unlock();
    return true;
}

bool 
DvbEpg::delEvent(time_t time, int direction)
{
    std::map<time_t, EpgEvent*>::iterator itBound = mSTimeMap.lower_bound(time);
    if (itBound == mSTimeMap.end())
        return false;
    lock();
    std::map<time_t, EpgEvent*>::iterator it;
    if (direction < 0) {
        for (it = mSTimeMap.begin(); it != itBound; ++it) {
            mEventMap.erase(it->second->mEventId);
            delete(it->second);
        }
        mSTimeMap.erase(mSTimeMap.begin(), itBound);
    } else {
        for (it = itBound; it != mSTimeMap.end(); ++it) {
            mEventMap.erase(it->second->mEventId);
            delete(it->second);
        }
        mSTimeMap.erase(itBound, mSTimeMap.end());
    }
    unlock();
    return true;
}

int 
DvbEpg::getCount(time_t stime, time_t etime)
{
    if (!stime || !etime)
        return mEventMap.size();
    std::map<time_t, EpgEvent*>::iterator itSBound = mSTimeMap.upper_bound(stime);
    std::map<time_t, EpgEvent*>::iterator itEBound = mSTimeMap.lower_bound(etime);
    std::map<time_t, EpgEvent*>::iterator it;
    if (itSBound == mSTimeMap.end())
        return 0;
    return std::distance(itSBound, itEBound);
}

std::vector<EpgEvent*> 
DvbEpg::getEvents(time_t stime, time_t etime)
{
    std::vector<EpgEvent*> events;
    std::map<time_t, EpgEvent*>::iterator itSBound = mSTimeMap.upper_bound(stime);
    std::map<time_t, EpgEvent*>::iterator itEBound = mSTimeMap.lower_bound(etime);
    std::map<time_t, EpgEvent*>::iterator it;
    if (itSBound == mSTimeMap.end())
        return events;
    if (itSBound != mSTimeMap.begin())
        itSBound--;
    for (it = itSBound; it != itEBound; ++it)
        events.push_back(it->second);
    return events;
}

std::string 
DvbEpg::timeStr(time_t time)
{
    char str[16] = { 0 };
    struct tm tm; 
    gmtime_r(&time, &tm);
    sprintf(str, "%04d%02d%02d%02d%02d%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    return str;
}

void 
DvbEpg::show()
{    
    std::map<uint32_t, EpgEvent*>::iterator it;
    for (it = mEventMap.begin(); it != mEventMap.end(); it++) {
        LogDvbDebug("DvbEpg--->EventId[%d] ChannelKey[%d] ChannelName[%s]\n", it->second->mEventId, mChannel->mChannelKey, mChannel->mChannelName.c_str());
        LogDvbDebug("StartTimeCtime: %s", ctime((const time_t*)&(it->second->mSTime)));
        LogDvbDebug("EndTimeCtime: %s", ctime((const time_t*)&(it->second->mETime)));
        std::map<std::string, EventText>::iterator text;
        for (text = it->second->mTexts.begin(); text != it->second->mTexts.end(); ++text)
            LogDvbDebug("Lan[%s] EventName[%s]\n", text->first.c_str(), text->second.mEventName.c_str());
    }
}
