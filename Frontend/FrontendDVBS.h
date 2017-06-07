#ifndef __FrontendDVBS__H_
#define __FrontendDVBS__H_

#include "Frontend.h"
#include "DvbEnum.h"

class FrontendDVBS : public Frontend {
public :
    FrontendDVBS();
    ~FrontendDVBS();
    bool tune(int tunerId, Transponder* transponder);
    bool tune(int tunerId, int freq, int symb, int polar);
    bool isTuned(int tunerId);
    int getSignalStrength(int tunerId);
    int getSignalQuality(int tunerId);
    int getParserBand(int tunerId);
    int getSnr(int tunerId);

    void setDiseqcPort(int tunerId, DiseqcPort_e port);
    void setToneMode(int tunerId, ToneMode_e tone);
    void setLnbPower(int tunerId, LnbPower_e power);
    void setDiseqcBurst(int tunerId, DiseqcBurst_e burst);
    void setRotorDirection(int tunerId, RotorDirection_e rotor);
private:
};
#endif
