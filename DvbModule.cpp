#include <stdio.h>
#include <unistd.h>

#include "DvbAssertions.h"
#include "DvbEnum.h"
#include "DvbManager.h"
#include "DvbScanEpg.h"
#include "DvbScanOta.h"
#include "DvbChannel.h"
#include "DvbScanChannel.h"
#include "DvbScanDateTime.h"
#include "Transponder.h"
#include "Frontend.h"
#include "FrontendDVBS.h"
#include "DvbDevice.h"
#include "DvbBlindScan.h"
#include "DvbEpg.h"
#include "DvbJsCall.h"
#include "SQLiteTableDVB.h"

#include "SysSetting.h"

#include "libxml/parser.h"
#include "libxml/tree.h"
#include <iostream>
#include <vector>
#include <list>

#include "libzebra.h"

/*
 *   sat.xml
 *   <?xml version="1.0" encoding="iso-8859-1"?>
 *   <satellites>
 *   <sat name="Hot Bird 6/8/9 (13.0E)" flags="0" position="130">
 *       <transponder frequency="10719000" symbol_rate="27500000" polarization="1" fec_inner="4" system="0" modulation="1"/>
 *       <transponder frequency="10758000" symbol_rate="27500000" polarization="1" fec_inner="4" system="0" modulation="1"/>
 *   </sat>
 *   <sat name="Eurobird 2 (25.5E)" flags="0" position="255">
 *       <transponder frequency="10996000" symbol_rate="2532000" polarization="1" fec_inner="3" system="0" modulation="1"/>
 *       <transponder frequency="11006000" symbol_rate="2200000" polarization="1" fec_inner="3" system="0" modulation="1"/>
 *   </sat>
 *   </satellites>
 *  */
static int _sat_xml_parser(const char* file, const char* encode)
{
    xmlDocPtr doc = 0;
    xmlNodePtr curNode = 0;
    xmlNodePtr lowNode = 0;
    doc = xmlReadFile(file, encode, XML_PARSE_RECOVER);
    if (!doc) {
        LogDvbError("Document not parsed successfully.\n");
        return -1;
    }
    curNode = xmlDocGetRootElement(doc);
    if (!curNode) {
        LogDvbError("xmlDocGetRootElement error!\n");
        xmlFreeDoc(doc);
        return -1;
    }
    if (xmlStrcmp(curNode->name, (xmlChar*)"satellites")) {
        LogDvbError("root node name is not satellites!\n");
        xmlFreeDoc(doc);
        return -1;
    }

    for (curNode = curNode->xmlChildrenNode; curNode; curNode = curNode->next) {
        //LogDvbDebug("curNode->name: %s\n", curNode->name);
        if (xmlStrcmp(curNode->name, (xmlChar*)"sat"))
            continue;
        DvbDeviceDVBS* device = new DvbDeviceDVBS();
        if (!device)
            continue;
        xmlChar* satename = xmlGetProp(curNode, (xmlChar*)"name");
        xmlChar* position = xmlGetProp(curNode, (xmlChar*)"position");
        LogDvbDebug("sat name = %s %s\n", satename, position);
        device->mSateliteName = (const char*)satename;
        device->mSatLongitude = (const char*)position;
        device->mFavorFlag  = 0;
        device->mDeviceKey  = UtilsCreateKey(device->mTunerId, device->mDiseqcPort, device->mToneMode, atoi((const char*)position));
        getDvbManager().addDvbDevice(device);
        xmlFree(satename);
        xmlFree(position);
        for (lowNode = curNode->children; lowNode; lowNode = lowNode->next) {
            //LogDvbDebug("lowNode->name: %s\n", lowNode->name);
            if (xmlStrcmp(lowNode->name, (xmlChar*)"transponder"))
                continue;
            TransponderDVBS* transponder = new TransponderDVBS(device);
            if (!transponder)
                continue;
            xmlChar* frequency    = xmlGetProp(lowNode, (xmlChar*)"frequency");
            xmlChar* symbolrate   = xmlGetProp(lowNode, (xmlChar*)"symbol_rate");
            xmlChar* polarization = xmlGetProp(lowNode, (xmlChar*)"polarization");
            xmlChar* fecinner     = xmlGetProp(lowNode, (xmlChar*)"fec_inner");
            xmlChar* modulation   = xmlGetProp(lowNode, (xmlChar*)"modulation");
            LogDvbDebug("frequency = %s %s %s %s %s\n", frequency, symbolrate, polarization, fecinner, modulation);
            transponder->mFrequency  = atoi((const char*)frequency);
            transponder->mSymbolRate = atoi((const char*)symbolrate);
            switch (atoi((const char*)polarization)) {
                case 0: transponder->mPolarization = ePolarHorizontal   ; break;
                case 1: transponder->mPolarization = ePolarVertical     ; break;
                case 2: transponder->mPolarization = ePolarCircularLeft ; break;
                case 3: transponder->mPolarization = ePolarCircularRight; break;
                default:
                        transponder->mPolarization = ePolarHorizontal;
            }
            switch (atoi((const char*)fecinner)) {
                case 1: transponder->mFecRate = eFecRate1_2; break;
                case 2: transponder->mFecRate = eFecRate2_3; break;
                case 3: transponder->mFecRate = eFecRate3_4; break;
                case 4: transponder->mFecRate = eFecRate5_6; break;
                case 5: transponder->mFecRate = eFecRate7_8; break;
                case 6: transponder->mFecRate = eFecRate8_9; break;
                case 7: transponder->mFecRate = eFecRate3_5; break;
                case 8: transponder->mFecRate = eFecRate3_5; break;
                case 9: transponder->mFecRate = eFecRate9_10; break;
                default:
                        transponder->mFecRate = eFecRate3_4;
            }
            switch (atoi((const char*)modulation)) {
                case 0: transponder->mModulation = eModulationAuto;  break;
                case 1: transponder->mModulation = eModulationQpsk;  break;
                case 2: transponder->mModulation = eModulationPsk8;  break;
                case 3: transponder->mModulation = eModulationQam16; break;
                default:
                        transponder->mModulation = eModulationQpsk;
            }
            transponder->mUniqueKey = UtilsCreateKey(transponder->mFrequency, transponder->mSymbolRate, transponder->mPolarization, 0);
            getDvbManager().addTransponder(transponder);
            xmlFree(frequency);
            xmlFree(symbolrate);
            xmlFree(polarization);
            xmlFree(fecinner);
            xmlFree(modulation);
        }
    }
    xmlFreeDoc(doc);
    return 0;
}

extern "C" int dvb_module_init()
{
    int flag = access(DVB_SQLITEDB_PATH, F_OK);
    DvbJsCallInit(IoctlContextType_eHWBaseC20);
    getDvbManager().loadDB();
    if (flag) {
        LogDvbDebug("not exist %s\n", DVB_SQLITEDB_PATH);
        if (!_sat_xml_parser("/home/hybroad/share/sat.xml", "iso-8859-1"))
            getDvbManager().saveDB(eDvbDeviceDB | eTransponderDB);
    }
 #if 0
    //Lock Upgrade Frequency
    int tDFrequency = 0;
    int tSymbolrate = 0;
    int tPolarization = 0;
    sysSettingGetInt("MainFrequency", &tDFrequency, 0);
    sysSettingGetInt("MainSymbolRate", &tSymbolrate, 0);
    sysSettingGetInt("MainPolarization", &tPolarization, 0);
    TransponderDVBS* transponder = 0;
    DvbDeviceDVBS* device = new DvbDeviceDVBS();
    if (device) {
        device->mTunerId = 1;
        transponder = new TransponderDVBS(device);
        if (transponder) {
            transponder->mFrequency    = tDFrequency;
            transponder->mSymbolRate   = tSymbolrate;
            transponder->mPolarization = (Polarization_e)tPolarization;
            bool isTune = getDvbManager().getFrontend()->tune(device->mTunerId, transponder);
            for (int i = 0; !isTune && i < 3; i++)
                isTune = getDvbManager().getFrontend()->tune(device->mTunerId, transponder);
            delete(transponder);
        }
        delete(device);
    }
    DvbScanOta* ota = new DvbScanOta(SCAN_OTA_NAME, Hippo::upgradeManager());
    getDvbManager().addDvbScan(ota);
    ota->scanStart();
    sleep(1);
    ota->startCheck(0);
#else
    DvbScan* scan = 0;
    scan = new DvbScanEpg(SCAN_EPG_NAME, 1);
    getDvbManager().addDvbScan(scan);
    scan->scanStart();

    scan = new DvbScanDateTime(SCAN_DATETIME_NAME, 1);
    getDvbManager().addDvbScan(scan);
    scan->scanStart();
#endif

    //getDvbManager().show();
    return 0;
}

extern "C" void dvb_module_ntpsync(int flag)
{
    LogDvbDebug("ntp sync flag [%d]\n", flag);
    if (1 != flag)
        return;
    DvbScan* scan = getDvbManager().getDvbScan(SCAN_DATETIME_NAME);
    if (!scan) {
        scan = new DvbScanDateTime(SCAN_DATETIME_NAME, 1);
        getDvbManager().addDvbScan(scan);
        scan->scanStart();
    }
}
