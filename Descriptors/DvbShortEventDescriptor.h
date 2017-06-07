#ifndef __DvbShortEvent__H_
#define __DvbShortEvent__H_

#include "DvbDescriptor.h"
#include <iostream>
#include <string>

class DvbShortEventDescriptor : public DvbDescriptor {
public:
    DvbShortEventDescriptor(const DvbDescriptor& descriptor);
    ~DvbShortEventDescriptor(){
    }
    std::string getIso639Lan() const;
    std::string getEventName() const;
    std::string getShortText() const;
private:
    int mNameLength;
    int mTextLength;
};

#endif
