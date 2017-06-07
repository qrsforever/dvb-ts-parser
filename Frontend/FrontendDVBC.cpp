#include "FrontendDVBC.h"

FrontendDVBC::FrontendDVBC()
{
}

FrontendDVBC::~FrontendDVBC()
{
}

bool 
FrontendDVBC::tune(int tunerId, Transponder* transponder)
{
    return false;
}

bool 
FrontendDVBC::isTuned(int tunerId)
{
    return false;
}

int 
FrontendDVBC::getSignalStrength(int tunerId)
{
    return -1;
}

int 
FrontendDVBC::getSignalQuality(int tunerId)
{
    return -1;
}

int 
FrontendDVBC::getParserBand(int tunerId)
{
    return -1;
}

int 
FrontendDVBC::getSnr(int tunerId)
{
    return -1;
}

