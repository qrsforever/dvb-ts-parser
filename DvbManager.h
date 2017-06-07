#ifndef __DvbManager__H_
#define __DvbManager__H_

#include "MessageHandler.h"
#include "DvbEnum.h"
#include "SysTime.h"
#include <iostream>
#include <list>
#include <vector>
#include <map>
#include "ProgramChannelC20.h"

class DvbScan;
class Frontend;
class DvbDevice;
class Transponder;
class DvbChannel;
class DvbReminder;
class SQLiteTableDVB;

class DvbManager : public Hippo::MessageHandler {
public:
    friend DvbManager& getDvbManager();
    ~DvbManager();
    void show();
    bool loadDB();
    bool saveDB(int flag = 0x1111);
    Frontend* getFrontend() { return mFrontend; }
    void setCountryCode(const char* code) { mCountryCode.assign(code); }
    const char* getCountryCode() { return mCountryCode.c_str(); }

    int getDvbDevicesCount(int type = 0);
    int getTranspondersCount(int deviceKey = 0);
    int getDvbChannelsCount(ServiceType_e type = eServiceAll, int transponderKey = 0);
    int getDvbRemindersCount() { return mReminders.size(); }

    std::vector<int> getDvbDeviceKeys(int type = 0); 
    std::vector<int> getTransponderKeys(int deviceKey = 0);
    std::vector<int> getDvbChannelKeys(ServiceType_e type = eServiceAll, int transponderKey = 0);
    std::vector<int> getDvbReminderKeys();

    DvbDevice* getDvbDevice(int key);
    Transponder* getTransponder(int key);
    DvbChannel* getDvbChannel(int key);
    DvbReminder* getDvbReminder(int key);
    DvbScan* getDvbScan(std::string name);

    int addDvbDevice(DvbDevice* device);
    int addTransponder(Transponder* transponder);
    int addDvbChannel(DvbChannel* channel);
    int addDvbScan(DvbScan* scan);
    int addDvbReminder(DvbReminder* reminder);

    void delDvbDevice(int key);
    void delTransponder(int key);
    void delDvbChannel(int key);
    void delDvbReminder(int key);
    void delDvbScan(std::string name);

    bool isDateTimeSync() { return mDateTimeSyncOk; }
    void setDvbDateTime(time_t utcTime, int timeOffset, int nextOffset, time_t changeTime);
    int getDateTimeOffset() { return mDataTimeOffset; }

    enum DvbMessage_ {
        eDvbMessageEpgUpdate    = 1,
        eDvbMessageEpgDelete    = 2,
        eDvbMessageTimeSyncOk   = 3,
        eDvbMessageBookReminder = 4,
    };

    virtual void handleMessage(Hippo::Message* msg);
    void onDvbMessageEpgUpdate();
    void onDvbMessageEpgDelete();
    void onDvbMessageTimeSyncOk();
    void onDvbMessageBookReminder();
    
    void setCurrentChannel(DvbChannel* channel) { mCurrentChannel = channel; }
    void setCurrentTunerId(int tunerId) { mCurrentTunerId = tunerId; }
    DvbChannel* getCurrentChannel() { return mCurrentChannel; }
    int getCurrentTunerId() { return mCurrentTunerId; }

private:
    DvbManager(TransmissionType_e type);
    Frontend* mFrontend;
    SQLiteTableDVB* mSQLiteDB;
    std::string mCountryCode;
    bool mDateTimeSyncOk;
    int mDataTimeOffset; 
    DvbScan* mScan;
    std::map<int, DvbDevice*> mDvbDevices;
    std::map<int, Transponder*> mTransponders;
    std::map<int, DvbChannel*> mDvbChannels;
    std::map<int, DvbReminder*> mReminders;
    std::map<std::string, DvbScan*> mDvbScans;
    int mCurrentTunerId;
    DvbChannel* mCurrentChannel;
};

int DvbChannelParseAdd(Hippo::ProgramChannelC20* program);
DvbManager& getDvbManager();
#endif
