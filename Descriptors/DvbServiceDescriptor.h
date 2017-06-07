#ifndef __DvbServiceDescriptor__H_
#define __DvbServiceDescriptor__H_

#include "DvbDescriptor.h"
#include <iostream>
#include <string>

class DvbServiceDescriptor : public DvbDescriptor {
public:
    DvbServiceDescriptor(const DvbDescriptor& descriptor);
    ~DvbServiceDescriptor() { 
    }
    int getServiceType() const ;
    std::string getProviderName() const;
    std::string getServiceName() const;
private:
    int mProviderNameLength;
    int mServiceNameLength;
};

#endif
