#include "DvbAssertions.h"
#include "DvbManager.h"
#include "DvbScan.h"
#include "DvbScanDateTime.h"
#include "Frontend.h"
#include "DvbDevice.h"
#include "DvbEpg.h"
#include "Transponder.h"
#include "DvbChannel.h"
#include "FrontendDVBC.h"
#include "FrontendDVBS.h"
#include "FrontendDVBT.h"
#include "SQLiteTableDVBC.h"
#include "SQLiteTableDVBS.h"
#include "SQLiteTableDVBT.h"
#include "SystemManager.h"
#include "ProgramChannelDvb.h"
#include "BrowserEventQueue.h"

#include "mid/mid_time.h"
#include "ntp/mid_ntp.h"

#include "Hippo_HString.h"
#include <stdlib.h>
#include <math.h>

extern "C" int yu_setTimeZone(double timeZone);

typedef std::map<int, DvbDevice*>::iterator DvbDeviceIter;
typedef std::map<int, Transponder*>::iterator TransponderIter;
typedef std::map<int, DvbChannel*>::iterator DvbChannelIter;
typedef std::map<int, DvbReminder*>::iterator DvbReminderIter;
typedef std::map<std::string, DvbScan*>::iterator DvbScanIter;

static DvbManager* gDvbManager = 0;

DvbManager& getDvbManager()
{
    if (!gDvbManager)
        gDvbManager = new DvbManager(eTransmissionTypeDVBS);
    return *gDvbManager;
}

DvbManager::DvbManager(TransmissionType_e type) : mFrontend(0), mSQLiteDB(0), mCountryCode("HUN"), mDateTimeSyncOk(false), mDataTimeOffset(0), mCurrentTunerId(0), mCurrentChannel(0)
{
    switch (type) {
        case eTransmissionTypeDVBC:
            mFrontend = new FrontendDVBC();
            break;
        case eTransmissionTypeDVBS:
            mFrontend = new FrontendDVBS();
            mSQLiteDB = new SQLiteTableDVBS(this);
            break;
        case eTransmissionTypeDVBT:
            mFrontend = new FrontendDVBT();
            break;
        default:
            ;
    }
}

DvbManager::~DvbManager()
{
    if (mFrontend)
        delete mFrontend;

    if (mSQLiteDB)
        delete (mSQLiteDB);
}

bool
DvbManager::loadDB()
{
    return mSQLiteDB->load();
}

bool
DvbManager::saveDB(int flag)
{
    return mSQLiteDB->save(flag);
}

int
DvbManager::getDvbDevicesCount(int type)
{
    if (!type)
        return mDvbDevices.size();
    int count = 0;
    for (DvbDeviceIter it = mDvbDevices.begin(); it != mDvbDevices.end(); ++it) {
        if (type == it->second->getFavorFlag())
            count++;
    }
    return count;
}

int
DvbManager::getTranspondersCount(int deviceKey)
{
    if (!deviceKey)
        return mTransponders.size();
    int count = 0;
    for (TransponderIter it = mTransponders.begin(); it != mTransponders.end(); ++it) {
        if (deviceKey == it->second->getDvbDeviceKey())
            count++;
    }
    return count;
}

int
DvbManager::getDvbChannelsCount(ServiceType_e type, int transponderKey)
{
    if (!transponderKey) {
        if (eServiceAll == type)
            return mDvbChannels.size();
        int count = 0;
        for (DvbChannelIter it = mDvbChannels.begin(); it != mDvbChannels.end(); ++it) {
            if (type == it->second->mServiceType)
                count++;
        }
        return count;
    }

    int count = 0;
    for (DvbChannelIter it = mDvbChannels.begin(); it != mDvbChannels.end(); ++it) {
        if (transponderKey == it->second->getTransponderKey() && (eServiceAll == type || type == it->second->mServiceType))
            count++;
    }
    return count;
}

std::vector<int>
DvbManager::getDvbDeviceKeys(int type)
{
    std::vector<int> keys;
    if (!type) {
        for (DvbDeviceIter it = mDvbDevices.begin(); it != mDvbDevices.end(); ++it)
            keys.push_back(it->first);
        return keys;
    }

    for (DvbDeviceIter it = mDvbDevices.begin(); it != mDvbDevices.end(); ++it) {
        if (type == it->second->getFavorFlag())
            keys.push_back(it->first);
    }
    return keys;
}

std::vector<int>
DvbManager::getTransponderKeys(int deviceKey)
{
    std::vector<int> keys;
    if (!deviceKey) {
        for (TransponderIter it = mTransponders.begin(); it != mTransponders.end(); ++it)
            keys.push_back(it->first);
        return keys;
    }

    for (TransponderIter it = mTransponders.begin(); it != mTransponders.end(); ++it) {
        if (deviceKey == it->second->getDvbDeviceKey())
            keys.push_back(it->first);
    }
    return keys;
}

std::vector<int>
DvbManager::getDvbChannelKeys(ServiceType_e type, int transponderKey)
{
    std::vector<int> keys;
    if (!transponderKey) {
        for (DvbChannelIter it = mDvbChannels.begin(); it != mDvbChannels.end(); ++it) {
            if (eServiceAll == type || it->second->mServiceType == type)
                keys.push_back(it->first);
        }
        return keys;
    }

    for (DvbChannelIter it = mDvbChannels.begin(); it != mDvbChannels.end(); ++it) {
        if (transponderKey == it->second->getTransponderKey() && (eServiceAll == type || it->second->mServiceType == type))
            keys.push_back(it->first);
    }
    return keys;
}

std::vector<int>
DvbManager::getDvbReminderKeys()
{
    std::vector<int> keys;
    for (DvbReminderIter it = mReminders.begin(); it != mReminders.end(); ++it)
        keys.push_back(it->first);
    return keys;
}

DvbDevice*
DvbManager::getDvbDevice(int key)
{
    DvbDeviceIter it = mDvbDevices.find(key);
    if (it != mDvbDevices.end())
        return it->second;
    return 0;
}

Transponder*
DvbManager::getTransponder(int key)
{
    TransponderIter it = mTransponders.find(key);
    if (it != mTransponders.end())
        return it->second;
    return 0;
}

DvbChannel*
DvbManager::getDvbChannel(int key)
{
    DvbChannelIter it = mDvbChannels.find(key);
    if (it != mDvbChannels.end())
        return it->second;
    return 0;
}

DvbReminder*
DvbManager::getDvbReminder(int key)
{
    DvbReminderIter it = mReminders.find(key);
    if (it != mReminders.end())
        return it->second;
    return 0;
}

DvbScan*
DvbManager::getDvbScan(std::string name)
{
    DvbScanIter it = mDvbScans.find(name);
    if (it != mDvbScans.end())
        return it->second;
    return 0;
}

int
DvbManager::addDvbDevice(DvbDevice* device)
{
    if (!device)
        return eDvbBadParam;
    DvbDeviceIter it = mDvbDevices.find(device->mDeviceKey);
    if (it != mDvbDevices.end())
        return eDvbObjExist;
    mDvbDevices[device->mDeviceKey] = device;
    return eDvbOk;
}

int
DvbManager::addTransponder(Transponder* transponder)
{
    if (!transponder)
        return eDvbBadParam;
    TransponderIter it = mTransponders.find(transponder->mUniqueKey);
    if (it != mTransponders.end())
        return eDvbObjExist;
    mTransponders[transponder->mUniqueKey] = transponder;
    return eDvbOk;
}

int
DvbManager::addDvbChannel(DvbChannel* channel)
{
    if (!channel)
        return eDvbBadParam;
    DvbChannelIter it = mDvbChannels.find(channel->mChannelKey);
    if (it != mDvbChannels.end())
        return eDvbObjExist;
    mDvbChannels[channel->mChannelKey] = channel;

    Hippo::ProgramChannelDvb* program = new Hippo::ProgramChannelDvb();
    if (program) {
        program->SetChanKey(channel->mChannelKey);
        program->SetDVB_ProgNum(channel->mServiceId);
        program->SetDVB_PMT_PID(channel->mProgramMapPid);
        program->SetDVB_TpFreq(channel->mTransponder->mFrequency);
        Hippo::systemManager().channelList().addProgram(program);
    }
    return eDvbOk;
}

int
DvbManager::addDvbReminder(DvbReminder* reminder)
{
    if (!reminder)
        return eDvbBadParam;
    DvbReminderIter it = mReminders.find(reminder->mBookId);
    if (it != mReminders.end())
        return eDvbObjExist;
    mReminders[reminder->mBookId] = reminder;
    return eDvbOk;
}

int
DvbManager::addDvbScan(DvbScan* scan)
{
    if (!scan)
        return eDvbBadParam;
    DvbScanIter it = mDvbScans.find(scan->getScanName());
    if (it != mDvbScans.end())
        return eDvbObjExist;
    mDvbScans[scan->getScanName()] = scan;
    return eDvbOk;
}

void
DvbManager::delDvbDevice(int key)
{
    DvbDeviceIter iter = mDvbDevices.find(key);
    if (iter == mDvbDevices.end())
        return ;

    std::vector<int> transponderKeys;
    for (TransponderIter it = mTransponders.begin(); it != mTransponders.end(); ++it) {
        if (key == it->second->mDvbDevice->mDeviceKey)
            transponderKeys.push_back(it->first);
    }
    for (unsigned int i = 0; i < transponderKeys.size(); ++i)
        delTransponder(transponderKeys[i]);

    delete (iter->second);
    mDvbDevices.erase(iter);
}

void
DvbManager::delTransponder(int key)
{
    TransponderIter iter = mTransponders.find(key);
    if (iter == mTransponders.end())
        return ;

    std::vector<int> channelKeys;
    for (DvbChannelIter it = mDvbChannels.begin(); it != mDvbChannels.end(); ++it) {
        if (key == it->second->mTransponder->mUniqueKey)
            channelKeys.push_back(it->first);
    }
    for (unsigned int i = 0; i < channelKeys.size(); ++i)
        delDvbChannel(channelKeys[i]);

    delete (iter->second);
    mTransponders.erase(iter);
}

void
DvbManager::delDvbChannel(int key)
{
    DvbChannelIter iter = mDvbChannels.find(key);
    if (iter == mDvbChannels.end())
        return ;

    delete (iter->second);
    mDvbChannels.erase(iter);

    Hippo::Program* program = Hippo::systemManager().channelList().getProgramByNumberKey(key);
    if (program)
        Hippo::systemManager().channelList().removeProgram(program);
}

void
DvbManager::delDvbReminder(int key)
{
    DvbReminderIter iter = mReminders.find(key);
    if (iter == mReminders.end())
        return;
    delete (iter->second);
    mReminders.erase(iter);
}

void
DvbManager::delDvbScan(std::string name)
{
    DvbScanIter iter = mDvbScans.find(name);
    if (iter == mDvbScans.end())
        return ;

    delete (iter->second);
    mDvbScans.erase(iter);
}

void
DvbManager::handleMessage(Hippo::Message* msg)
{
    switch (msg->what) {
        case eDvbMessageEpgUpdate:
            onDvbMessageEpgUpdate();
            break;
        case eDvbMessageEpgDelete:
            onDvbMessageEpgDelete();
            break;
        case eDvbMessageTimeSyncOk:
            onDvbMessageTimeSyncOk();
            break;
        case eDvbMessageBookReminder:
            onDvbMessageBookReminder();
            break;
        default:
            ;
    }
}

void
DvbManager::onDvbMessageEpgUpdate()
{
    if (!mDateTimeSyncOk) {
        sendEmptyMessageDelayed(eDvbMessageEpgUpdate, 60000);
        return;
    }
    char tStrEvent[4040] = "{\"type\":\"EVENT_DVB_EPG_UPDATE\",\"EPGUpdateList\":[]}";
    int  tStrLen = strlen(tStrEvent) - 2; //2 = ]}
    time_t now = time(0);
    for (DvbChannelIter it = mDvbChannels.begin(); tStrLen < 4040 && it != mDvbChannels.end(); ++it) {
        DvbEpg* epg = it->second->getEpg();
        epg->lock();
        if (epg->getUpdateFlag()) {
            /* 86400 = 24 * 3600 */
            int s = (epg->getMinUpdateTime() - now) / 86400 - 1;
            if (s < 0 || s > 6)
                s = 0;
            int e = (epg->getMaxUpdateTime() - now) / 86400 + 1;
            if (e < 0 || e > 6)
                e = 6;
            int  tLen = 0;
            char tBuf[16] = "";
            while (s <= e)
                tLen += sprintf(tBuf + tLen, "%d,", s++);
            if (tLen > 0)
                tBuf[tLen - 1] = 0;

            tStrLen += sprintf(tStrEvent + tStrLen, "{\"chanKey\":%d,\"day\":[%s]},", it->second->mChannelKey, tBuf);
            epg->setUpdateFlag(false);
        }
        epg->unlock();
    }
    if (tStrLen <= 48)
        return;
    strcpy(tStrEvent + tStrLen - 1, "]}");
    browserEventSend(tStrEvent, 0);
    LogDvbDebug("tStrEvent: %s\n", tStrEvent);
}

void
DvbManager::onDvbMessageEpgDelete()
{
    if (!mDateTimeSyncOk) {
        sendEmptyMessageDelayed(eDvbMessageEpgDelete, 60000);
        return;
    }
    time_t now = time(0);
    for (DvbChannelIter it = mDvbChannels.begin(); it != mDvbChannels.end(); ++it) {
        DvbEpg* epg = it->second->getEpg();
        /* 259200 = 3 * 24 * 3600 */
        epg->delEvent(now - 259200, -1);
    }
    sendEmptyMessageDelayed(eDvbMessageEpgDelete, 86400000); //24 * 3600 * 1000 (ms)
}

void
DvbManager::onDvbMessageTimeSyncOk()
{
    delDvbScan(SCAN_DATETIME_NAME);
    sendEmptyMessageDelayed(eDvbMessageEpgDelete, 86400000); //24 * 3600 * 1000 (ms)
    if (mReminders.size() > 0)
        sendEmptyMessage(eDvbMessageBookReminder);
    if (mDvbChannels.size() > 0)
        sendEmptyMessage(eDvbMessageEpgUpdate);
}

void
DvbManager::onDvbMessageBookReminder()
{
    time_t now = time(0);
    std::vector<int> bookIds;
    for (DvbReminderIter it = mReminders.begin(); it != mReminders.end(); ++it) {
        if (it->second->mStartTime < now + 6)
            bookIds.push_back(it->first);
    }
    DvbReminder* reminder = 0;
    for (std::size_t i = 0; i < bookIds.size(); ++i) {
        reminder = getDvbReminder(bookIds[i]);
        if (!reminder)
            continue;
        if (reminder->mStartTime > now - 6) {
            DvbChannel* channel = getDvbChannel(reminder->mChannelKey);
            if (!channel) {
                delDvbReminder(reminder->mBookId);
                continue;
            }
            EpgEvent* event = channel->getEpg()->getEvent(reminder->mEventId);
            if (!event)
                continue;
            char e[4096] = { 0 };
            std::string lan = mCountryCode;
            if (reminder->mEventId < 0)
                snprintf(e, 4095, "{\"type\":\"EVENT_BOOK_COME_IN\",\"bookID\":%d,\"chanKey\":%d,\"channelName\":\"%s\","
                    "\"startTime\":\"%s\",\"periodType\":%d, \"extendDescription\":\"%s\"",
                    reminder->mBookId, reminder->mChannelKey, channel->mChannelName.c_str(),
                    DvbEpg::timeStr(reminder->mStartTime).c_str(), reminder->mPeriodType, reminder->mExtendDes.c_str());
            else
                snprintf(e, 4095, "{\"type\":\"EVENT_BOOK_COME_IN\",\"bookID\":%d,\"chanKey\":%d,\"channelName\":\"%s\","
                    "\"programID\":\"%d\",\"programName\":\"%s\",\"startTime\":\"%s\","
                    "\"periodType\":%d, \"extendDescription\":\"%s\"",
                    reminder->mBookId, reminder->mChannelKey, channel->mChannelName.c_str(),
                    reminder->mEventId, event->getEventName(lan).c_str(),
                    DvbEpg::timeStr(reminder->mStartTime).c_str(), reminder->mPeriodType, reminder->mExtendDes.c_str());
            browserEventSend(e, 0);
        }

        struct tm tm;
        gmtime_r(&reminder->mStartTime, &tm);
        static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        switch (reminder->mPeriodType) {
            case DvbReminder::eDaily:
                reminder->mStartTime += 24 * 3600;
                break;
            case DvbReminder::eWeekly:
                reminder->mStartTime += 7 * 24 * 3600;
                break;
            case DvbReminder::eMonthly:
                reminder->mStartTime += days[tm.tm_mon] * 24 * 3600;
                if (tm.tm_mon == 1 && UtilsIsLeapYear(tm.tm_year + 1900))
                    reminder->mStartTime += 24 * 3600;
                break;
            case DvbReminder::eYearly:
                reminder->mStartTime += 365 * 24 * 3600;
                if ((tm.tm_mon <= 1 && UtilsIsLeapYear(tm.tm_year + 1900)) ||
                    (tm.tm_mon >= 2 && UtilsIsLeapYear(tm.tm_year + 1901)))
                    reminder->mStartTime += 24 * 3600;
                break;
            case DvbReminder::eOnce:
            default:
                delDvbReminder(reminder->mBookId);
        }
    }
    if (bookIds.size() > 0)
        saveDB(eDvbReminderDB);

    if (mReminders.size() > 0) {
        DvbReminderIter it = mReminders.begin();
        time_t min = it->second->mStartTime;
        for (; it != mReminders.end(); ++it) {
            if (it->second->mStartTime < min)
                min = it->second->mStartTime;
        }
        sendEmptyMessageDelayed(eDvbMessageBookReminder, (min - time(0)) * 1000);
    }
}

void
DvbManager::setDvbDateTime(time_t utcTime, int timeOffset, int nextOffset, time_t changeTime)
{
    LogDvbDebug("utcTime[%ld] timeOffset[%d] nextOffset[%d] changeTime[%ld]\n", utcTime, timeOffset, nextOffset, changeTime);
    if (!mid_ntp_status()) {
        if (timeOffset < nextOffset) {
            yu_setTimeZone(timeOffset / 3600.0);
            //appSettingSetInt("lightstart", changeTime);
        } else {
            yu_setTimeZone(nextOffset / 3600.0);
            //appSettingSetInt("lightstop", changeTime);
        }
        //appSettingSetInt("jumpstep", abs(timeOffset - nextOffset));
        struct timeval tv;
        tv.tv_sec  = utcTime;
        tv.tv_usec = 0;
        settimeofday(&tv, NULL);
        tzset();
    } else {
        mDataTimeOffset = time(0) - utcTime;
        LogDvbDebug("date time offset [%d]\n", mDataTimeOffset);
    }
    mDateTimeSyncOk = true;
}

void
DvbManager::show()
{
    LogDvbDebug("DvbDevice[%d] Transponder[%d] DvbChannel[%d] DvbScan[%d]\n", mDvbDevices.size(), mTransponders.size(), mDvbChannels.size(), mDvbScans.size());
    for (DvbDeviceIter it = mDvbDevices.begin(); it != mDvbDevices.end(); ++it)
        it->second->show();
    for (TransponderIter it = mTransponders.begin(); it != mTransponders.end(); ++it)
        it->second->show();
    for (DvbChannelIter it = mDvbChannels.begin(); it != mDvbChannels.end(); ++it)
        it->second->show();
    for (DvbReminderIter it = mReminders.begin(); it != mReminders.end(); ++it)
        it->second->show();
    for (DvbScanIter it = mDvbScans.begin(); it != mDvbScans.end(); ++it)
        it->second->show();
}

int DvbChannelParseAdd(Hippo::ProgramChannelC20* program)
{
    const int defTunerId = 1;
    const int defDiseqcType = 0;
    const ToneMode_e defToneMode = eToneOff;
    const DiseqcPort_e defDiseqcPort = eDiseqcPortA;

    int deviceKey = UtilsCreateKey(defTunerId, defToneMode, defDiseqcPort, program->GetDVB_SatLocation());
    DvbDeviceDVBS* device = static_cast<DvbDeviceDVBS*>(getDvbManager().getDvbDevice(deviceKey));
    if (!device) {
        if ((device = new DvbDeviceDVBS())) {
            device->mFavorFlag  = 1;
            device->mTunerId    = defTunerId;
            device->mDiseqcType = defDiseqcType;
            device->mDiseqcPort = defDiseqcPort;
            device->mToneMode   = defToneMode;
            device->mSateliteName.assign(program->GetDVB_SatName().c_str());
            device->mSatLongitude.assign(UtilsInt2Str(program->GetDVB_SatLocation()));
            device->mDeviceKey  = deviceKey;
            if (eDvbOk != getDvbManager().addDvbDevice(device)) {
                delete(device);
                return eDvbError;
            }
        }
    }

    int transponderKey = UtilsCreateKey(program->GetDVB_TpFreq(), program->GetDVB_Symb(), (Polarization_e)program->GetDVB_Polarity(), 0);
    TransponderDVBS* transponder = static_cast<TransponderDVBS*>(getDvbManager().getTransponder(transponderKey));
    if (!transponder) {
        if ((transponder = new TransponderDVBS(device))) {
            transponder->mFrequency    = program->GetDVB_TpFreq();
            transponder->mSymbolRate   = program->GetDVB_Symb();
            transponder->mPolarization = (Polarization_e)program->GetDVB_Polarity();
            transponder->mUniqueKey    = transponderKey;
            if (eDvbOk != getDvbManager().addTransponder(transponder)) {
                delete(transponder);
                return eDvbError;
            }
        }
    }

    int channelKey = UtilsCreateKey(program->GetDVB_ProgNum(), program->GetDVB_PMT_PID(), program->GetDVB_OrigNetID(), 0);
    DvbChannel* channel = getDvbManager().getDvbChannel(channelKey);
    if (!channel) {
        if ((channel = new DvbChannel(transponder))) {
            channel->mServiceId         = program->GetDVB_ProgNum();
            channel->mProgramMapPid     = program->GetDVB_PMT_PID();
            channel->mServiceType       = eServiceAll;
            channel->mChannelType       = eChannelAll ;
            channel->mChannelName       = program->GetChanName().c_str();
            channel->mTransportStreamId = program->GetDVB_PMT_PID();
            channel->mOriginalNetworkId = program->GetDVB_OrigNetID();
            channel->mChannelKey = channelKey;
            if (eDvbOk != getDvbManager().addDvbChannel(channel)) {
                delete(channel);
                return eDvbError;
            }
        }
    }
    return eDvbOk;
}

extern "C" int dvb_player_pvr_demux_lock(int tunerId, int key)
{
    LogDvbDebug("tunerId %d key %d\n", tunerId, key);
    DvbChannel* channel = getDvbManager().getDvbChannel(key);
    if (!channel)
        return -1;
    getDvbManager().setCurrentChannel(channel);
    getDvbManager().setCurrentTunerId(tunerId);
    return getDvbManager().getFrontend()->tune(tunerId, channel->mTransponder) ? 1 : 0;
}
