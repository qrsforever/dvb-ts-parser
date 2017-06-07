#ifndef __Frontend__DVBT__H_
#define __Frontend__DVBT__H_

#include "Frontend.h"

class FrontendDVBT : public Frontend {
public :
    FrontendDVBT();
    ~FrontendDVBT();
    bool tune(int tunerId, Transponder* transponder);
    bool isTuned(int tunerId);
    int getSignalStrength(int tunerId);
    int getSignalQuality(int tunerId);
    int getParserBand(int tunerId);
    int getSnr(int tunerId);
};
#endif
