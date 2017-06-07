#ifndef __DvbScanEpg__H_
#define __DvbScanEpg__H_

#include "DvbScan.h"
#include "Transponder.h"

#include <iostream>
#include <map>
#include <string>

class DvbChannel;
class DvbScanEpg : public DvbScan {
public:
    DvbScanEpg(std::string name, int tunerId);
    ~DvbScanEpg();

    virtual int parseSection(const uint8_t* data, uint16_t size);
    virtual int getProgress() { return 0; };
protected:
    virtual void Run(void* threadID);
    int _ParseEIT(const uint8_t* data, uint16_t size);
private:
    int mTunerId;
};

#endif
