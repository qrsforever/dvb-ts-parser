#ifndef __DvbLinkageDescriptor__H_
#define __DvbLinkageDescriptor__H_

#include "DvbDescriptor.h"
#include <iostream>
#include <string>

class DvbLinkageDescriptor : public DvbDescriptor {
public:
    DvbLinkageDescriptor(const DvbDescriptor& descriptor);
    ~DvbLinkageDescriptor() { 
    }
    uint16_t getTransportStreamId() const;
    uint16_t getOriginalNetworkId() const;
    uint16_t getServiceId() const;
    uint8_t getLinkageType() const;
    const uint8_t* getLinkageData() const;
    uint32_t getLinkageLength() const;
};

#endif
