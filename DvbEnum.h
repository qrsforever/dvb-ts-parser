#ifndef __DVBENUM__H_
#define __DVBENUM__H_

typedef enum DvbErrorCode_ {
    eDvbBadParam = -3,
    eDvbObjExist = -2,
    eDvbError    = -1,
    eDvbOk       = 0,
}DvbErrorCode_e;

typedef enum TransmissionType_ {
    eTransmissionTypeDVBC = 1,
    eTransmissionTypeDVBS = 2,
    eTransmissionTypeDVBT = 3,
}TransmissionType_e ;

typedef enum FecRate_ {
    eFecRate1_2 = 1,
    eFecRate2_3 = 2,
    eFecRate3_4 = 3,
    eFecRate4_5 = 4,
    eFecRate5_6 = 5,
    eFecRate6_7 = 6,
    eFecRate7_8 = 7,
    eFecRate8_9 = 8,
    eFecRateAuto= 9,
    eFecRate1_3 = 10,
    eFecRate1_4 = 11,
    eFecRate2_5 = 12,
    eFecRate3_5 = 13,
    eFecRate9_10= 14
}FecRate_e;

typedef enum Modulation_ {
    eModulationAuto  = 0,
    eModulationQpsk  = 1,
    eModulationPsk8  = 2,
    eModulationQam16 = 3,
    eModulationQam32 = 4,
    eModulationQam64 = 5,
    eModulationQam128 = 6,
    eModulationQam256 = 7
}Modulation_e;

typedef enum Bandwidth_ {
    eBandwidth8MHz = 0,
    eBandwidth7MHz = 1,
    eBandwidth6MHz = 2,
    eBandwidth5MHz = 3,
}Bandwidth_e;

typedef enum Polarization_ {
    ePolarHorizontal    = 0,
    ePolarVertical      = 1,
    ePolarCircularLeft  = 2,
    ePolarCircularRight = 3
}Polarization_e;

typedef enum LnbType_ {
    eLnbManual    = 0,  /* User can manually select LNB */
    eLnbStandard  = 1,  /* fixed: LOF1:5150 LOF2:5750 */
    eLnbUniversal = 2   /* fixed: LOF1:9750 LOF2:10600 */
}LnbType_e;

typedef enum LnbPower_ {
    ePowerOff = 0,
    ePowerOn  = 1,
}LnbPower_e;

typedef enum ToneMode_ {
    eToneAuto = 0,
    eToneOff  = 1,
    eToneOn   = 2,
}ToneMode_e ;

typedef enum DiseqcPort_ {
    eDiseqcPortA = 0,
    eDiseqcPortB = 1,
    eDiseqcPortC = 2,
    eDiseqcPortD = 3,
}DiseqcPort_e ;

typedef enum DiseqcBurst_ {
    eBurstNone  = 0,
    eBurstMiniA = 1,
    eBurstMiniB = 2
}DiseqcBurst_e ;

typedef enum RotorDirection_ {
    eRotorEast  = 0,
    eRotorWest  = 1,
    eRotorSouth = 2,
    eRotorNorth = 3
}RotorDirection_e;

typedef enum DemuxFilterType_{
    eDemuxTsFilter  = 0,
    eDemuxPsiFilter = 1,
    eDemuxPesFilter = 2,
}DemuxFilterType_e;

typedef enum DefaultPsiSiPid_ {
    ePatPid = 0x0000,
    eCatPid = 0x0001,
    eNitPid = 0x0010,
    eSdtPid = 0x0011,
    eBatPid = 0x0011,
    eEitPid = 0x0012,
    eTdtPid = 0x0014,
    eTotPid = 0x0014,
}DefaultPsiSiPid_t;

typedef enum ServiceType_{
   eServiceAll   = 0,
   eServiceTv    = 1,              
   eServiceRadio = 2,
}ServiceType_e;

typedef enum ChannelType_ {
    eChannelAll = 0,
    eChannelFta = 1,
    eChannelCa  = 2,
}ChannelType_e;

typedef enum DatabaseTableID_ {
    eDvbDeviceDB    = 0x0001,
    eTransponderDB  = 0x0010,
    eDvbChannelDB   = 0x0100,
    eDvbReminderDB  = 0x1000,
}DatabaseTableID_e;

const char* EnumToStr(TransmissionType_e transmissionType);
const char* EnumToStr(FecRate_e fecRate);
const char* EnumToStr(Modulation_e modulation);
const char* EnumToStr(Polarization_e polarization);
const char* EnumToStr(Bandwidth_e bandwidth);
const char* EnumToStr(ToneMode_ tune);
const char* EnumToStr(DiseqcPort_e port);
const char* EnumToStr(ServiceType_e type);
const char* EnumToStr(ChannelType_e type);
const char* EnumToStr(DefaultPsiSiPid_t pid);

#endif
