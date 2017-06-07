#include "DvbEnum.h"

const char* EnumToStr(TransmissionType_e transmissionType)
{
    switch (transmissionType) {
        case eTransmissionTypeDVBC : return "DVBC";
        case eTransmissionTypeDVBS : return "DVBS";
        case eTransmissionTypeDVBT : return "DVBC";
    }
    return "NONE";
}

const char* EnumToStr(FecRate_e fecRate)
{
    switch (fecRate) {
        case eFecRate1_2 : return "1/2";
		case eFecRate2_3 : return "2/3";
		case eFecRate3_4 : return "3/4";
		case eFecRate4_5 : return "4/5";
		case eFecRate5_6 : return "5/6";
        case eFecRate6_7 : return "6/7";
		case eFecRate7_8 : return "7/8";
		case eFecRate8_9 : return "8/9";
        case eFecRateAuto: return "AUTO";
        case eFecRate1_3 : return "1/3";
        case eFecRate1_4 : return "1/4";
        case eFecRate2_5 : return "2/5";
        case eFecRate3_5 : return "3/5";
        case eFecRate9_10: return "9/10";
    }
    return "NONE";
}

const char* EnumToStr(Bandwidth_e bandwidth)
{
    switch (bandwidth) {
		case eBandwidth8MHz : return "8MHz";
		case eBandwidth7MHz : return "7MHz";
		case eBandwidth6MHz : return "6MHz";
		case eBandwidth5MHz : return "5MHz"; 
    }
    return "NONE";
}

const char* EnumToStr(Modulation_e modulation)
{
    switch (modulation) {
        case eModulationAuto  : return "Auto"; 
        case eModulationQpsk  : return "Qpsk"; 
        case eModulationPsk8  : return "Psk8";
        case eModulationQam16 : return "Qam16";
        case eModulationQam32 : return "Qam32";
        case eModulationQam64 : return "Qam64";
        case eModulationQam128: return "Qam128";
        case eModulationQam256: return "Qam256";
    }
    return "NONE";
}

const char* EnumToStr(Polarization_e polarization)
{
	switch (polarization) {
        case ePolarHorizontal   : return "H";
        case ePolarVertical     : return "V";
        case ePolarCircularLeft : return "L";
        case ePolarCircularRight: return "R";
	}
	return "NONE";
}

const char* EnumToStr(ToneMode_ tune)
{
    switch (tune) {
        case eToneOff  : return "ToneOff";
        case eToneOn   : return "ToneOn";
        case eToneAuto : return "ToneAuto";
    }
    return "NONE";
}

const char* EnumToStr(DiseqcPort_e port)
{
    switch (port) {
        case eDiseqcPortA: return "A";
        case eDiseqcPortB: return "B";
        case eDiseqcPortC: return "C";
        case eDiseqcPortD: return "D";
    }
    return "NONE";
}

const char* EnumToStr(ServiceType_e type)
{
    switch (type) {
        case eServiceAll  : return "All";
        case eServiceTv   : return "TV";
        case eServiceRadio: return "Radio";
    }
    return "NONE";
}

const char* EnumToStr(ChannelType_e type)
{
    switch (type) {
        case eChannelAll : return "All";
        case eChannelFta : return "FTA";
        case eChannelCa  : return "CA";
    }
    return "NONE";
}

const char* EnumToStr(DefaultPsiSiPid_t pid)
{
    switch (pid) {
        case ePatPid : return "PAT";
        case eCatPid : return "CAT";
        case eNitPid : return "NIT";
        case eSdtPid : return "SDT&BAT";
        case eEitPid : return "EIT";
        case eTotPid : return "TDT&TOT";
    }
    return "NONE";
}
