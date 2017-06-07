#ifndef __DvbEpg__H_
#define __DvbEpg__H_

#include <pthread.h>
#include <stdint.h>
#include <iostream>
#include <map>
#include <vector>

class EventText {
public:
    EventText() {
    }
    ~EventText() {}
    EventText(const EventText &text) {}

    std::string	mLangCode;
    std::string	mEventName;
    std::string	mShortText;
    std::string	mExtendText;
};

class EpgEvent {
public:
    EpgEvent();
    ~EpgEvent();
    uint32_t mEventId;
    uint64_t mSTime;
    uint64_t mETime;
    bool mScrambled;
    int getRating(std::string& lan);
    std::map<std::string, EventText>& getEventText() { return mTexts; }
    std::string getEventName(std::string& lan);
    std::string getShortText(std::string& lan);
    std::string getExtendText(std::string& lan);
    std::map<std::string, EventText> mTexts;
    std::map<std::string, int> mRatings;
};

class DvbChannel;
class DvbEpg {
public:
    DvbEpg(DvbChannel* channel);
    ~DvbEpg();
    void update() { }
    void setVersion(int version) { mVersionNumber = version; }
    int getVersion() { return mVersionNumber; }
    bool isExist(uint32_t eventId) { return mEventMap.end() != mEventMap.find(eventId); }
    EpgEvent* getEvent(uint32_t eventId);
    bool addEvent(EpgEvent* event); 
    bool delEvent(uint32_t eventId);
    bool delEvent(time_t time, int direction);

    static std::string timeStr(time_t time);

    void setUpdateFlag(bool b) { 
        if (!b) {
            mMinUpdateTime = 0xefffffffffffffffLL;
            mMaxUpdateTime = 0;
        }
        mUpdateFlag = b; 
    }
    bool getUpdateFlag() { return mUpdateFlag; }
    void setMinUpdateTime(uint64_t t) { mMinUpdateTime = t; }
    void setMaxUpdateTime(uint64_t t) { mMaxUpdateTime = t; }
    uint64_t getMinUpdateTime() { return mMinUpdateTime; }
    uint64_t getMaxUpdateTime() { return mMaxUpdateTime; }

    int getCount(time_t stime = 0, time_t etime = 0);
    std::vector<EpgEvent*> getEvents(time_t stime, time_t etime);

    int lock() { return pthread_mutex_lock(&mMutex); }
    int unlock() { return pthread_mutex_unlock(&mMutex); }

    void show();
private:
    pthread_mutex_t mMutex;
    std::map<uint32_t, EpgEvent*> mEventMap;
    std::map<time_t, EpgEvent*> mSTimeMap;
    DvbChannel* mChannel;
    int mVersionNumber;
    bool mUpdateFlag;
    uint64_t mMinUpdateTime;
    uint64_t mMaxUpdateTime;
};

//std::map<int, DvbEpg*> ;
#endif
