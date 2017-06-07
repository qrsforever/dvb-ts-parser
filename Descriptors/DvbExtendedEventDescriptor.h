#ifndef __DvbExtendedEventDescriptor__H_
#define __DvbExtendedEventDescriptor__H_

#include "DvbDescriptor.h"
#include <iostream>
#include <string>

class DvbExtendedEventDescriptor : public DvbDescriptor {
public:
    DvbExtendedEventDescriptor(const DvbDescriptor& descriptor);
    ~DvbExtendedEventDescriptor() { 
    }
    std::string getIso639Lan() const;
    std::string getItemDescript() const;
    std::string getExtendedText() const;
private:
    int mItemLength;
    int mTextLength;
};

#endif
