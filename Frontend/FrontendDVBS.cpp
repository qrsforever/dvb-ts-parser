#include "DvbAssertions.h"
#include "FrontendDVBS.h"
#include "Transponder.h"
#include "DvbDevice.h"
#include "libzebra.h"

FrontendDVBS::FrontendDVBS()
{
    DTV_dvbs_init();
}

FrontendDVBS::~FrontendDVBS()
{
}

bool 
FrontendDVBS::tune(int tunerId, Transponder* tp)
{
    TransponderDVBS* transponder = static_cast<TransponderDVBS*>(tp);
    DTV_FRONTEND_TUNER_MODULE* module = DTV_frontend_OpenTunerModule(tunerId);
    if (!transponder || !module)
        return false;
    DvbDeviceDVBS* device = static_cast<DvbDeviceDVBS*>(transponder->mDvbDevice);
    int polarity = 0;
    switch (transponder->mPolarization) {
        case ePolarHorizontal   : polarity = 0; break;
        case ePolarVertical     : polarity = 1; break;
        case ePolarCircularLeft : polarity = 0; break;
        case ePolarCircularRight: polarity = 0; break;
        default: polarity = 0;
    }
    int switch22K = 0; 
    switch (device->mToneMode) {
        case eToneOff  : switch22K = 0; break;
        case eToneOn   : switch22K = 1; break;
        case eToneAuto : switch22K = 0; break;
        default: switch22K = 0;
    }
    int diseqcPort = 0;
    switch (device->mDiseqcPort) {
        case eDiseqcPortA : diseqcPort = 1; break;
        case eDiseqcPortB : diseqcPort = 2; break;
        case eDiseqcPortC : diseqcPort = 3; break;
        case eDiseqcPortD : diseqcPort = 4; break;
        default: diseqcPort = 1;
    }

    module->setting.frequency_10KHz = transponder->mIFrequency ? (transponder->mIFrequency / 10) :(transponder->DF2IF(transponder->mFrequency) / 10);
    module->setting.symborate_Kbps  = transponder->mSymbolRate;
    module->setting.bandWidth_MHz   = transponder->mBandWidth;
    module->setting.QAM             = transponder->mModulation;
    module->setting.polarity        = polarity;
    module->setting.switch22K       = switch22K;
    module->setting.diseqc_port     = diseqcPort;
  
    if (DTV_frontend_Tune(module)) {
        LogDvbWarn("can not lock Freq10K[%d] Symb[%d] Polar[%d]\n", module->setting.frequency_10KHz, module->setting.symborate_Kbps, module->setting.polarity);
        return false;
    }
    return (1 == DTV_frontend_GetLockStatus(module));
}

bool 
FrontendDVBS::tune(int tunerId, int freq, int symb, int polar)
{
    DTV_FRONTEND_TUNER_MODULE* module = DTV_frontend_OpenTunerModule(tunerId);
    if (!module)
        return false;
    module->setting.frequency_10KHz = freq / 10;
    module->setting.symborate_Kbps  = symb;
    module->setting.polarity        = polar;
    return (1 == DTV_frontend_GetLockStatus(module));
}

bool
FrontendDVBS::isTuned(int tunerId)
{
    DTV_FRONTEND_TUNER_MODULE* module = DTV_frontend_OpenTunerModule(tunerId);
    if (!module)
        return false;
    return  (1 == DTV_frontend_GetLockStatus(module));
}

int
FrontendDVBS::getSignalStrength(int tunerId)
{
    DTV_FRONTEND_TUNER_MODULE* module = DTV_frontend_OpenTunerModule(tunerId);
    if (!module)
        return 0;
    return DTV_frontend_GetStrength(module) / 25;     
}

int
FrontendDVBS::getSignalQuality(int tunerId)
{
    DTV_FRONTEND_TUNER_MODULE* module = DTV_frontend_OpenTunerModule(tunerId);
    if (!module)
        return 0;
    return DTV_frontend_GetQuality(module) / 25;     
}

int 
FrontendDVBS::getParserBand(int tunerId)
{
    DTV_FRONTEND_TUNER_MODULE* module = DTV_frontend_OpenTunerModule(tunerId);
    if (!module)
        return -1;
    return module->parserBand;
}

int
FrontendDVBS::getSnr(int tunerId)
{
    //DTV_FRONTEND_TUNER_MODULE* module = DTV_frontend_OpenTunerModule(tunerId);
    return -1;
}

void 
FrontendDVBS::setDiseqcPort(int tunerId, DiseqcPort_e port)
{
    DTV_FRONTEND_TUNER_MODULE* module = DTV_frontend_OpenTunerModule(tunerId);
    if (!module)
        return;
    switch (port) {
        case eDiseqcPortA: module->setting.diseqc_port = 1; break;
        case eDiseqcPortB: module->setting.diseqc_port = 2; break;
        case eDiseqcPortC: module->setting.diseqc_port = 3; break;
        case eDiseqcPortD: module->setting.diseqc_port = 4; break;
        default: return;
    }
    DTV_DVBSfrontend_SetDiseqc(module);
}

void 
FrontendDVBS::setToneMode(int tunerId, ToneMode_e tone)
{
    DTV_FRONTEND_TUNER_MODULE* module = DTV_frontend_OpenTunerModule(tunerId);
    if (!module)
        return;
    switch (tone) {
        case eToneOff : module->setting.switch22K = 0; break;
        case eToneOn  : module->setting.switch22K = 1; break;
        case eToneAuto: module->setting.switch22K = 0; break;
        default: return;
    }
    DTV_DVBSfrontend_Set22K(module);
}

void 
FrontendDVBS::setLnbPower(int tunerId, LnbPower_e power)
{
    DTV_FRONTEND_TUNER_MODULE* module = DTV_frontend_OpenTunerModule(tunerId);
    if (!module)
        return;
    switch (power) {
        case ePowerOff: break;
        case ePowerOn : break;
        default: return;
    }
    //TODO
}

void 
FrontendDVBS::setDiseqcBurst(int tunerId, DiseqcBurst_e burst)
{
    DTV_FRONTEND_TUNER_MODULE* module = DTV_frontend_OpenTunerModule(tunerId);
    if (!module)
        return;
    switch (burst) {
        case eBurstNone : break;
        case eBurstMiniA: break;
        case eBurstMiniB: break;
        default: return;
    }
    //TODO
}

void 
FrontendDVBS::setRotorDirection(int tunerId, RotorDirection_e rotor)
{
    DTV_FRONTEND_TUNER_MODULE* module = DTV_frontend_OpenTunerModule(tunerId);
    if (!module)
        return;
    switch (rotor) {
        case eRotorEast : break;
        case eRotorWest : break;
        case eRotorSouth: break;
        case eRotorNorth: break;
        default: return;
    }
    //TODO
}
