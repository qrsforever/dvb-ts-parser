#ifndef __DvbScanDateTime__H_
#define __DvbScanDateTime__H_

#include "DvbScan.h"
#include "Transponder.h"

#include <iostream>
#include <map>
#include <string>

class DvbChannel;
class DvbScanDateTime : public DvbScan {
public:
    DvbScanDateTime(std::string name, int tunerId);
    ~DvbScanDateTime();

    virtual int parseSection(const uint8_t* data, uint16_t size);
    virtual int getProgress() { return 0; };
protected:
    virtual void Run(void* threadID);
    int _ParseTDT(const uint8_t* data, uint16_t size);
    int _ParseTOT(const uint8_t* data, uint16_t size);
private:
    int mTunerId;
};

#endif
