#ifndef __Frontend__H_
#define __Frontend__H_

#include "DvbEnum.h"

class Transponder;
class Frontend {
public:
    virtual bool tune(int tunerId, Transponder* transponder) = 0;
    virtual bool isTuned(int tunerId) = 0;
    virtual int getSignalStrength(int tunerId) = 0;
    virtual int getSignalQuality(int tunerId) = 0;
    virtual int getParserBand(int tunerId) = 0;
    virtual int getSnr(int tunerId) = 0;
    virtual ~Frontend() {};
protected:
    Frontend() { };
private:
};

#endif
