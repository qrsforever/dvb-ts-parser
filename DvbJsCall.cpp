#include "DvbAssertions.h"
#include "DvbJsCall.h"
#include "DvbEnum.h"
#include "DvbManager.h"
#include "DvbScanEpg.h"
#include "DvbChannel.h"
#include "DvbScanChannel.h"
#include "DvbAntennaSetup.h"
#include "Transponder.h"
#include "Frontend.h"
#include "FrontendDVBS.h"
#include "DvbDevice.h"
#include "DvbBlindScan.h"
#include "DvbEpg.h"

#include "AppSetting.h"
#include "SysSetting.h"

#include "mid/mid_time.h"
#include "mid/mid_tools.h"
#include "json/json.h"
#include "json/json_object.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

typedef enum DvbSearch_ {
    eDvbScanIdle = 0,
	eDvbAntennaSetup,
	eDvbBlindScan,
	eDvbScanChannelBySat,
	eDvbScanChannelByTps,
}DvbSearchType_t;

static DvbSearchType_t gDvbScanType = eDvbScanIdle;

static int _JseRead_dvb_sat_get_count(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen )
{
	return sprintf(jseValue, "%d", getDvbManager().getDvbDevicesCount());
}

/*
 * 获取第一个卫星的信息
 *      var sJson = Utility.getValueByName("dvb_sat_get_list,{"position":0,"count":1}")
 * STB 返回
 *      {"satCount":1,"satList”:"[{"satID":1,"tunerID":0,"satName":"AA", "satLongitude":"-60.5","lnbType":0,"lnbFreqLow":0,"lnbFreqHigh":0,"b22K":0,"diSEqCType":0,"diSEqCPort":0,"motorType":0,"favoriteFlag":0}]"}
 */
static int _JseRead_dvb_sat_get_list(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen )
{
    struct json_object* obj = json_tokener_parse(jseParam);
    if (!obj)
        return -1;
    int position = json_object_get_int(json_object_object_get(obj, "position"));
    int count    = json_object_get_int(json_object_object_get(obj, "count"));
    json_object_put(obj);

    struct json_object* jObject = json_object_new_object();
    struct json_object* jArray  = json_object_new_array();

    std::vector<int> deviceKeys = getDvbManager().getDvbDeviceKeys();
    DvbDeviceDVBS* device = 0;
    int satCount = 0;
    struct json_object* jSatList = 0;
    for (std::size_t i = position; satCount < count && i < deviceKeys.size(); ++i) {
        device = static_cast<DvbDeviceDVBS*>(getDvbManager().getDvbDevice(deviceKeys[i]));
        if (device && (jSatList = json_object_new_object())) {
            json_object_object_add(jSatList, "satID",        json_object_new_int(device->mDeviceKey));
            json_object_object_add(jSatList, "tunerID",      json_object_new_int(device->mTunerId));
            json_object_object_add(jSatList, "satName",      json_object_new_string(device->mSateliteName.c_str()));
            json_object_object_add(jSatList, "satLongitude", json_object_new_string(device->mSatLongitude.c_str()));
            json_object_object_add(jSatList, "lnbType",      json_object_new_int(device->mLnbType));
            json_object_object_add(jSatList, "lnbFreqLow",   json_object_new_int(device->mLOF1 / 1000));
            json_object_object_add(jSatList, "lnbFreqHigh",  json_object_new_int(device->mLOF2 / 1000));
            json_object_object_add(jSatList, "b22K",         json_object_new_int(device->mToneMode));
            json_object_object_add(jSatList, "diSEqCType",   json_object_new_int(device->mDiseqcType));
            if (device->mDiseqcType)
                json_object_object_add(jSatList, "diSEqCPort",   json_object_new_int(device->mDiseqcPort));
            json_object_object_add(jSatList, "motorType",    json_object_new_int(device->mMotorType));
            json_object_object_add(jSatList, "favoriteFlag", json_object_new_int(device->mFavorFlag));
            json_object_array_add(jArray, jSatList);
            satCount++;
        }
    }
    json_object_object_add(jObject,"satCount", json_object_new_int(satCount));
	json_object_object_add(jObject,"satList",  jArray);

	strncpy(jseValue, json_object_to_json_string(jObject), JSE_BUFFER_SIZE);
	json_object_put(jObject);
	LogDvbDebug("jseValue = %s\n", jseValue);
    return 0;
}

static int _JseRead_dvb_my_sat_get_count(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
	return sprintf(jseValue, "%d", getDvbManager().getDvbDevicesCount(1));
}

/*
 *  获取第一个卫星的信息
 *      var sJson = Utility.getValueByName("dvb_my_sat_get_list,{“position”:0,”count”:1}”)
 *  STB 返回
 *      {"satCount":1,”satList":"[{"satID":1,"tunerID":0,"satName":"AA","satLongitude":"60.1","lnbType":0,"lnbFreqLow":0,”lnbFreqHigh":0,"b22K":0,"diSEqCType":0,"diSEqCPort":0,"motorType":0}]"}
 */
static int _JseRead_dvb_my_sat_get_list(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen )
{
    struct json_object* obj = json_tokener_parse(jseParam);
    if (!obj)
        return -1;
    int position = json_object_get_int(json_object_object_get(obj, "position"));
    int count    = json_object_get_int(json_object_object_get(obj, "count"));
    json_object_put(obj);

    struct json_object* jObject = json_object_new_object();
    struct json_object* jArray  = json_object_new_array();

    std::vector<int> deviceKeys = getDvbManager().getDvbDeviceKeys(1);
    DvbDeviceDVBS* device = 0;
    int satCount = 0;
    struct json_object* jSatList = 0;
    for (std::size_t i = position; satCount < count && i < deviceKeys.size(); ++i) {
        device = static_cast<DvbDeviceDVBS*>(getDvbManager().getDvbDevice(deviceKeys[i]));
        if (device && (jSatList = json_object_new_object())) {
            json_object_object_add(jSatList, "satID",        json_object_new_int(device->mDeviceKey));
            json_object_object_add(jSatList, "tunerID",      json_object_new_int(device->mTunerId));
            json_object_object_add(jSatList, "satName",      json_object_new_string(device->mSateliteName.c_str()));
            json_object_object_add(jSatList, "satLongitude", json_object_new_string(device->mSatLongitude.c_str()));
            json_object_object_add(jSatList, "lnbType",      json_object_new_int(device->mLnbType));
            json_object_object_add(jSatList, "lnbFreqLow",   json_object_new_int(device->mLOF1 / 1000));
            json_object_object_add(jSatList, "lnbFreqHigh",  json_object_new_int(device->mLOF2 / 1000));
            json_object_object_add(jSatList, "b22K",         json_object_new_int(device->mToneMode));
            json_object_object_add(jSatList, "diSEqCType",   json_object_new_int(device->mDiseqcType));
            if (device->mDiseqcType)
                json_object_object_add(jSatList, "diSEqCPort",   json_object_new_int(device->mDiseqcPort));
            json_object_object_add(jSatList, "motorType",    json_object_new_int(device->mMotorType));
            json_object_array_add(jArray, jSatList);
            satCount++;
        }
    }
    json_object_object_add(jObject,"satCount", json_object_new_int(satCount));
	json_object_object_add(jObject,"satList",  jArray);

	strncpy(jseValue, json_object_to_json_string(jObject), JSE_BUFFER_SIZE);
	json_object_put(jObject);
	LogDvbDebug("jseValue = %s\n", jseValue);
    return 0;
}

/*
 *  保存卫星参数
 *      Utility.setValueByName(‘dvb_sat_save_params’, ‘{ “satID”:1,”tunerID”:0,”satName”:”AA”,“satLongitude”:”60”,”lnbType”:0,”lnbFreqLow”:0,”lnbFreqHigh”:0,”b22K”:0,”diSEqCType”:0,”diSEqCPort”:0,”motorType”:0,”favoriteFlag”:0}’);
 */
static int _JseWrite_dvb_sat_save_params(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseValue: %s\n", jseValue);
    struct json_object* obj = json_tokener_parse(jseValue);
    if (!obj)
        return -1;
    DvbDeviceDVBS* device = 0;
    struct json_object* lookup = json_object_object_get(obj, "satID");
    if (!lookup)
        goto Err;
    device = static_cast<DvbDeviceDVBS*>(getDvbManager().getDvbDevice(json_object_get_int(lookup)));
    if (!device)
        goto Err;
    if ((lookup = json_object_object_get(obj, "tunerID")))      device->mTunerId      = json_object_get_int(lookup);
    if ((lookup = json_object_object_get(obj, "satName")))      device->mSateliteName = json_object_get_string(lookup);
    if ((lookup = json_object_object_get(obj, "satLongitude"))) device->mSatLongitude = json_object_get_string(lookup);
    if ((lookup = json_object_object_get(obj, "lnbType")))      device->mLnbType      = (LnbType_e)json_object_get_int(lookup);
    if ((lookup = json_object_object_get(obj, "lnbFreqLow")))   device->mLOF1         = json_object_get_int(lookup) * 1000;
    if ((lookup = json_object_object_get(obj, "lnbFreqHigh")))  device->mLOF2         = json_object_get_int(lookup) * 1000;
    if ((lookup = json_object_object_get(obj, "b22K")))         device->mToneMode     = (ToneMode_e)json_object_get_int(lookup);
    if ((lookup = json_object_object_get(obj, "diSEqCType")))   device->mDiseqcType   = json_object_get_int(lookup);
    if ((lookup = json_object_object_get(obj, "diSEqCPort")))   device->mDiseqcPort   = (DiseqcPort_e)json_object_get_int(lookup);
    if ((lookup = json_object_object_get(obj, "motorType")))    device->mMotorType    = json_object_get_int(lookup);
    if ((lookup = json_object_object_get(obj, "favoriteFlag"))) device->mFavorFlag    = json_object_get_int(lookup);

    json_object_put(obj);
    getDvbManager().saveDB(eDvbDeviceDB);
    return 0;
Err:
    json_object_put(obj);
    return -1;
}

/*
 * 添加一个名称为 AA,经度为东经 60 度的卫星
 *      Utility.setValueByName(‘dvb_sat_add’, ‘{“satName”: “AA”,“satLongitude”:”60”}’);
 */
static int _JseWrite_dvb_sat_add(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseValue: %s\n", jseValue);
    struct json_object* obj = json_tokener_parse(jseValue);
    if (!obj)
        return -1;
    std::string tSateliteName = json_object_get_string(json_object_object_get(obj, "satName"));
    std::string tSatLongitude = json_object_get_string(json_object_object_get(obj, "satLongitude"));
    json_object_put(obj);

    DvbDeviceDVBS* device = 0;
    std::vector<int> deviceKeys = getDvbManager().getDvbDeviceKeys();
    for (std::size_t i = 0; i < deviceKeys.size(); ++i) {
         device = static_cast<DvbDeviceDVBS*>(getDvbManager().getDvbDevice(deviceKeys[i]));
         if (!device)
             continue;
         if (device->mSateliteName == tSateliteName)
             return -2;
         if (device->mSatLongitude == tSatLongitude)
             return -3;
    }
    device = new DvbDeviceDVBS();
    device->mDeviceKey =  UtilsCreateKey(device->mTunerId, device->mDiseqcPort, device->mToneMode, atoi(tSatLongitude.c_str()));
    device->mSateliteName = tSateliteName;
    device->mSatLongitude = tSatLongitude;
    getDvbManager().addDvbDevice(device);
    getDvbManager().saveDB(eDvbDeviceDB);
    return 0;
}

/*
 *  删除 ID 为 1 的卫星
 *      Utility.setValueByName(‘dvb_sat_delete’, ‘{“satID”: 1 }’);
 */
static int _JseWrite_dvb_sat_delete(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseValue: %s\n", jseValue);
    struct json_object* obj = json_tokener_parse(jseValue);
    if (!obj)
        return -1;

    struct json_object* lookup = json_object_object_get(obj, "satID");
    if (!lookup)
        goto Err;
    getDvbManager().delDvbDevice(json_object_get_int(lookup));
    getDvbManager().saveDB(eDvbDeviceDB);
    json_object_put(obj);
    return 0;
Err:
    json_object_put(obj);
    return -1;
}

/*
 *  查询 ID 为 1 的卫星下的 TP 个数
 *      Utility.getValueByName(‘dvb_tp_get_count,{“satID”:1}’)
 *  查询所有卫星下的 TP 个数
 *      Utility.getValueByName(‘dvb_tp_get_count,{“satID”:0}’)
 *  */
static int _JseRead_dvb_tp_get_count(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    struct json_object* obj = json_tokener_parse(jseParam);
    if (!obj)
        return -1;
    int satId = json_object_get_int(json_object_object_get(obj, "satID"));
    json_object_put(obj);
    return sprintf(jseValue, "%d", getDvbManager().getTranspondersCount(satId));
}

/*
 *  获取所有卫星下第一个 TP 的信息:
 *      Utility.getValueByName(‘dvb_tp_get_list,{”satID”:-1 ,“position”:0,”count”:1}’);
 *  STB 返回:
 *      {“tpNum”:1,”tpList”:[{“tpID”:1,”satID”:2,“frequency”:11200, “symbolRate”:27500,”polarization”:0}]}
 *  获取 ID 为 2 的卫星下的 TP 信息
 *      Utility.getValueByName(‘dvb_tp_get_list,{ ”satID”:2,“position”:0,”count”:1}’);
 *  */
static int _JseRead_dvb_tp_get_list(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseParam: %s\n", jseParam);
    struct json_object* obj = json_tokener_parse(jseParam);
    if (!obj)
        return -1;
    int satId    = json_object_get_int(json_object_object_get(obj, "satID"));
    int position = json_object_get_int(json_object_object_get(obj, "position"));
    int count    = json_object_get_int(json_object_object_get(obj, "count"));
    json_object_put(obj);

    struct json_object* jObject = json_object_new_object();
    struct json_object* jArray  = json_object_new_array();

    std::vector<int> transponderKeys = getDvbManager().getTransponderKeys(satId);
    TransponderDVBS* transponder = 0;
    int tpNum = 0;
    struct json_object* jTpsList = 0;
    for (std::size_t i = position; tpNum < count && i < transponderKeys.size(); ++i) {
        transponder = static_cast<TransponderDVBS*>(getDvbManager().getTransponder(transponderKeys[i]));
        if (transponder && (jTpsList = json_object_new_object())) {
            json_object_object_add(jTpsList, "tpID",         json_object_new_int(transponder->mUniqueKey));
            json_object_object_add(jTpsList, "satID",        json_object_new_int(transponder->getDvbDeviceKey()));
            json_object_object_add(jTpsList, "frequency",    json_object_new_int(transponder->mFrequency / 1000));
            json_object_object_add(jTpsList, "symbolRate",   json_object_new_int(transponder->mSymbolRate));
            json_object_object_add(jTpsList, "polarization", json_object_new_int((Polarization_e)transponder->mPolarization));
            json_object_array_add(jArray, jTpsList);
            tpNum++;
        }
    }
    json_object_object_add(jObject, "tpNum", json_object_new_int(tpNum));
    json_object_object_add(jObject, "tpList", jArray);

    strncpy(jseValue, json_object_to_json_string(jObject), JSE_BUFFER_SIZE);
    json_object_put(jObject);
    LogDvbDebug("jseValue = %s\n", jseValue);
    return 0;
}

/*
 * 修改 ID 为 2 的 TP 参数后保存
 *      Utility.setValueByName(‘dvb_tp_save_params’, ‘{“tpID”:2, “frequency”:11200,“symbolRate”:27500,“polarization”:0 }’);
 *  */
static int _JseWrite_dvb_tp_save_params(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseValue: %s\n", jseValue);
    struct json_object* obj = json_tokener_parse(jseValue);
    if (!obj)
        return -1;
    TransponderDVBS* transponder = 0;
    struct json_object* lookup = json_object_object_get(obj, "tpID");
    if (!lookup)
        goto Err;

    transponder = static_cast<TransponderDVBS*>(getDvbManager().getTransponder((json_object_get_int(lookup))));
    if (!transponder)
        goto Err;
    if ((lookup = json_object_object_get(obj, "frequency")))    transponder->mFrequency    = json_object_get_int(lookup) * 1000;
    if ((lookup = json_object_object_get(obj, "symbolRate")))   transponder->mSymbolRate   = json_object_get_int(lookup);
    if ((lookup = json_object_object_get(obj, "polarization"))) transponder->mPolarization = (Polarization_e)json_object_get_int(lookup);
    getDvbManager().saveDB(eDvbDeviceDB);
    json_object_put(obj);
    return 0;
Err:
    json_object_put(obj);
    return -1;
 }

/*
 *  为 ID 为 2 的卫星添加一个TP
 *      Utility.setValueByName(‘dvb_tp_add’,’{“satID”:2, “frequency”:11200, “symbolRate”:27500,“polarization”:0 }’);
 *  */
static int _JseWrite_dvb_tp_add(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseValue: %s\n", jseValue);
    struct json_object* obj = json_tokener_parse(jseValue);
    if (!obj)
        return -1;
    int tSatId        = json_object_get_int(json_object_object_get(obj, "staID"));
    int tFrequency    = json_object_get_int(json_object_object_get(obj, "frequency"));
    int tSymbolrate   = json_object_get_int(json_object_object_get(obj, "symbolRate"));
    int tPolarization = json_object_get_int(json_object_object_get(obj, "polarization"));
    int tUniqueKey    = UtilsCreateKey(tFrequency, tSymbolrate, tPolarization, 0);
    json_object_put(obj);

    TransponderDVBS* transponder = 0;
    DvbDevice* device = getDvbManager().getDvbDevice(tSatId);
    if (!device)
        goto Err;
    if (getDvbManager().getTransponder(tUniqueKey))
        return -2;
    transponder = new TransponderDVBS(device);
    transponder->mUniqueKey    = tUniqueKey;
    transponder->mFrequency    = tFrequency * 1000;
    transponder->mSymbolRate   = tSymbolrate;
    transponder->mPolarization = (Polarization_e)tPolarization;
    getDvbManager().addTransponder(transponder);
    getDvbManager().saveDB(eTransponderDB);
    return 0;
Err:
    return -1;
}

/*
 * 删除 ID 为 2 的 TP
 *     Utility.setValueByName(‘dvb_tp_delete’, ‘{“tpID”:2}’);
 *  */
static int _JseWrite_dvb_tp_delete( const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseValue: %s\n", jseValue);
    struct json_object* obj = json_tokener_parse(jseValue);
    if (!obj)
        return -1;

    struct json_object* lookup = json_object_object_get(obj, "tpID");
    if (!lookup)
        goto Err;
    getDvbManager().delTransponder(json_object_get_int(lookup));
    getDvbManager().saveDB(eTransponderDB);
    json_object_put(obj);
    return 0;
Err:
    json_object_put(obj);
    return -1;
}

/*
 *  未使用任何开关情况下,启动天线自动安装,盲扫所有卫星并保存 TP
 *         Utility.setValueByName (‘dvb_antenna_auto_setup_start’, ‘{“setupType”:0, “satSearchMode”:blindScanAll”,” isSearchTP”:1}’);
 *  使用 DiSEqC 1.0 开关情况下,启动天线自动安装,盲扫预置卫星不保存 TP
 *         Utility.setValueByName (‘dvb_antenna_auto_setup_start’, ‘{“setupType”:1, “satSearchMode”: ”blindScanPreset”, ” isSearchTP”:0}’);
 *  */
static int _JseWrite_dvb_antenna_setup_start(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseValue: %s\n", jseValue);
    struct json_object* obj = json_tokener_parse(jseValue);
    if (!obj)
        return -1;
    getDvbManager().delDvbScan(ANTENNA_SETUP_NAME);

    int tSetupType    = json_object_get_int(json_object_object_get(obj, "setupType"));
    bool tIsSearchTP  = json_object_get_boolean(json_object_object_get(obj, "isSearchTP"));
    std::string tMode = json_object_get_string(json_object_object_get(obj, "satSearchMode"));
    json_object_put(obj);

    int tSearchMode = 0;
    if (tMode == "blindScanAl")
        tSearchMode = 0;
    else if (tMode == "blindScanPreset")
        tSearchMode = 1;
    else if (tMode == "scanTestTP")
        tSearchMode = 2;
    DvbScan* scan = new DvbAntennaSetup(ANTENNA_SETUP_NAME, 1, tSetupType, tSearchMode, tIsSearchTP);
    if (!scan)
        goto Err;

    if (eDvbOk != getDvbManager().addDvbScan(scan)) {
        delete(scan);
        goto Err;
    }
    gDvbScanType = eDvbAntennaSetup;
    return scan->scanStart();
Err:
    gDvbScanType = eDvbScanIdle;
    LogDvbDebug("error");
    return -1;
}

/*
 *   获取天线自动安装的搜索进度
 *         Utility.getValueByName(‘dvb_antenna_auto_setup_get_progress’);
 *   当前完成 50%,搜索到 port2,搜索到 1 颗卫星,STB 返回:
 *         {“progress”:50,”diSEqCPort”:1,”satCount”:1}
 *  */
static int _JseRead_dvb_antenna_setup_get_progress(const char* jseFunc, const char* jseParam, char * jseValue, int jseLen)
{
    DvbAntennaSetup* scan = static_cast<DvbAntennaSetup*>(getDvbManager().getDvbScan(ANTENNA_SETUP_NAME));
    if (!scan)
        return -1;
    struct json_object* obj = json_object_new_object();
    if (!obj)
        return -1;
    json_object_object_add(obj, "progress",   json_object_new_int(scan->getProgress()));
    if (scan->getSetupType())
        json_object_object_add(obj, "diSEqCPort", json_object_new_int(scan->getCurrentPort()));
    json_object_object_add(obj, "satCount",   json_object_new_int(scan->getDeviceKeys().size()));
    strcpy(jseValue, json_object_to_json_string(obj));
    json_object_put(obj);
	LogDvbDebug("jseValue = %s\n", jseValue);
	return 0;
}

/*
 *  获取天线自动安装搜索到的卫星信息:
 *      Utility.getValueByName(‘dvb_antenna_auto_setup_get_info’);
 *  当前有搜索结果,且使用了 DiSEqC 1.0 时,STB 返回
 *      “{“count”:2, ”satList”:[{”satID”:60,”tunnerID”:0,“satName”:”sat1”,”satLongitude”:”80”,”diSEqCPort”:0},{”satID”:61,”tunnerID”:0,“satName”:”sat2”,”satLongitude”:”100”,”diSEqCPort”:1}]}”
 *  当前有搜索结果,且未使用任何开关时,STB 返回
 *      “{“count”:1, ”satList”:[{”satID”:60,”tunnerID”:0,“satName”:”sat1”,”satLongitude”:”80”}]}”
 *  当前无搜索结果,STB 返回
 *      “{“count”:0,”satList”:[]}”
 *  */
static int _JseRead_dvb_antenna_setup_get_info(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    DvbAntennaSetup* scan = static_cast<DvbAntennaSetup*>(getDvbManager().getDvbScan(ANTENNA_SETUP_NAME));
    if (!scan)
        return -1;
    struct json_object* jObject = json_object_new_object();
    struct json_object* jArray  = json_object_new_array();
    std::vector<int> deviceKeys = scan->getDeviceKeys();

    DvbDeviceDVBS* device = 0;
    int count = 0;
    struct json_object* jSatList = 0;
	for (std::size_t i = 0; i < deviceKeys.size(); i++) {
        device = static_cast<DvbDeviceDVBS*>(getDvbManager().getDvbDevice(deviceKeys[i]));
		if (device && (jSatList = json_object_new_object())) {
            json_object_object_add(jSatList, "satID",       json_object_new_int(device->mDeviceKey));
            json_object_object_add(jSatList, "tunerID",     json_object_new_int(device->mTunerId));
            json_object_object_add(jSatList, "satName",     json_object_new_string(device->mSateliteName.c_str()));
            json_object_object_add(jSatList, "satLongitude",json_object_new_string(device->mSatLongitude.c_str()));
            if (scan->getSetupType())
                json_object_object_add(jSatList, "diSEqCPort",  json_object_new_int(device->mDiseqcPort));
            json_object_array_add(jArray, jSatList);
            count++;
        }
	}
	json_object_object_add(jObject,"count", json_object_new_int(count));
	json_object_object_add(jObject,"satList", jArray);

	strcpy(jseValue, json_object_to_json_string(jObject));
	json_object_put(jObject);

    if (100 == scan->getProgress()) {
        getDvbManager().delDvbScan(ANTENNA_SETUP_NAME);
        gDvbScanType = eDvbScanIdle;
    }
	LogDvbDebug("jseValue = %s\n", jseValue);
	return 0;
}

/*
 * 盲扫 ID 为 1 的卫星下的 TP
 *      Utility.setValueByName(‘dvb_blind_search_tp_start’, ‘{“satID”:1}’);
 *  */
static int _JseWrite_dvb_blind_search_tp_start(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseValue : %s\n", jseValue);
    struct json_object* obj = json_tokener_parse(jseValue);
    if (!obj)
        return -1;
    getDvbManager().delDvbScan(BLIND_SCAN_NAME);

    int satId = json_object_get_int(json_object_object_get(obj, "satID"));
    json_object_put(obj);
    if (!getDvbManager().getDvbDevice(satId))
        return -1;
    std::vector<int> deviceKeys(1, satId);
    DvbBlindScan* scan = new DvbBlindScan(BLIND_SCAN_NAME, deviceKeys);
    if (!scan)
        return -1;
    gDvbScanType = eDvbBlindScan;
    getDvbManager().addDvbScan(scan);
    return scan->scanStart();
}

/*
 * 获取盲扫 TP 的搜索进度
 *     Utility.getValueByName(‘dvb_blind_search_tp_get_progress’);
 * 当前搜索完成 50%,搜索到 10 个 TP,STB 返回
 *     {“progress”:50,”count”:10}
 *  */
static int _JseRead_dvb_blind_tp_get_progress(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    DvbBlindScan* scan = static_cast<DvbBlindScan*>(getDvbManager().getDvbScan(BLIND_SCAN_NAME));
    if (!scan)
        return -1;
	struct json_object* obj = json_object_new_object();
    if (!obj)
        return -1;
    json_object_object_add(obj, "progress", json_object_new_int(scan->getProgress()));
    json_object_object_add(obj, "tpCount",  json_object_new_int(scan->getTransponderKeys().size()));
	strcpy(jseValue, json_object_to_json_string(obj));
	json_object_put(obj);
    if (100 == scan->getProgress()) {
        getDvbManager().delDvbScan(BLIND_SCAN_NAME);
        gDvbScanType = eDvbScanIdle;
    }
	LogDvbDebug("jseValue = %s\n", jseValue);
    return 0;
}

/*
 *  获取搜索到的 TP 个数
 *         Utility.getValueByName(‘dvb_new_tp_get_count’);
 *  当前搜索到的 TP 个数为 10,STB 返回”10”
 *  */
static int _JseRead_dvb_new_tp_get_count(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    DvbBlindScan* scan = static_cast<DvbBlindScan*>(getDvbManager().getDvbScan(BLIND_SCAN_NAME));
    if (!scan)
        return -1;
	return sprintf(jseValue, "%d", scan->getTransponderKeys().size());
}

/*
 *  从第 1 个开始获取 2 个搜索到的 TP 信息
 *     Utility.getValueByName(‘dvb_new_tp_get_info,{“position”: 0, “count”: 2}’);
 *  STB 返回
 *     {“tpNum”:2,”tplist”: [{“tpID”:1,”satID”:2,“frequency”:11200, “symbolRate”:27500,”polarization”:0},{“tpID”:2,”satID”:2,“frequency”:11500, “symbolRate”:28500,”polarization”:1}]}
 *  */
static int _JseRead_dvb_new_tp_get_info(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    DvbBlindScan* scan = static_cast<DvbBlindScan*>(getDvbManager().getDvbScan(BLIND_SCAN_NAME));
    if (!scan)
        return -1;
    struct json_object* obj = json_tokener_parse(jseParam);
    if (!obj)
        return -1;
    int position = json_object_get_int(json_object_object_get(obj, "position"));
    int count    = json_object_get_int(json_object_object_get(obj, "count"));
    json_object_put(obj);

	struct json_object* jObject = json_object_new_object();
	struct json_object* jArray  = json_object_new_array();

    std::vector<int> transponderKeys = scan->getTransponderKeys();
    TransponderDVBS* transponder = 0;
    struct json_object* jTpsList = 0;
    int tpNum = 0;
	for (std::size_t i = position; tpNum < count && i < transponderKeys.size(); i++){
        transponder = static_cast<TransponderDVBS*>(getDvbManager().getTransponder(transponderKeys[i]));
		if (transponder && (jTpsList = json_object_new_object())) {
            json_object_object_add(jTpsList, "tpID",         json_object_new_int(transponder->mUniqueKey));
            json_object_object_add(jTpsList, "frequency",    json_object_new_int(transponder->mFrequency / 1000));
            json_object_object_add(jTpsList, "symbolRate",   json_object_new_int(transponder->mSymbolRate));
            json_object_object_add(jTpsList, "polarization", json_object_new_int(transponder->mPolarization));
            json_object_array_add(jArray, jTpsList);
            tpNum++;
        }
	}
	json_object_object_add(jObject, "tpNum", json_object_new_int(tpNum));
	json_object_object_add(jObject, "tpList", jArray);

	strncpy(jseValue, json_object_to_json_string(jObject), JSE_BUFFER_SIZE);
	json_object_put(jObject);
	LogDvbDebug("jseValue = %s\n", jseValue);
	return 0;
}

static int _JseWrite_dvb_search_stop(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    DvbScan* scan = 0;
	switch (gDvbScanType) {
		case eDvbAntennaSetup:
            scan = static_cast<DvbAntennaSetup*>(getDvbManager().getDvbScan(ANTENNA_SETUP_NAME));
			break;
		case eDvbBlindScan:
            scan = static_cast<DvbBlindScan*>(getDvbManager().getDvbScan(BLIND_SCAN_NAME));
			break;
		case eDvbScanChannelBySat:
		case eDvbScanChannelByTps:
            scan = static_cast<DvbScanChannel*>(getDvbManager().getDvbScan(SCAN_CHANNEL_NAME));
			break;
		default:
            return -1;
	}
    if (!scan)
        return -1;

    LogDvbDebug("scan stop [%s]\n", scan->getScanName().c_str());
    getDvbManager().delDvbScan(scan->getScanName());
    gDvbScanType = eDvbScanIdle;
    return 0;
}

/*
 * 搜索 TPID 为 1 的 TP 下的频道,不搜索 NIT 表,只搜索 FTA 频道,搜索所有频道
 *     Utility.setValueByName(‘dvb_tp_search_start’, ‘{ “tpID”:1, “nitFlag”: 0,“ftaFlag”:1,“tvRadioSelect”:0}’)
 *  */
static int _JseWrite_dvb_tp_search_start(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseValue : %s\n", jseValue);
    struct json_object* obj = json_tokener_parse(jseValue);
    if (!obj)
        return -1;
    getDvbManager().delDvbScan(SCAN_CHANNEL_NAME);

    int tpId = json_object_get_int(json_object_object_get(obj, "tpID"));
    if (!getDvbManager().getTransponder(tpId)) {
        json_object_put(obj);
        return -1;
    }
    struct json_object* lookup =  0;
    std::vector<int> tKeys(1, tpId);
    int tNitFlag = 0, tFtaFlag = 0, tTvRadio = 0;
    if ((lookup = json_object_object_get(obj, "nitFlag")))       tNitFlag = json_object_get_int(lookup);
    if ((lookup = json_object_object_get(obj, "ftaFlag")))       tFtaFlag = json_object_get_int(lookup);
    if ((lookup = json_object_object_get(obj, "tvRadioSelect"))) tTvRadio = json_object_get_int(lookup);
    json_object_put(obj);

    DvbScanChannel* scan = new DvbScanChannel(SCAN_CHANNEL_NAME, tKeys, (bool)tNitFlag, (ChannelType_e)tFtaFlag, (ServiceType_e)tTvRadio);
    if (eDvbOk != getDvbManager().addDvbScan(scan)) {
        if (scan)
            delete(scan);
        return -1;
    }
    gDvbScanType = eDvbScanChannelByTps;
    return scan->scanStart();
}

/*
 *  获取搜索 TP 上的频道的搜索进度
 *      Utility.getValueByName(‘dvb_tp_search_get_progress’);
 *  当前搜索完成 50%,搜索到 10 个音视频频道,10 个音频频道,STB 返回
 *      {“progress”:50,” allCount”:20,”videoCount”:10,” audioCount “:10}
 *  */
static int _JseRead_dvb_tp_search_get_progress(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    DvbScanChannel* scan = static_cast<DvbScanChannel*>(getDvbManager().getDvbScan(SCAN_CHANNEL_NAME));
    if (!scan)
        return -1;
    struct json_object* obj = json_object_new_object();
    if (!obj)
        return -1;

    json_object_object_add(obj, "progress",   json_object_new_int(scan->getProgress()));
    json_object_object_add(obj, "allCount",   json_object_new_int(scan->getChnlsCount(eServiceAll)));
    json_object_object_add(obj, "videoCount", json_object_new_int(scan->getChnlsCount(eServiceTv)));
    json_object_object_add(obj, "audioCount", json_object_new_int(scan->getChnlsCount(eServiceRadio)));

    strcpy(jseValue, json_object_to_json_string(obj));
    json_object_put(obj);
    if (100 == scan->getProgress()) {
        DvbScanEpg* scan = new DvbScanEpg(SCAN_EPG_NAME, 1);
        if (eDvbOk == getDvbManager().addDvbScan(scan))
            scan->scanStart();
        else
            delete(scan);
    }
	LogDvbDebug("jseValue = %s\n", jseValue);
	return 0;
}
/*
 *  搜索 ID 为 1,2,3 的卫星上的频道
 *        Utility.setValueByName(‘dvb_sat_search_start’, ‘{“satNum”:3, “satIDList”:[1,2,3],“ftaFlag”:1, “tvRadioSelect”:0}’);
 *  ftaFlag: 0: ALL channels 1: FTA channels
 *  tvRadioSelect:  0: ALL 	1: TV 	2: BroadCast
 *  */
static int _JseWrite_dvb_sat_search_start(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseValue : %s\n", jseValue);
    struct json_object* obj = json_tokener_parse(jseValue);
    if (!obj)
        return -1;
    getDvbManager().delDvbScan(SCAN_CHANNEL_NAME);

    int tSatNum  = json_object_get_int(json_object_object_get(obj, "satNum"));
    int tFtaFlag = json_object_get_int(json_object_object_get(obj, "ftaFlag"));
    int tTvRadio = json_object_get_int(json_object_object_get(obj, "tvRadioSelect"));
    struct json_object* tArray = json_object_object_get(obj, "satIDList");
    int tArrayLen = json_object_array_length(tArray);
    if (tSatNum > tArrayLen)
        tSatNum = tArrayLen;

    std::vector<int> tTpsKeys;
    for (int i = 0; i < tSatNum; ++i) {
        std::vector<int> tKeys = getDvbManager().getTransponderKeys(json_object_get_int(json_object_array_get_idx(tArray, i)));
        for (std::size_t j = 0; j < tKeys.size(); ++j)
            tTpsKeys.push_back(tKeys[j]);
    }
	json_object_put(obj);

    DvbScanChannel* scan = 0;
    if (0 == tTpsKeys.size())
        goto Err;
    scan = new DvbScanChannel(SCAN_CHANNEL_NAME, tTpsKeys, false, (ChannelType_e)tFtaFlag, (ServiceType_e)tTvRadio);
    if (eDvbOk != getDvbManager().addDvbScan(scan)) {
        if (scan)
            delete(scan);
        goto Err;
    }
    return scan->scanStart();
Err:
    LogDvbDebug("error\n");
    return -1;
}

/*
 * 获取搜索卫星上的频道的搜索进度
 *     Utility.getValueByName(‘dvb_sat_search_get_progress’);
 * 当前搜索完成 20%,搜索到 ID 为 1 的卫星下的 ID 为 1 的 TP,当前共搜索到 10 个音视频频道,10 个音频频道,STB 返回
 *     {“progress”:20,”satID”:1,”tpID”:1,”allCount”:20,”videoCount”:10,” audioCount “:10}
 *  */
static int _JseRead_dvb_sat_search_get_progress(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    DvbScanChannel* scan = static_cast<DvbScanChannel*>(getDvbManager().getDvbScan(SCAN_CHANNEL_NAME));
    if (!scan)
        return -1;
    struct json_object* obj = json_object_new_object();
    if (!obj)
        return -1;

    int progress = scan->getProgress();
    Transponder* transponder = getDvbManager().getTransponder(scan->getCurrentTpKey());
    if (transponder) {
        json_object_object_add(obj, "progress",   json_object_new_int(progress));
        json_object_object_add(obj, "satID",      json_object_new_int(transponder->getDvbDeviceKey()));
        json_object_object_add(obj, "tpID",       json_object_new_int(transponder->mUniqueKey));
        json_object_object_add(obj, "allCount",   json_object_new_int(scan->getChnlsCount(eServiceAll)));
        json_object_object_add(obj, "videoCount", json_object_new_int(scan->getChnlsCount(eServiceTv)));
        json_object_object_add(obj, "audioCount", json_object_new_int(scan->getChnlsCount(eServiceRadio)));
    }

    strcpy(jseValue, json_object_to_json_string(obj));
    json_object_put(obj);
    if (100 == progress) {
        getDvbManager().delDvbScan(SCAN_CHANNEL_NAME);
        gDvbScanType = eDvbScanIdle;
    }
	LogDvbDebug("jseValue = %s\n", jseValue);
	return 0;
}

/*
 *  var sValue = Utility.getValueByName(‘dvb_new_chnls_get_count,{“type”:type}’);
 *  type : 0: ALL 1: TV 2: Broadcast
 *  */
static int _JseRead_dvb_new_chnls_get_count(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseParam: %s\n", jseParam);
    DvbScanChannel* scan = static_cast<DvbScanChannel*>(getDvbManager().getDvbScan(SCAN_CHANNEL_NAME));
    if (!scan)
        return -1;
    struct json_object* obj = json_tokener_parse(jseParam);
    if (!obj)
        return -1;
    int tServiceType = (json_object_get_int(json_object_object_get(obj, "type")));
    int tChnlsCount  = 0;
    switch (tServiceType) {
        case 0: tChnlsCount = scan->getChnlsCount(eServiceAll);   break;
        case 1: tChnlsCount = scan->getChnlsCount(eServiceTv);    break;
        case 2: tChnlsCount = scan->getChnlsCount(eServiceRadio); break;
    }
    json_object_put(obj);
    return sprintf(jseValue, "%d", tChnlsCount);
}
/*
 * 从第 1 个开始获取 2 个搜索到的电视频道
 * var sJson = Utility.getValueByName(‘dvb_new_chnls_get_info, {“type”:1,”position”:0,”count”:2}’)
 * STB 返回
 * ‘{“channelNum”:2,”channelList”:[{“channelID”:1,“chanKey”:1,”tpID”:1,”satID”:1,“channelType”:1,”channelName”:”CCTV-1”,”isHD”:0,”scrambled”:0},{“channelID”:2, “chanKey”:2,”tpID”:1,”satID”:1,“channelType”:1,”channelName”:”CCTV-2”,”isHD”:0,”scrambled”:0}]}’
 *  */
static int _JseRead_dvb_new_chnls_get_info(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseParam: %s\n", jseParam);
    DvbScanChannel* scan = static_cast<DvbScanChannel*>(getDvbManager().getDvbScan(SCAN_CHANNEL_NAME));
    if (!scan)
        return -1;
    struct json_object* obj = json_tokener_parse(jseParam);
    if (!obj)
        return -1;
    int type     = json_object_get_int(json_object_object_get(obj, "type"));
    int position = json_object_get_int(json_object_object_get(obj, "position"));
    int count    = json_object_get_int(json_object_object_get(obj, "count"));
    json_object_put(obj);

	struct json_object* jObject = json_object_new_object();
	struct json_object* jArray  = json_object_new_array();

    ServiceType_e serType = eServiceAll;
    switch (type) {
        case 0 :
            serType = eServiceAll;
            break;
        case 1 :
            serType = eServiceTv;
            break;
        case 2 :
            serType = eServiceRadio;
            break;
        default:
            ;
    }
    std::vector<int> tChannelKeys = scan->getDvbChannelKeys();
    DvbChannel* channel = 0;
    int channelNum = 0;
    struct json_object* jChnlList = 0;
	for (std::size_t i = position; channelNum < count && i < tChannelKeys.size(); i++){
        channel = getDvbManager().getDvbChannel(tChannelKeys[i]);
        if (!channel || serType != channel->mServiceType)
            continue;
		if ((jChnlList = json_object_new_object())) {
            json_object_object_add(jChnlList, "channelID",  json_object_new_int(channel->mChannelKey));
            json_object_object_add(jChnlList, "chanKey",    json_object_new_int(channel->mChannelKey));
            json_object_object_add(jChnlList, "tpID",       json_object_new_int(channel->getTransponderKey()));
            json_object_object_add(jChnlList, "satID",      json_object_new_int(channel->mTransponder->getDvbDeviceKey()));
            json_object_object_add(jChnlList, "channelType",json_object_new_int(channel->mServiceType));
            json_object_object_add(jChnlList, "channelName",json_object_new_string(channel->mChannelName.c_str()));
            json_object_object_add(jChnlList, "isHD",       json_object_new_int(0));
            json_object_object_add(jChnlList, "scrambled",  json_object_new_int(channel->mChannelType));
            json_object_array_add(jArray, jChnlList);
            channelNum++;
        }
    }
	json_object_object_add(jObject,"channelNum", json_object_new_int(channelNum));
	json_object_object_add(jObject,"channelList", jArray);

	strncpy(jseValue, json_object_to_json_string(jObject), JSE_BUFFER_SIZE);
	json_object_put(jObject);
	LogDvbDebug("jseValue = %s\n", jseValue);
	return 0;
}

/*
 *  var sValue = Utility.getValueByName(‘dvb_get_chnls_count,{“type”:0}’)
 *      频道列表类型
 *          0 表示所有频道
 *          1 表示电视频道
 *          2 表示广播频道
 *  */
static int _JseRead_dvb_get_chnls_count(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    struct json_object* obj = json_tokener_parse(jseParam);
    if (!obj)
        return -1;
    int tServiceType = json_object_get_int(json_object_object_get(obj, "type"));
    int tChnlsCount  = 0;
    switch (tServiceType) {
        case 0: tChnlsCount = getDvbManager().getDvbChannelsCount(eServiceAll);   break;
        case 1: tChnlsCount = getDvbManager().getDvbChannelsCount(eServiceTv);    break;
        case 2: tChnlsCount = getDvbManager().getDvbChannelsCount(eServiceRadio); break;
    }
    json_object_put(obj);
    return sprintf(jseValue, "%d", tChnlsCount);
}

/*
 *  从第一个频道开始获取 2 个频道信息:
 *     Var sJson =Utility.setValueByName(‘dvb_get_channel_list’,’{”type”:0,”position”:0,”count”:2 }’);
 *  STB 返回:
 *     {"channelNum":1,"channelList":[{”chanKey”:1,"tpID":1,"channelType":1,"channelName":"CCTV-1,"locked":0,"skip":0,"isHD":0,"Scrambled":0,”chanBandwidth“:3000,”HDCPEnable”:0,”CGMSAEnable”:0,”MacrovisionEnable”:0}]}
 *  */
static int _JseRead_dvb_get_channel_list(const char* jseFunc, const char* jseParam, char* jseValue, int jseLlen)
{
    struct json_object* obj = json_tokener_parse(jseParam);
    if (!obj)
        return -1;
    int type     = json_object_get_int(json_object_object_get(obj, "type"));
    int position = json_object_get_int(json_object_object_get(obj, "position"));
    int count    = json_object_get_int(json_object_object_get(obj, "count"));
    json_object_put(obj);

	struct json_object* jObject = json_object_new_object();
	struct json_object* jArray  = json_object_new_array();

    ServiceType_e tServiceType = eServiceAll;
    switch (type) {
        case 0: tServiceType = eServiceAll;   break;
        case 1: tServiceType = eServiceTv;    break;
        case 2: tServiceType = eServiceRadio; break;
    }
    std::vector<int> tChannelKeys = getDvbManager().getDvbChannelKeys(tServiceType);
    DvbChannel* channel = 0;
    int channelNum = 0;
	struct json_object* jChnlList = 0;
	for (std::size_t i = position; channelNum < count && i < tChannelKeys.size(); i++){
        channel = getDvbManager().getDvbChannel(tChannelKeys[i]);
		if (channel && (jChnlList = json_object_new_object())) {
            json_object_object_add(jChnlList, "chanKey",          json_object_new_int(channel->mChannelKey));
            json_object_object_add(jChnlList, "tpID",             json_object_new_int(channel->getTransponderKey()));
            json_object_object_add(jChnlList, "satID",            json_object_new_int(channel->mTransponder->getDvbDeviceKey()));
            json_object_object_add(jChnlList, "channelType",      json_object_new_int(channel->mServiceType));
            json_object_object_add(jChnlList, "channelName",      json_object_new_string(channel->mChannelName.c_str()));
            json_object_object_add(jChnlList, "isHD",             json_object_new_int(0));
            json_object_object_add(jChnlList, "scrambled",        json_object_new_int(channel->mChannelType));
            json_object_object_add(jChnlList, "chanBandwidth",    json_object_new_int(0));
            json_object_object_add(jChnlList, "HDCPEnable",       json_object_new_int(0));
            json_object_object_add(jChnlList, "CGMSAEnable",      json_object_new_int(0));
            json_object_object_add(jChnlList, "MacrovisionEnable",json_object_new_int(0));
            json_object_array_add(jArray, jChnlList);
            channelNum++;
        }
	}
	json_object_object_add(jObject,"channelNum", json_object_new_int(count));
	json_object_object_add(jObject,"channelList", jArray);

	strncpy(jseValue, json_object_to_json_string(jObject), JSE_BUFFER_SIZE);
	json_object_put(jObject);
	LogDvbDebug("jseValue = %s\n", jseValue);
	return 0;
}

/*
 *   获取 ID 为 2 的频道的参数信息
 *       Utility.getValueByName("dvb_get_channel_paras_by_chanKey,{"chanKey":2}");
 *   STB 返回:
 *      {”chanKey”:1,"tpID":1,"channelType":1,"channelName":"CCTV-1,"locked:0","skip":0,"isHD":0,"Scrambled": 0,”chanBandwidth“:3000,”HDCPEnable”:0,”CGMSAEnable”:0,”MacrovisionEnable”:0}
 *  */
static int _JseRead_dvb_get_channel_paras_by_chankey(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    struct json_object* obj = json_tokener_parse(jseParam);
    if (!obj)
        return -1;
    int tChannelKey = json_object_get_int(json_object_object_get(obj, "chanKey"));
    json_object_put(obj);

    DvbChannel* channel = getDvbManager().getDvbChannel(tChannelKey);
	if(!channel)
        return -1;

    struct json_object* jObject = json_object_new_object();
    if (!jObject)
        return -1;

    json_object_object_add(jObject, "chanKey",          json_object_new_int(channel->mChannelKey));
    json_object_object_add(jObject, "tpID",             json_object_new_int(channel->getTransponderKey()));
    json_object_object_add(jObject, "satID",            json_object_new_int(channel->mTransponder->getDvbDeviceKey()));
    json_object_object_add(jObject, "channelType",      json_object_new_int(channel->mServiceType));
    json_object_object_add(jObject, "channelName",      json_object_new_string(channel->mChannelName.c_str()));
    json_object_object_add(jObject, "isHD",             json_object_new_int(0));
    json_object_object_add(jObject, "scrambled",        json_object_new_int(channel->mChannelType));
    json_object_object_add(jObject, "chanBandwidth",    json_object_new_int(0));
    json_object_object_add(jObject, "HDCPEnable",       json_object_new_int(0));
    json_object_object_add(jObject, "CGMSAEnable",      json_object_new_int(0));
    json_object_object_add(jObject, "MacrovisionEnable",json_object_new_int(0));

    strncpy(jseValue, json_object_to_json_string(jObject), JSE_BUFFER_SIZE);
    json_object_put(jObject);
    LogDvbDebug("jseValue = %s\n", jseValue);
    return 0;
}

/*
 *  获取系统频道号为 2 的频道的在视频频道列表中的位置
 *        Utility.getValueByName(‘dvb_get_channel_position_by_channelKey,{“type”:1,"chanKey":2}’);
 *  该频道在视频频道列表中是第 3 个频道,STB 返回:
 *        “2”
 *  */
static int _JseRead_dvb_get_channel_position_by_chankey(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    struct json_object* obj = json_tokener_parse(jseParam);
    if (!obj)
        return -1;
    int tType     = json_object_get_int(json_object_object_get(obj, "type"));
    int tChnlKey  = json_object_get_int(json_object_object_get(obj, "chanKey"));
    json_object_put(obj);

    ServiceType_e tServiceType = eServiceAll;
    switch (tType) {
        case 0: tServiceType = eServiceAll;
        case 1: tServiceType = eServiceTv;
        case 2: tServiceType = eServiceRadio;
    }
    std::vector<int> tChannelKeys = getDvbManager().getDvbChannelKeys(tServiceType);
    int position = -1;
    for (std::size_t i = 0; i < tChannelKeys.size(); ++i) {
        if (tChnlKey == tChannelKeys[i])
            position = i;
    }
    return sprintf(jseValue, "%d", position);
}
/*
 *  var sValue = Utility.getValueByName(‘dvb_programItem_get_count,{ “chanKey”:chanKey, “day”: day }’);
 *  无论 STB 是否支持 UTC 时间,该接口都以本地时间来判断在第几天
 *  */
static int _JseRead_dvb_programItem_get_count(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    if (!getDvbManager().isDateTimeSync()) {
        strcpy(jseValue, "0");
        return -1;
    }

    LogDvbDebug("jseParam: %s\n", jseParam);
    struct json_object* obj = json_tokener_parse(jseParam);
    if (!obj)
        return -1;

    int tChnlKey = json_object_get_int(json_object_object_get(obj, "chanKey"));
    int tDay     = json_object_get_int(json_object_object_get(obj, "day"));
    json_object_put(obj);

    DvbEpg* epg = 0;
    DvbChannel* channel = getDvbManager().getDvbChannel(tChnlKey);
    if (!channel || !(epg = channel->getEpg()))
        return -1;

    time_t tTimeUTC = 0, tSTimeUTC = 0, tETimeUTC = 0;
	struct tm tLocalTime;
    int itemCount = 0;
    //time(&tTimeUTC);
    tTimeUTC = time(0) - getDvbManager().getDateTimeOffset();
    localtime_r(&tTimeUTC, &tLocalTime);
	tLocalTime.tm_hour = tLocalTime.tm_min = tLocalTime.tm_sec = 0;
	tTimeUTC  = mktime(&tLocalTime);
	tSTimeUTC = tTimeUTC + 86400 * tDay; //86400 = 24 * 3600
	tETimeUTC = tTimeUTC + 86400 * (tDay + 1);

    epg->lock();
    itemCount = epg->getCount(tSTimeUTC, tETimeUTC);
    epg->unlock();
    LogDvbDebug("itemCount : %d\n", itemCount);
    return sprintf(jseValue, "%d", itemCount);;
}

/*
 *  获取频道 3 当天的节目单,从第一个节目单开始,获取两个:
 *         var sValue = Utility.getValueByName(‘dvb_programItem_get, {”chanKey”:3,”day”:0,”postion”:0,”count”:2}’)
 *  如果 STB 支持 UTC 时间,STB 返回: (STB是支持的）
 *     {“programCount”:2,” programList”:[
 *         {“chanKey”:105,“programName”:”新闻联播”,“programID”:“0x0001”,”serviceID”:”0x0030”,
 *             “startTime”:”2011-10-19 19:00:00 UTC+08:00”,“endTime”:”2011-10-19 19:30:00 UTC+08:00 DST”,“programRate”:0,
 *             “programDescription”:[{“languageCode”:”chn”,“programInformation”:”国家大事播报”}]},
 *         {“chanKey”:105,“programName”:”地道战”,“programID”:”0x0002”,”serviced”:”0x0030”,
 *             “startTime”:”2011-10-19 19:40:00 UTC+08:00”,“endTime”:”2011-10-19 20:40:00 UTC+08:00 ”,“programRate”:18,“programDescription”[]
 *         }]
 *     }
 *   如果 STB 不支持 UTC 时间,STB 返回:
 *     {“programCount”:2,” programList”:[
 *         {“chanKey”:105,“programName”:”新闻联播”,“programID”:“0x0001”,”serviceID”:”0x0030”,
 *             “startTime”:”20111019190000”,“endTime”:”20111019193000”,“programRate”:0,
 *             “programDescription”:[{“languageCode”:”chn”,“programInformation”:”国家大事播报”}]},
 *         {“chanKey”:105,“programName”:”地道战”,“programID”:”0x0002”,”serviced”:”0x0030”,
 *             “startTime”:”20111019194000”,“endTime”:”20111019204000”,“programRate”:18,“programDescription”[]
 *         }]
 *     }
 *   */
static int _JseRead_dvb_programItem_get(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    if (!getDvbManager().isDateTimeSync()) {
        strcpy(jseValue, "0");
        return -1;
    }

    LogDvbDebug("jseParam: %s\n", jseParam);
    struct json_object* obj = json_tokener_parse(jseParam);
    if (!obj)
        return -1;

    int tChnlKey = json_object_get_int(json_object_object_get(obj, "chanKey"));
    int tDay     = json_object_get_int(json_object_object_get(obj, "day"));
    int position = json_object_get_int(json_object_object_get(obj, "position"));
    int count    = json_object_get_int(json_object_object_get(obj, "count"));
    json_object_put(obj);

    time_t tTimeUTC = 0, tSTimeUTC = 0, tETimeUTC = 0;
	struct tm tLocalTime, tSTime, tETime;

    //time(&tTimeUTC);
    tTimeUTC = time(0) - getDvbManager().getDateTimeOffset();
    localtime_r(&tTimeUTC, &tLocalTime);
	tLocalTime.tm_hour = tLocalTime.tm_min = tLocalTime.tm_sec = 0;
	tTimeUTC  = mktime(&tLocalTime);
	tSTimeUTC = tTimeUTC + 86400 * tDay; //86400 = 24 * 3600
	tETimeUTC = tTimeUTC + 86400 * (tDay + 1);

    std::string tLan = getDvbManager().getCountryCode();
    int tTimezone = 0, tSaveFlag = 0, tSLight = 0, tELight = 0, tOffsetSec = 0;
    bool bTimeDST = false;
    char sTimezone[16] = { 0 };
	sysSettingGetInt("timezone", &tTimezone, 0);
    appSettingGetInt("saveflag", &tSaveFlag, 0);
    appSettingGetInt("lightstart", &tSLight, 0);
    appSettingGetInt("lightstop",  &tELight, 0);
    mid_tool_timezone2str(tTimezone, sTimezone);
    tOffsetSec = get_local_time_zone();
    if(tSaveFlag && (time(0) > tSLight) && (time(0) < tELight)){
        bTimeDST = true;
        tOffsetSec += get_saving_time_sec();
    }

    DvbEpg* epg = 0;
    DvbChannel* channel = getDvbManager().getDvbChannel(tChnlKey);
    if (!channel || !(epg = channel->getEpg()))
        return -1;
    //epg->show();

	struct json_object* jObject = json_object_new_object();
	struct json_object* jArray  = json_object_new_array();

	struct json_object* jEventList = 0;
	struct json_object* jSubArray = 0;
	struct json_object* jSubEventList = 0;

    char tBufferST[64] = { 0 };
    char tBufferET[64] = { 0 };
    int programCount = 0;
    epg->lock();
    std::vector<EpgEvent*> tEvents = epg->getEvents(tSTimeUTC, tETimeUTC);
    for (std::size_t i = position; programCount < count && i < tEvents.size(); ++i) {
        if ((jEventList = json_object_new_object())) {
            tSTimeUTC = (time_t)tEvents[i]->mSTime + (time_t)tOffsetSec;
            tETimeUTC = (time_t)tEvents[i]->mETime + (time_t)tOffsetSec;
            gmtime_r(&tSTimeUTC, &tSTime);
            gmtime_r(&tETimeUTC, &tETime);
            sprintf(tBufferST, "%04d-%02d-%02d %02d:%02d:%02d %s%s", 1900+tSTime.tm_year, 1+tSTime.tm_mon, tSTime.tm_mday, tSTime.tm_hour, tSTime.tm_min, tSTime.tm_sec, sTimezone, bTimeDST ? " DST" : "");
            sprintf(tBufferET, "%04d-%02d-%02d %02d:%02d:%02d %s%s", 1900+tETime.tm_year, 1+tETime.tm_mon, tETime.tm_mday, tETime.tm_hour, tETime.tm_min, tETime.tm_sec, sTimezone, bTimeDST ? " DST" : "");
            json_object_object_add(jEventList, "chanKey",       json_object_new_int(tChnlKey));
            json_object_object_add(jEventList, "programName",   json_object_new_string(tEvents[i]->getEventName(tLan).c_str()));
            json_object_object_add(jEventList, "programID",     json_object_new_string(UtilsInt2Str(tEvents[i]->mEventId).c_str()));
            json_object_object_add(jEventList, "serviceID",     json_object_new_string(UtilsInt2Str(channel->mServiceId).c_str()));
            json_object_object_add(jEventList, "startTime",     json_object_new_string(tBufferST));
            json_object_object_add(jEventList, "endTime",       json_object_new_string(tBufferET));
            json_object_object_add(jEventList, "programRate",   json_object_new_int(tEvents[i]->getRating(tLan)));
            if ((jSubArray = json_object_new_array())) {
                std::map<std::string, EventText>& tTexts = tEvents[i]->getEventText();
                std::map<std::string, EventText>::iterator it;
                for (it = tTexts.begin(); it != tTexts.end(); ++it) {
                    if ((jSubEventList = json_object_new_object())) {
                        json_object_object_add(jSubEventList, "languageString",     json_object_new_string(it->second.mLangCode.c_str()));
                        json_object_object_add(jSubEventList, "programInformation", json_object_new_string(it->second.mShortText.c_str()));
                        json_object_array_add(jSubArray, jSubEventList);
                    }
                }
                json_object_object_add(jEventList, "programDescription", jSubArray);
            }
            json_object_array_add(jArray, jEventList);
            programCount++;
        }
    }
    epg->unlock();
	json_object_object_add(jObject,"programCount", json_object_new_int(programCount));
	json_object_object_add(jObject,"programList", jArray);
	strncpy(jseValue, json_object_to_json_string(jObject), JSE_BUFFER_SIZE);
	json_object_put(jObject);
	LogDvbDebug("jseValue = %s\n", jseValue);
	return 0;
}

/*
 * 当前节目单为新闻联播,下一个节目单为地道战,STB 支持 UTC 时间时,返回:
 * {“itemCount”:2,”items”:[
 *     {“programName”:”新闻联播”,“programID”:”0x0001”,”serviced”:”0x0030”, “startTime”:”2011-10-19 19:00:00 UTC+08:00 DST”,“endTime”:”2011-10-19 19:30:00 UTC+08:00 DST”,”programType”:0},
 *     {“programName”:”地道战”,“programID”:”0x0002”, ”serviced”:”0x0030”, “startTime”:”2011-10-19 19:40:00 UTC+08:00”,“endTime”:”2011-10-19 20:40:00 UTC+08:00”,”programType”:1}
 * ]}
 *  programType:
 *      0:当前节目单
 *      1:后续节目单
 *  */
static int _JseRead_dvb_get_pf_program_item(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    if (!getDvbManager().isDateTimeSync()) {
        strcpy(jseValue, "0");
        return -1;
    }

    time_t tTimeUTC = 0, tSTimeUTC = 0, tETimeUTC = 0;
	struct tm tSTime, tETime;
    //time(&tTimeUTC);
    tTimeUTC = time(0) - getDvbManager().getDateTimeOffset();
	tSTimeUTC = tTimeUTC - 2;
	tETimeUTC = tTimeUTC + 86400;

    std::string tLan = getDvbManager().getCountryCode();
    int tTimezone = 0, tSaveFlag = 0, tSLight = 0, tELight = 0, tOffsetSec = 0;
    bool bTimeDST = false;
    char sTimezone[16] = { 0 };
	sysSettingGetInt("timezone", &tTimezone, 0);
    appSettingGetInt("saveflag", &tSaveFlag, 0);
    appSettingGetInt("lightstart", &tSLight, 0);
    appSettingGetInt("lightstop",  &tELight, 0);
    mid_tool_timezone2str(tTimezone, sTimezone);
    tOffsetSec = get_local_time_zone();
    if(tSaveFlag && (time(0) > tSLight) && (time(0) < tELight)){
        bTimeDST = true;
        tOffsetSec += get_saving_time_sec();
    }

    DvbEpg* epg = 0;
    DvbChannel* channel = getDvbManager().getCurrentChannel();
    if (!channel || !(epg = channel->getEpg()))
        return -1;
    //epg->show();

	struct json_object* jObject = json_object_new_object();
	struct json_object* jArray  = json_object_new_array();

	struct json_object* jEventList = 0;

    char tBufferST[64] = { 0 };
    char tBufferET[64] = { 0 };
    epg->lock();
    std::vector<EpgEvent*> tEvents = epg->getEvents(tSTimeUTC, tETimeUTC);
    for (std::size_t i = 0; i < 2 && i < tEvents.size(); ++i) {
        if ((jEventList = json_object_new_object())) {
            tSTimeUTC = (time_t)tEvents[i]->mSTime + (time_t)tOffsetSec;
            tETimeUTC = (time_t)tEvents[i]->mETime + (time_t)tOffsetSec;
            gmtime_r(&tSTimeUTC, &tSTime);
            gmtime_r(&tETimeUTC, &tETime);
            sprintf(tBufferST, "%04d-%02d-%02d %02d:%02d:%02d %s%s", 1900+tSTime.tm_year, 1+tSTime.tm_mon, tSTime.tm_mday, tSTime.tm_hour, tSTime.tm_min, tSTime.tm_sec, sTimezone, bTimeDST ? " DST" : "");
            sprintf(tBufferET, "%04d-%02d-%02d %02d:%02d:%02d %s%s", 1900+tETime.tm_year, 1+tETime.tm_mon, tETime.tm_mday, tETime.tm_hour, tETime.tm_min, tETime.tm_sec, sTimezone, bTimeDST ? " DST" : "");
            json_object_object_add(jEventList, "programName",   json_object_new_string(tEvents[i]->getEventName(tLan).c_str()));
            json_object_object_add(jEventList, "programID",     json_object_new_string(UtilsInt2Str(tEvents[i]->mEventId).c_str()));
            json_object_object_add(jEventList, "serviceID",     json_object_new_string(UtilsInt2Str(channel->mServiceId).c_str()));
            json_object_object_add(jEventList, "startTime",     json_object_new_string(tBufferST));
            json_object_object_add(jEventList, "endTime",       json_object_new_string(tBufferET));
            json_object_object_add(jEventList, "programType",   json_object_new_int(i));
            json_object_array_add(jArray, jEventList);
        }
    }
    epg->unlock();
	json_object_object_add(jObject,"programCount", json_object_new_int(json_object_array_length(jArray)));
	json_object_object_add(jObject,"programList", jArray);
	strncpy(jseValue, json_object_to_json_string(jObject), JSE_BUFFER_SIZE);
	json_object_put(jObject);
	LogDvbDebug("jseValue = %s\n", jseValue);
	return 0;
}

/*
 *  获取 tunner1 上的信号质量
 *     Utility.getValueByName(‘dvb_get_signal_quality,{“tunerID”:0}’);
 *  tunner1 上当前无信号,STB 返回”0”
 *  tunner1 上当前信号质量为 50%,STB 返回”50”
 *
 *  tunerID:
 *      0 表示 tuner1
 *      1 表示 tuner2
 *     -1 表示当前播放节目所使用的 tuner,由STB 端去获取。
 *
 *  */
static int _JseRead_dvb_get_signal_quality(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseParam: %s\n", jseParam);
    if (eDvbScanIdle != gDvbScanType)
        return 0;
    struct json_object* obj = json_tokener_parse(jseParam);
    if (!obj)
        return -1;

    int tTunerId = json_object_get_int(json_object_object_get(obj, "tunerID"));
    json_object_put(obj);
    if (tTunerId < 0 || tTunerId > 2)
        tTunerId = getDvbManager().getCurrentTunerId();
    return sprintf(jseValue, "%d", getDvbManager().getFrontend()->getSignalQuality(tTunerId));
}

static int _JseRead_dvb_get_signal_strength(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseParam: %s\n", jseParam);
    if (eDvbScanIdle != gDvbScanType)
        return 0;
    struct json_object* obj = json_tokener_parse(jseParam);
    if (!obj)
        return -1;

    int tTunerId = json_object_get_int(json_object_object_get(obj, "tunerID"));
    json_object_put(obj);
    if (tTunerId < 0 || tTunerId > 2)
        tTunerId = getDvbManager().getCurrentTunerId();
    return sprintf(jseValue, "%d", getDvbManager().getFrontend()->getSignalStrength(tTunerId));
}

static int _JseRead_dvb_get_tuner_lock_status(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseParam: %s\n", jseParam);
    struct json_object* obj = json_tokener_parse(jseParam);
    if (!obj)
        return -1;

    int tTunerId = json_object_get_int(json_object_object_get(obj, "tunerID"));
    json_object_put(obj);
    if (tTunerId < 0 || tTunerId > 2)
        tTunerId = getDvbManager().getCurrentTunerId();
    return sprintf(jseValue, "%d", getDvbManager().getFrontend()->isTuned(tTunerId) ? 1 : 0);
}

/*
 *  在 tunner1 上锁定 ID 为 2 的 TP
 *     Utility.setValueByName (‘dvb_tune’, ‘{“tunerID”:0, “tpID”:2}’);
 *  */
static int _JseWrite_dvb_tune(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseValue: %s\n", jseValue);
    struct json_object* obj = json_tokener_parse(jseValue);
    if (!obj)
        return -1;
    Transponder* transponder = 0;
    int tTunerId = json_object_get_int(json_object_object_get(obj, "tunerID"));
    int transKey = json_object_get_int(json_object_object_get(obj, "tpID"));
    json_object_put(obj);
    if (tTunerId < 0 || tTunerId > 2)
        goto Err;
    transponder = getDvbManager().getTransponder(transKey);
    if (!transponder)
        goto Err;
    getDvbManager().getFrontend()->tune(tTunerId, transponder);
    return 0;
Err:
    return -1;
}

/*
 *  删除频道系统频道号为 1 的频道
 *         Utility.setValueByName (‘dvb_delete_channel’, ‘{“chanKeyList”:[1]}’);
 *  删除频道系统频道号为 1 和 2 的两个频道
 *         Utility.setValueByName (‘dvb_delete_channel’, ‘{“chanKeyList”:[1,2]}’);
 *  */
static int _JseWrite_dvb_delete_channel(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseValue: %s\n", jseValue);
    struct json_object* obj = json_tokener_parse(jseValue);
    if (!obj)
        return -1;
    struct json_object* tArray = json_object_object_get(obj, "chanKeyList");
    int tArrayLen = json_object_array_length(tArray);
    for (int i = 0; i < tArrayLen; ++i) {
        int chnlkey = json_object_get_int(json_object_array_get_idx(tArray, i));
        getDvbManager().delDvbChannel(chnlkey);
    }
    json_object_put(obj);
    getDvbManager().saveDB(eDvbChannelDB);
    return 0;
}

/*
 *  将系统频道号为 1 的频道名称修改为 CCTV100
 *     Utility.setValueByName(‘dvb_set_channel_name’,’{”chanKey”:1,”name”:”CCTV100”}’);
 *      0:修改成功
 *     -1:修改失败(其他原因)
 *     -2: 修改失败,有重名频道
 *  */
static int _JseWrite_dvb_set_channel_name(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseValue: %s\n", jseValue);
    struct json_object* obj = json_tokener_parse(jseValue);
    if (!obj)
        return -1;
    int tChanKey = json_object_get_int(json_object_object_get(obj, "chanKey"));
    std::string tChannelName = json_object_get_string(json_object_object_get(obj, "name"));
    json_object_put(obj);

    DvbChannel* channel = 0;
    std::vector<int> tChanKeys = getDvbManager().getDvbChannelKeys();
    for (std::size_t i = 0; i < tChanKeys.size(); ++i) {
        channel = getDvbManager().getDvbChannel(tChanKeys[i]);
        if (!channel || channel->mChannelKey == tChanKey)
            continue;
        if (channel->mChannelName == tChannelName)
            return -2;
    }
    channel = getDvbManager().getDvbChannel(tChanKey);
    if (channel) {
        channel->mChannelName = tChannelName;
        getDvbManager().saveDB(eDvbChannelDB);
    }
    return 0;
}

/*
 *  查询和“CCTV”模糊匹配的频道
 *     var sJson = Utility.getValueByName(‘dvb_chnl_search_by_name,{“name”:”CCTV”,”searchType”:1}’);
 *  ID 为 1,2,3,4,5 的频道名称包含 CCTV,STB 返回
 *     {“channelCount”:5,” chanKeyList”:[1,2,3,4,5]} STB 不对自定义频道号的重复性进行判断
 *
 *  searchType: 0 - accurate search 1 - fuzzy search
 *  */
static int _JseRead_dvb_chnl_search_by_name(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseValue: %s\n", jseValue);
    struct json_object* obj = json_tokener_parse(jseValue);
    if (!obj)
        return -1;

    std::string tName = json_object_get_string(json_object_object_get(obj, "name"));
    int  tSearchType  = json_object_get_int(json_object_object_get(obj, "searchType"));
    json_object_put(obj);

	struct json_object* jObject = json_object_new_object();
	struct json_object* jArray  = json_object_new_array();

    DvbChannel* channel = 0;
    std::vector<int> tChanKeys = getDvbManager().getDvbChannelKeys();
    for (std::size_t i = 0; i < tChanKeys.size(); ++i) {
        channel = getDvbManager().getDvbChannel(tChanKeys[i]);
        if (!channel)
            continue;
        switch (tSearchType) {
            case 0:  //accurate search
                if (!strcmp(tName.c_str(), channel->mChannelName.c_str()))
                    json_object_array_add(jArray, json_object_new_int(tChanKeys[i]));
                break;
            case 1: //fuzzy search
                if (strcasestr(tName.c_str(), channel->mChannelName.c_str()))
                    json_object_array_add(jArray, json_object_new_int(tChanKeys[i]));
                break;
            default:
                ;
        }
    }
	json_object_object_add(jObject, "channelCount", json_object_new_int(json_object_array_length(jArray)));
	json_object_object_add(jObject, "chanKeyList",  jArray);

	strcpy(jseValue, json_object_to_json_string(jObject));
	LogDvbDebug("jseValue = %s\n", jseValue);
	json_object_put(jObject);
	return 0;
}

/*
 *  获取 ID 为 1 的频道正在播放节目的 ID 为 23 的节目的详细介绍信息:
 *     Utility.getValueByName(‘dvb_get_channel_more_description,{”chanKey”:1,”programID”:”0x0023”}’)
 *  当前正在播放地道战,返回地道战的节目详细介绍,该介绍信息来自于 EIT 表的扩展事件描述:
 *     {“programName”:”新闻联播”,“programID”:”0x0001”,”serviceID”:”0x30”,“startTime”:” 2011-10-19 19:00:00 UTC+08:00”,“endTime”:”2011-10-19 19:30:00 UTC+08:00”,“programRate”:0, “extendDescriptionInfor”:[{“languageCode”:”chn”,“description”:”国家大事播报”}]}
 *  */
static int _JseRead_dvb_get_channel_more_description(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    if (!getDvbManager().isDateTimeSync()) {
        strcpy(jseValue, "0");
        return -1;
    }

    LogDvbDebug("jseParam: %s\n", jseParam);
    struct json_object* obj = json_tokener_parse(jseParam);
    if (!obj)
        return -1;

    int tChnlKey = json_object_get_int(json_object_object_get(obj, "chanKey"));
    int tEventId = json_object_get_int(json_object_object_get(obj, "programID"));
    json_object_put(obj);

    time_t tSTimeUTC = 0, tETimeUTC = 0;
	struct tm tSTime, tETime;

    std::string tLan = getDvbManager().getCountryCode();
    int tTimezone = 0, tSaveFlag = 0, tSLight = 0, tELight = 0, tOffsetSec = 0;
    bool bTimeDST = false;
    char sTimezone[16] = { 0 };
	sysSettingGetInt("timezone", &tTimezone, 0);
    appSettingGetInt("saveflag", &tSaveFlag, 0);
    appSettingGetInt("lightstart", &tSLight, 0);
    appSettingGetInt("lightstop",  &tELight, 0);
    mid_tool_timezone2str(tTimezone, sTimezone);
    tOffsetSec = get_local_time_zone();
    if(tSaveFlag && (time(0) > tSLight) && (time(0) < tELight)){
        bTimeDST = true;
        tOffsetSec += get_saving_time_sec();
    }

    DvbEpg* epg = 0;
    DvbChannel* channel = getDvbManager().getDvbChannel(tChnlKey);
    if (!channel || !(epg = channel->getEpg()))
        return -1;
    //epg->show();

    EpgEvent* tEvent = epg->getEvent(tEventId);
    if (!tEvent)
        return -1;

	struct json_object* jObject = json_object_new_object();

	struct json_object* jSubArray = 0;
	struct json_object* jSubEventList = 0;

    char tBufferST[64] = { 0 };
    char tBufferET[64] = { 0 };
    epg->lock();
    tSTimeUTC = (time_t)tEvent->mSTime + (time_t)tOffsetSec;
    tETimeUTC = (time_t)tEvent->mETime + (time_t)tOffsetSec;
    gmtime_r(&tSTimeUTC, &tSTime);
    gmtime_r(&tETimeUTC, &tETime);
    sprintf(tBufferST, "%04d-%02d-%02d %02d:%02d:%02d %s%s", 1900+tSTime.tm_year, 1+tSTime.tm_mon, tSTime.tm_mday, tSTime.tm_hour, tSTime.tm_min, tSTime.tm_sec, sTimezone, bTimeDST ? " DST" : "");
    sprintf(tBufferET, "%04d-%02d-%02d %02d:%02d:%02d %s%s", 1900+tETime.tm_year, 1+tETime.tm_mon, tETime.tm_mday, tETime.tm_hour, tETime.tm_min, tETime.tm_sec, sTimezone, bTimeDST ? " DST" : "");
    json_object_object_add(jObject, "programName", json_object_new_string(tEvent->getEventName(tLan).c_str()));
    json_object_object_add(jObject, "programID",   json_object_new_string(UtilsInt2Str(tEvent->mEventId).c_str()));
    json_object_object_add(jObject, "serviceID",   json_object_new_string(UtilsInt2Str(channel->mServiceId).c_str()));
    json_object_object_add(jObject, "startTime",   json_object_new_string(tBufferST));
    json_object_object_add(jObject, "endTime",     json_object_new_string(tBufferET));
    json_object_object_add(jObject, "programRate", json_object_new_int(tEvent->getRating(tLan)));
    if ((jSubArray = json_object_new_array())) {
        std::map<std::string, EventText>& tTexts = tEvent->getEventText();
        std::map<std::string, EventText>::iterator it;
        for (it = tTexts.begin(); it != tTexts.end(); ++it) {
            if ((jSubEventList = json_object_new_object())) {
                json_object_object_add(jSubEventList, "languageString", json_object_new_string(it->second.mLangCode.c_str()));
                json_object_object_add(jSubEventList, "description",    json_object_new_string(it->second.mExtendText.c_str()));
                json_object_array_add(jSubArray, jSubEventList);
            }
        }
    }
    json_object_object_add(jObject, "extendDescriptionInfor", jSubArray);
    epg->unlock();
	strncpy(jseValue, json_object_to_json_string(jObject), JSE_BUFFER_SIZE);
	json_object_put(jObject);
	LogDvbDebug("jseValue = %s\n", jseValue);
	return 0;
}

/*
 *  获取 ID 为 1 的频道当前节目的级别:
 *     Utility.getValueByName(‘dvb_get_channel_rating,{”chanKey”:1}’)
 *  当前节目级别为 5,STB 返回”5”
 *  */
static int _JseRead_dvb_get_channel_rating(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseParam: %s\n", jseParam);
    struct json_object* obj = json_tokener_parse(jseParam);
    if (!obj)
        return -1;

    int tRating  = -1;
    int tChnlKey = json_object_get_int(json_object_object_get(obj, "chanKey"));
    json_object_put(obj);

    time_t tSTimeUTC = 0, tETimeUTC = 0;
    std::string tLan = getDvbManager().getCountryCode();
    DvbEpg* epg = 0;
    DvbChannel* channel = getDvbManager().getDvbChannel(tChnlKey);
    if (!channel || !(epg = channel->getEpg()))
        return -1;

    tSTimeUTC = time(0);
    tETimeUTC = tSTimeUTC + 3600;
    std::vector<EpgEvent*> tEvents = epg->getEvents(tSTimeUTC, tETimeUTC);
    if (tEvents.size() > 0)
        tRating = tEvents[0]->getRating(tLan);
    return sprintf(jseValue, "%d", tRating);
}

static int _JseRead_dvb_get_current_program_position(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    return -1;
}

static int _JseRead_dvb_get_book_count(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
	return sprintf(jseValue, "%d", getDvbManager().getDvbRemindersCount());
}

/*
 * var sValue = Utility.setValueByName(‘dvb_chnl_book’,’{“chanKey”: chanKey,“programID”: “ programID”, “startTime”: “startTime “, “periodType“: periodType,“extendDescription“: “extendDescription”}’);
 *   0:预约成功
 *  -1:预约失败
 *  -2:预约已满
 *  -3:预约冲突
 *  */
static int _JseWrite_dvb_chnl_add_book(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseValue: %s\n", jseValue);
    if (getDvbManager().getDvbRemindersCount() >= REMINDER_MAX_COUNT)
        return -2;
    struct json_object* obj = json_tokener_parse(jseValue);
    if (!obj)
        return -1;
    struct json_object* lookup = 0;
    std::string tStartTimeStr;
    unsigned int year = 0, month = 0, day = 0, hour = 0, min = 0, second = 0;
    DvbReminder* reminder = new DvbReminder();
    if ((lookup = json_object_object_get(obj, "chanKey")))    reminder->mChannelKey = json_object_get_int(lookup);
    if ((lookup = json_object_object_get(obj, "programID")))  reminder->mEventId = atoi(json_object_get_string(lookup));
    if ((lookup = json_object_object_get(obj, "startTime")))  tStartTimeStr = json_object_get_string(lookup);
    if ((lookup = json_object_object_get(obj, "periodType"))) reminder->mPeriodType = json_object_get_int(lookup);
    if ((lookup = json_object_object_get(obj, "extendDescription"))) reminder->mExtendDes = json_object_get_string(lookup);
    json_object_put(obj);
    sscanf(tStartTimeStr.c_str(), "%04d%02d%02d%02d%02d%02d", &year, &month, &day, &hour, &min, &second);
    reminder->mStartTime = UtilsMakeTime(year, month, day, hour, min, second);
    reminder->mBookId = UtilsCreateKey(reminder->mChannelKey, reminder->mEventId, reminder->mStartTime, reminder->mPeriodType);
    if (eDvbOk != getDvbManager().addDvbReminder(reminder)) {
        delete(reminder);
        return -3;
    }
    getDvbManager().saveDB(eDvbReminderDB);
    getDvbManager().sendEmptyMessage(DvbManager::eDvbMessageBookReminder);
    return 0;
}

/*
 * var sValue = Utility.setValueByName (‘dvb_chnl_del_book’, ‘{“bookIDList”:bookIDList }’);
 * 预约 ID 列表,[bookID1,bookID2,...,bookIDn],可以传空数组表示所有。
 *  */
static int _JseWrite_dvb_chnl_del_book(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    LogDvbDebug("jseValue: %s\n", jseValue);
    struct json_object* obj = json_tokener_parse(jseValue);
    if (!obj)
        return -1;
    struct json_object* tArray = json_object_object_get(obj, "bookIDList");
    int tArrayLen = json_object_array_length(tArray);
    if (!tArrayLen) {
        std::vector<int> bookIds = getDvbManager().getDvbReminderKeys();
        for (std::size_t i = 0; i < bookIds.size(); ++i)
            getDvbManager().delDvbReminder(bookIds[i]);

    } else {
        for (int i = 0; i < tArrayLen; ++i)
            getDvbManager().delDvbReminder(json_object_get_int(json_object_array_get_idx(tArray, i)));
    }
    json_object_put(obj);
    getDvbManager().saveDB(eDvbReminderDB);
    return 0;
}

static int _JseRead_dvb_get_book_list(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    struct json_object* obj = json_tokener_parse(jseParam);
    if (!obj)
        return -1;

    int bookNumber = 0;
    int tPosition = json_object_get_int(json_object_object_get(obj, "position"));
    int tBookCnt  = json_object_get_int(json_object_object_get(obj, "count"));
    json_object_put(obj);

	struct json_object* jObject = json_object_new_object();
	struct json_object* jArray  = json_object_new_array();
	struct json_object* jBookList = NULL;

    std::string lan = getDvbManager().getCountryCode();
    EpgEvent* event = 0;
    DvbChannel* channel = 0;
    DvbReminder* reminder = 0;
    std::vector<int> bookIds = getDvbManager().getDvbReminderKeys();
    for (std::size_t i = tPosition; bookNumber < tBookCnt && i < bookIds.size(); ++i) {
        reminder = getDvbManager().getDvbReminder(bookIds[i]);
        if (!reminder)
            continue;
        channel  = getDvbManager().getDvbChannel(reminder->mChannelKey);
        if (channel && (jBookList = json_object_new_object())) {
            event = channel->getEpg()->getEvent(reminder->mEventId);
            json_object_object_add(jBookList, "bookID",  json_object_new_int(reminder->mBookId));
            json_object_object_add(jBookList, "chanKey", json_object_new_int(reminder->mChannelKey));
            if (event && reminder->mEventId > 0)
                json_object_object_add(jBookList, "programID", json_object_new_string(UtilsInt2Str(reminder->mEventId).c_str()));
            json_object_object_add(jBookList, "serviceID", json_object_new_string(UtilsInt2Str(channel->mServiceId).c_str()));
            if (event && reminder->mEventId > 0)
                json_object_object_add(jBookList, "programName", json_object_new_string(event->getEventName(lan).c_str()));
            json_object_object_add(jBookList, "startTime",  json_object_new_string(DvbEpg::timeStr(reminder->mStartTime).c_str()));
            json_object_object_add(jBookList, "periodType", json_object_new_int(reminder->mPeriodType));
            if (reminder->mExtendDes.size() > 0)
                json_object_object_add(jBookList, "extendDescription", json_object_new_string(reminder->mExtendDes.c_str()));
            json_object_array_add(jArray, jBookList);
            bookNumber++;
        }
    }
	json_object_object_add(jObject, "bookNumber", json_object_new_int(bookNumber));
    json_object_object_add(jObject, "bookList", jArray);
	strcpy(jseValue, json_object_to_json_string(jObject));
	json_object_put(jObject);
	LogDvbDebug("jseValue = %s\n", jseValue);
	return 0;
}

/*
 *  获取 STB 上一次的频道系统自定义频道号(不分电视广播,所有频道在一起排列):
 *      var sValue = Utility.getValueByName(‘dvb_get_last_play_chanKey,{“type”:1}’)
 *  STB 上一次播放的视频频道系统频道号为 12 返回
 *      “12”
 *  type: 0:All channels 1:TV channels 2:Broadcast channels
 *  */
static int _JseRead_dvb_get_last_play_by_chankey(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    return -1;
}

static int _JseRead_dvb_get_local_position(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    return -1;
}

static int _JseWrite_dvb_set_local_position( const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    return -1;
}

static int _JseRead_dvb_get_tuner_lnb_connect_type(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    return -1;
}

static int _JseWrite_dvb_set_tuner_lnb_connect_type(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    return -1;
}

static int _JseRead_dvb_get_tuner_num(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    return -1;
}

static int _JseRead_dvb_get_bat_category_list(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    return -1;
}


static int _JseWrite_dvb_satList_update(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    return -1;
}

static int _JseWrite_dvb_testTPList_update(const char* jseFunc, const char* jseParam, char* jseValue, int jseLen)
{
    return -1;
}

int DvbJsCallInit(ioctl_context_type_e type)
{
    a_Hippo_API_JseRegister("dvb_sat_get_count"                   , _JseRead_dvb_sat_get_count                  , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_sat_get_list"                    , _JseRead_dvb_sat_get_list                   , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_my_sat_get_count"                , _JseRead_dvb_my_sat_get_count               , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_my_sat_get_list"                 , _JseRead_dvb_my_sat_get_list                , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_sat_save_params"                 , NULL                                        , _JseWrite_dvb_sat_save_params           , type);
    a_Hippo_API_JseRegister("dvb_sat_add"                         , NULL                       				    , _JseWrite_dvb_sat_add                   , type);
    a_Hippo_API_JseRegister("dvb_sat_delete"                      , NULL  				                        , _JseWrite_dvb_sat_delete                , type);
    a_Hippo_API_JseRegister("dvb_tp_get_count"                    , _JseRead_dvb_tp_get_count                   , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_tp_get_list"                     , _JseRead_dvb_tp_get_list                    , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_tp_save_params"                  , NULL						                , _JseWrite_dvb_tp_save_params            , type);
    a_Hippo_API_JseRegister("dvb_tp_add"                          , NULL                         			    , _JseWrite_dvb_tp_add                    , type);
    a_Hippo_API_JseRegister("dvb_tp_delete"                       , NULL                                        , _JseWrite_dvb_tp_delete                 , type);
    a_Hippo_API_JseRegister("dvb_antenna_auto_setup_start"        , NULL                                        , _JseWrite_dvb_antenna_setup_start       , type);
    a_Hippo_API_JseRegister("dvb_antenna_auto_setup_get_progress" , _JseRead_dvb_antenna_setup_get_progress     , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_antenna_auto_setup_get_info"     , _JseRead_dvb_antenna_setup_get_info         , NULL     							      , type);
    a_Hippo_API_JseRegister("dvb_blind_search_tp_start"           , NULL                                        , _JseWrite_dvb_blind_search_tp_start     , type);
    a_Hippo_API_JseRegister("dvb_blind_search_tp_get_progress"    , _JseRead_dvb_blind_tp_get_progress          , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_new_tp_get_count"                , _JseRead_dvb_new_tp_get_count               , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_new_tp_get_info"                 , _JseRead_dvb_new_tp_get_info                , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_search_stop"                     , NULL                                        , _JseWrite_dvb_search_stop               , type);
    a_Hippo_API_JseRegister("dvb_tp_search_start"                 , NULL                                        , _JseWrite_dvb_tp_search_start           , type);
    a_Hippo_API_JseRegister("dvb_tp_search_get_progress"          , _JseRead_dvb_tp_search_get_progress         , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_sat_search_start"                , NULL                                        , _JseWrite_dvb_sat_search_start          , type);
    a_Hippo_API_JseRegister("dvb_sat_search_get_progress"         , _JseRead_dvb_sat_search_get_progress        , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_new_chnls_get_count"             , _JseRead_dvb_new_chnls_get_count            , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_new_chnls_get_info"              , _JseRead_dvb_new_chnls_get_info             , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_get_chnls_count"                 , _JseRead_dvb_get_chnls_count                , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_get_channel_list"                , _JseRead_dvb_get_channel_list               , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_get_channel_paras_by_chanKey"    , _JseRead_dvb_get_channel_paras_by_chankey   , NULL                                    , type);
	a_Hippo_API_JseRegister("dvb_get_channel_position_by_chanKey" , _JseRead_dvb_get_channel_position_by_chankey, NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_programItem_get_count"           , _JseRead_dvb_programItem_get_count          , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_programItem_get"                 , _JseRead_dvb_programItem_get                , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_get_pf_programItem"              , _JseRead_dvb_get_pf_program_item            , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_get_signal_quality"              , _JseRead_dvb_get_signal_quality             , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_get_signal_strength"             , _JseRead_dvb_get_signal_strength            , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_get_tuner_lock_status"           , _JseRead_dvb_get_tuner_lock_status          , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_tune"                            , NULL                                        , _JseWrite_dvb_tune                      , type);
    a_Hippo_API_JseRegister("dvb_delete_channel"                  , NULL                                        , _JseWrite_dvb_delete_channel            , type);
    a_Hippo_API_JseRegister("dvb_set_channel_name"                , NULL                                        , _JseWrite_dvb_set_channel_name          , type);
    a_Hippo_API_JseRegister("dvb_chnl_search_by_name"             , _JseRead_dvb_chnl_search_by_name            , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_get_channel_more_description"    , _JseRead_dvb_get_channel_more_description   , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_get_channel_rating"              , _JseRead_dvb_get_channel_rating             , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_get_current_program_position"    , _JseRead_dvb_get_current_program_position   , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_get_last_play_chanKey"           , _JseRead_dvb_get_last_play_by_chankey       , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_get_local_position"              , _JseRead_dvb_get_local_position             , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_set_local_position"              , NULL                                        , _JseWrite_dvb_set_local_position        , type);
    a_Hippo_API_JseRegister("dvb_get_tuner_lnb_connect_type"      , _JseRead_dvb_get_tuner_lnb_connect_type     , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_set_tuner_lnb_connect_type"      , NULL                                        , _JseWrite_dvb_set_tuner_lnb_connect_type, type);
    a_Hippo_API_JseRegister("dvb_get_tuner_num"                   , _JseRead_dvb_get_tuner_num                  , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_get_bat_category_list"           , _JseRead_dvb_get_bat_category_list          , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_get_book_count"                  , _JseRead_dvb_get_book_count                 , NULL                                    , type);
    a_Hippo_API_JseRegister("dvb_chnl_book"                       , NULL                                        , _JseWrite_dvb_chnl_add_book             , type);
    a_Hippo_API_JseRegister("dvb_chnl_del_book"                   , NULL                                        , _JseWrite_dvb_chnl_del_book             , type);
    a_Hippo_API_JseRegister("dvb_get_book_list"                   , _JseRead_dvb_get_book_list                  , NULL                                    , type);

    a_Hippo_API_JseRegister("dvb_satList_update"                  , NULL                                        , _JseWrite_dvb_satList_update            , type);
    a_Hippo_API_JseRegister("dvb_testTPList_update"               , NULL                                        , _JseWrite_dvb_testTPList_update         , type);
    a_Hippo_API_JseRegister("dvb_get_bat_category_list"           , _JseRead_dvb_get_bat_category_list          , NULL                                    , type);

    return 0;
}
