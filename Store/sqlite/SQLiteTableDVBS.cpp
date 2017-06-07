#include "DvbAssertions.h"
#include "SQLiteTableDVBS.h"
#include "SQLiteDatabase.h"
#include "DvbDevice.h"
#include "Transponder.h"
#include "DvbChannel.h"
#include "DvbManager.h"

//DvbDevice SQL
static const int kDevTableVersion = 1;
static const char* kDevCreateSQL = "CREATE TABLE devices(DeviceKey INT PRIMARYKEY, TunerId INT, FavorFlag INT, LnbType INT, LOF1 INT, LOF2 INT, ToneMode INT, DiseqcType INT, DiseqcPort INT, MotorType INT, SatLongitude TEXT, SatelliteName TEXT)";
static const char* kDevInsertSQL = "INSERT INTO devices(DeviceKey, TunerId, FavorFlag, LnbType, LOF1, LOF2, ToneMode, DiseqcType, DiseqcPort, MotorType, SatLongitude, SatelliteName) VALUES(?,?,?,?,?,?,?,?,?,?,?,?)";
static const char* kDevSelectSQL = "SELECT DeviceKey, TunerId, FavorFlag, LnbType, LOF1, LOF2, ToneMode, DiseqcType, DiseqcPort, MotorType, SatLongitude, SatelliteName FROM devices";
static const char* kDevDeleteSQL = "DELETE FROM devices";

//Transponder SQL
static const int kTpsTableVersion = 1;
static const char* kTpsCreateSQL = "CREATE TABLE transponders(TransponderKey INT PRIMARYKEY, DeviceKey INT, IFrequency INT, Frequency INT, SymbolRate INT, Polarization INT, BandWidth INT, Modulation INT, FecRate INT)";
static const char* kTpsInsertSQL = "INSERT INTO transponders(TransponderKey, DeviceKey, IFrequency, Frequency, SymbolRate, Polarization, BandWidth, Modulation, FecRate) VALUES(?,?,?,?,?,?,?,?,?)";
static const char* kTpsSelectSQL = "SELECT TransponderKey, DeviceKey, IFrequency, Frequency, SymbolRate, Polarization, BandWidth, Modulation, FecRate FROM transponders";
static const char* kTpsDeleteSQL = "DELETE FROM transponders";

//DvbChannel SQL
static const int kChlTableVersion = 1;
static const char* kChlCreateSQL = "CREATE TABLE dvbchannels(ChannelKey INT PRIMARYKEY, TransponderKey INT, ChannelName TEXT, ServiceType INT, PmtPid INT, TransportStreamId INT, OriginalNetworkId INT, ServiceId INT, IsEncrypt INT)";
static const char* kChlInsertSQL = "INSERT INTO dvbchannels(ChannelKey, TransponderKey, ChannelName, ServiceType, PmtPid, TransportStreamId, OriginalNetworkId, ServiceId, IsEncrypt) VALUES(?,?,?,?,?,?,?,?,?)";
static const char* kChlSelectSQL = "SELECT ChannelKey, TransponderKey, ChannelName, ServiceType, PmtPid, TransportStreamId, OriginalNetworkId, ServiceId, IsEncrypt FROM dvbchannels";
static const char* kChlDeleteSQL = "DELETE FROM dvbchannels";

//DvbReminder SQL
static const int kRemTableVersion = 1;
static const char* kRemCreateSQL = "CREATE TABLE reminders(BookID INT PRIMARY KEY, PeriodType INT, ChannelKey INT, ProgramID INT, StartTime BIGINT, ExtendDes TEXT)";
static const char* kRemInsertSQL = "INSERT INTO reminders(BookID , PeriodType, ChannelKey, ProgramID, StartTime, ExtendDes) VALUES(?,?,?,?,?,?)";
static const char* kRemSelectSQL = "SELECT BookID , PeriodType, ChannelKey, ProgramID, StartTime, ExtendDes FROM reminders";
static const char* kRemDeleteSQL = "DELETE FROM reminders";

SQLiteTableDVBS::SQLiteTableDVBS(DvbManager* manager) : SQLiteTableDVB(manager)
{
    if (!_TableExists("devices")) {
        if (!mDB->exec(kDevCreateSQL))
            return;
        setTableVersion("devices", kDevTableVersion);
    } else {
        //TODO
        if (kDevTableVersion != getTableVersion("devices"))
            LogDvbWarn("device table need update\n");
        else 
            LogDvbDebug("device table not need update\n");
    }

    if (!_TableExists("transponders")) {
        if (!mDB->exec(kTpsCreateSQL))
            return;
        setTableVersion("transponders", kTpsTableVersion);
    } else {
        //TODO
        if (kDevTableVersion != getTableVersion("transponders"))
            LogDvbWarn("transponders table need update\n");
        else 
            LogDvbDebug("transponders table not need update\n");
    }

    if (!_TableExists("dvbchannels")) {
        if (!mDB->exec(kChlCreateSQL))
            return;
        setTableVersion("dvbchannels", kChlTableVersion);
    } else {
        //TODO
        if (kDevTableVersion != getTableVersion("dvbchannels"))
            LogDvbWarn("dvbchannels table need update\n");
        else 
            LogDvbDebug("dvbchannels table not need upate\n");
    }

    if (!_TableExists("reminders")) {
        if (!mDB->exec(kRemCreateSQL))
            return;
        setTableVersion("reminders", kRemTableVersion);
    } else {
        //TODO
        if (kDevTableVersion != getTableVersion("reminders"))
            LogDvbWarn("reminders table need update\n");
        else 
            LogDvbDebug("reminders table not need upate\n");
    }
}

SQLiteTableDVBS::~SQLiteTableDVBS()
{
}

bool SQLiteTableDVBS::load()
{
    LogDvbDebug("sqlite load\n");
    SQLiteResultSet* rs = NULL;
    //DvbDevice Select SQL
    if (!(rs = mDB->query(kDevSelectSQL))) {
        LogDvbError("SQL %s\n", kDevSelectSQL);
        return false;
    }
    while (rs->next()) {
        DvbDeviceDVBS* device = new DvbDeviceDVBS();
        if (device) {
            // DeviceKey, TunerId, FavorFlag, LnbType, LOF1, LOF2, ToneMode, DiseqcType, DiseqcPort, MotorType, SatLongitude, SatelliteName
            device->mDeviceKey    = rs->columnInt(0);
            device->mTunerId      = rs->columnInt(1);
            device->mFavorFlag    = rs->columnInt(2);
            device->mLnbType      = static_cast<LnbType_e>(rs->columnInt(3));
            device->mLOF1         = rs->columnInt(4);
            device->mLOF2         = rs->columnInt(5);
            device->mToneMode     = static_cast<ToneMode_e>(rs->columnInt(6));
            device->mDiseqcType   = rs->columnInt(7);
            device->mDiseqcPort   = static_cast<DiseqcPort_e>(rs->columnInt(8));
            device->mMotorType    = rs->columnInt(9);
            device->mSatLongitude = rs->columnText(10);
            device->mSateliteName = rs->columnText(11);
            //LogDvbDebug("add EquipmentKey[%d] from sqlite\n", device->mDeviceKey);
            mManager->addDvbDevice(device); 
        }
    }
    rs->close();

    //Transponder Select SQL
    if (!(rs = mDB->query(kTpsSelectSQL))) {
        LogDvbError("SQL %s\n", kTpsSelectSQL);
        return false;
    }
    while (rs->next()) {
        DvbDevice* device = mManager->getDvbDevice(rs->columnInt(1));
        if (device) {
            // TransponderKey, DeviceKey, IFrequency, Frequency, SymbolRate, Polarization, BandWidth, Modulation, FecRate
            TransponderDVBS* transponder = new TransponderDVBS(device); 
            transponder->mUniqueKey    = rs->columnInt(0);
            transponder->mIFrequency   = rs->columnInt(2);
            transponder->mFrequency    = rs->columnInt(3);
            transponder->mSymbolRate   = rs->columnInt(4);
            transponder->mPolarization = static_cast<Polarization_e>(rs->columnInt(5));
            transponder->mBandWidth    = rs->columnInt(6); 
            transponder->mModulation   = static_cast<Modulation_e>(rs->columnInt(7));
            transponder->mFecRate      = static_cast<FecRate_e>(rs->columnInt(8));
            //LogDvbDebug("add TransponderKey[%d] from sqlite\n", transponder->mUniqueKey);
            mManager->addTransponder(transponder);
        }
    }
    rs->close();

    //DvbChannel Select SQL
    if (!(rs = mDB->query(kChlSelectSQL))) {
        LogDvbError("SQL %s\n", kChlSelectSQL);
        return false;
    }
    while (rs->next()) {
        Transponder* transponder = mManager->getTransponder(rs->columnInt(1));
        if (transponder) {
            // ChannelKey, TransponderKey, ChannelName, ServiceType, PmtPid, TransportStreamId, OriginalNetworkId, ServiceId, IsEncrypt
            DvbChannel* channel = new DvbChannel(transponder);
            channel->mChannelKey        = rs->columnInt(0);
            channel->mChannelName       = rs->columnText(2);
            channel->mServiceType       = static_cast<ServiceType_e>(rs->columnInt(3));
            channel->mProgramMapPid     = rs->columnInt(4);
            channel->mTransportStreamId = rs->columnInt(5);
            channel->mOriginalNetworkId = rs->columnInt(6);
            channel->mServiceId         = rs->columnInt(7);
            channel->mChannelType       = static_cast<ChannelType_e>(rs->columnInt(8));
            //LogDvbDebug("add ChannelKey[%d] from sqlite\n", channel->mChannelKey);
            mManager->addDvbChannel(channel);
        }
    }
    rs->close();

    //Reminder Select SQL
    if (!(rs = mDB->query(kRemSelectSQL))) {
        LogDvbError("SQL %s\n", kRemSelectSQL);
        return false;
    }
    while (rs->next()) {
        //BookID , PeriodType, ChannelKey, ProgramID, StartTime, ExtendDes 
        DvbReminder* reminder = new DvbReminder();
        reminder->mBookId     = rs->columnInt(0);
        reminder->mPeriodType = rs->columnInt(1);
        reminder->mChannelKey = rs->columnInt(2);
        reminder->mEventId    = rs->columnInt(3);
        reminder->mStartTime  = rs->columnInt64(4);
        reminder->mExtendDes  = rs->columnText(5);
        //LogDvbDebug("add BookId[%d] from sqlite\n", reminder->mBookId);
        mManager->addDvbReminder(reminder);
    }
    return true;
}

bool SQLiteTableDVBS::save(int flag)
{
    LogDvbDebug("sqlite save %02x\n", flag);
    //DvbDevice Insert SQL
    if ((flag & eDvbDeviceDB)) {
        mDB->exec(kDevDeleteSQL);
        mDB->cacheSQL(kDevInsertSQL);
        std::vector<int> devlist = mManager->getDvbDeviceKeys();
        for (std::size_t i = 0; i < devlist.size(); ++i) {
            DvbDeviceDVBS* device = static_cast<DvbDeviceDVBS*>(mManager->getDvbDevice(devlist[i]));
            if (!device)
                continue;
            SQLiteValue values[12];
            // DeviceKey, TunerId, FavorFlag, LnbType, LOF1, LOF2, ToneMode, DiseqcType, DiseqcPort, MotorType, SatLongitude, SatelliteName
            values[0]  = SQLInt(device->mDeviceKey);
            values[1]  = SQLInt(device->mTunerId);
            values[2]  = SQLInt(device->mFavorFlag);
            values[3]  = SQLInt(device->mLnbType);
            values[4]  = SQLInt(device->mLOF1);
            values[5]  = SQLInt(device->mLOF2);
            values[6]  = SQLInt(device->mToneMode);
            values[7]  = SQLInt(device->mDiseqcType);
            values[8]  = SQLInt(device->mDiseqcPort);
            values[9]  = SQLInt(device->mMotorType);
            values[10] = SQLText(device->mSatLongitude.c_str());
            values[11] = SQLText(device->mSateliteName.c_str());
            mDB->exec(kDevInsertSQL, values, 12);
        }
        mDB->uncacheSQL(kDevInsertSQL);
    }

    //Transponder Insert SQL
    if ((flag & eTransponderDB)) {
        mDB->exec(kTpsDeleteSQL);
        mDB->cacheSQL(kTpsInsertSQL);
        std::vector<int> tpslist = mManager->getTransponderKeys();
        for (std::size_t i = 0; i < tpslist.size(); ++i) {
            TransponderDVBS* transponder = static_cast<TransponderDVBS*>(mManager->getTransponder(tpslist[i]));
            if (!transponder)
                continue;
            SQLiteValue values[9];
            // TransponderKey, DeviceKey, IFrequency, Frequency, SymbolRate, Polarization, BandWidth, Modulation, FecRate
            values[0] = SQLInt(transponder->mUniqueKey);
            values[1] = SQLInt(transponder->getDvbDeviceKey());
            values[2] = SQLInt(transponder->mIFrequency);
            values[3] = SQLInt(transponder->mFrequency);
            values[4] = SQLInt(transponder->mSymbolRate);
            values[5] = SQLInt(transponder->mPolarization);
            values[6] = SQLInt(transponder->mBandWidth);
            values[7] = SQLInt(transponder->mModulation);
            values[8] = SQLInt(transponder->mFecRate);
            mDB->exec(kTpsInsertSQL, values, 9);
        }
        mDB->uncacheSQL(kTpsInsertSQL);
    }

    //DvbChannel Insert SQL 
    if ((flag & eDvbChannelDB)) {
        mDB->exec(kChlDeleteSQL);
        mDB->cacheSQL(kChlInsertSQL);
        std::vector<int> chnllist = mManager->getDvbChannelKeys();
        for (std::size_t i = 0; i < chnllist.size(); ++i) {
            DvbChannel* channel = mManager->getDvbChannel(chnllist[i]);
            if (!channel)
                continue;
            SQLiteValue values[9];
            // ChannelKey, TransponderKey, ChannelName, ServiceType, PmtPid, TransportStreamId, OriginalNetworkId, ServiceId, IsEncrypt
            values[0] = SQLInt(channel->mChannelKey);
            values[1] = SQLInt(channel->getTransponderKey());
            values[2] = SQLText(channel->mChannelName.c_str());
            values[3] = SQLInt(channel->mServiceType);
            values[4] = SQLInt(channel->mProgramMapPid);
            values[5] = SQLInt(channel->mTransportStreamId);
            values[6] = SQLInt(channel->mOriginalNetworkId);
            values[7] = SQLInt(channel->mServiceId);
            values[8] = SQLInt(channel->mChannelType);
            mDB->exec(kChlInsertSQL, values, 9);
        }
        mDB->uncacheSQL(kChlInsertSQL);
    }

    //Reminder InsertSQL
    if ((flag & eDvbReminderDB)) {
        mDB->exec(kRemDeleteSQL);
        mDB->cacheSQL(kRemInsertSQL);
        std::vector<int> remlist = mManager->getDvbReminderKeys();
        for (std::size_t i = 0; i < remlist.size(); ++i) {
            DvbReminder* reminder = mManager->getDvbReminder(remlist[i]);
            if (!reminder)
                continue;
            SQLiteValue values[6];
            //BookID , PeriodType, ChannelKey, ProgramID, StartTime, ExtendDes 
            values[0] = SQLInt(reminder->mBookId);
            values[1] = SQLInt(reminder->mPeriodType);
            values[2] = SQLInt(reminder->mChannelKey);
            values[3] = SQLInt(reminder->mEventId);
            values[4] = SQLInt64(reminder->mStartTime);
            values[5] = SQLText(reminder->mExtendDes.c_str());
            mDB->exec(kRemInsertSQL, values, 6);
        }
        mDB->uncacheSQL(kRemInsertSQL);
    }
    return true;
 }
