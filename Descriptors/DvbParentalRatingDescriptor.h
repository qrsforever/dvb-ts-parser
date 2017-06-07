#ifndef __DvbParentalRatingDescriptor__H_
#define __DvbParentalRatingDescriptor__H_

#include "DvbDescriptor.h"
#include <iostream>
#include <map>
#include <string>

class DvbParentalRatingDescriptor : public DvbDescriptor {
public:
    DvbParentalRatingDescriptor(const DvbDescriptor& descriptor);
    ~DvbParentalRatingDescriptor() {
        mParentalRatings.clear();
    }
    std::map<std::string, int> getRatings() const;
    int getRating(const char* code);
private:
    std::map<std::string, int> mParentalRatings;
};

#endif
