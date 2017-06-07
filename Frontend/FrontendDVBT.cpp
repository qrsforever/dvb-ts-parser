#include "FrontendDVBT.h"

FrontendDVBT::FrontendDVBT()
{
}

FrontendDVBT::~FrontendDVBT()
{
}

bool 
FrontendDVBT::tune(int tunerId, Transponder* transponder)
{
    return false;
}

bool 
FrontendDVBT::isTuned(int tunerId)
{
    return false;
}

int
FrontendDVBT::getSignalStrength(int tunerId)
{
    return 0;
}

int
FrontendDVBT::getSignalQuality(int tunerId)
{
    return 0;
}

int
FrontendDVBT::getParserBand(int tunerId)
{
    return -1;
}

int
FrontendDVBT::getSnr(int tunerId)
{
    return -1;
}
