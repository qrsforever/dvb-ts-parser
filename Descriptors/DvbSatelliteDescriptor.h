#ifndef __DvbSatelliteDescriptor__H_
#define __DvbSatelliteDescriptor__H_

#include "DvbDescriptor.h"

class DvbSatelliteDescriptor : public DvbDescriptor {
public:
    DvbSatelliteDescriptor(const DvbDescriptor& descriptor);
    ~DvbSatelliteDescriptor() { 
    }
    uint32_t getFrequency() const;
    uint16_t getOrbitalPostion() const;
    uint8_t getWestEastFlag() const;
    uint8_t getPolarization() const;
    uint8_t getModulationType() const; 
    uint32_t getSymbolRate() const ;
};

#endif
