#ifndef __Frontend__DVBC__H_
#define __Frontend__DVBC__H_

#include "Frontend.h"

class Transponder;
class FrontendDVBC : public Frontend {
public:
    FrontendDVBC();
    ~FrontendDVBC();
    bool tune(int tunerId, Transponder* transponder);
    bool isTuned(int tunerId);
    int getSignalStrength(int tunerId);
    int getSignalQuality(int tunerId);
    int getParserBand(int tunerId);
    int getSnr(int tunerId);
private:
};
#endif
