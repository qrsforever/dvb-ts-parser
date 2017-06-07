#include "DvbAssertions.h"
#include "DvbUtils.h"
#include "DvbParentalRatingDescriptor.h"

/*
parental_rating_descriptor(){
    descriptor_tag          8
    descriptor_length       8
    for (i=0;i<N;i++){
        country_code        24
        rating              8
    }
}
*/

DvbParentalRatingDescriptor::DvbParentalRatingDescriptor(const DvbDescriptor& descriptor) : DvbDescriptor(descriptor)
{
    /*   6 = (8 + 8 + 24 + 8) / 8
     *   descriptor_tag          8
     *   descriptor_length       8
     *   country_code            24
     *   rating                  8
     *  */
    if (getLength() < 6) { // 6 is the length of the parental_rating_descriptor at least
        initData();
        return;
    }
    uint32_t offsetBytes = 0;
    uint32_t count = (getLength() - 2) / 4;
    for (uint32_t i = 0; i < count; ++i) {
        offsetBytes = i * 4;
        std::string countryCode("");
        countryCode.append(1, (uint8_t)UtilsGetBits(getData(), offsetBytes, 16, 8));
        countryCode.append(1, (uint8_t)UtilsGetBits(getData(), offsetBytes, 24, 8));
        countryCode.append(1, (uint8_t)UtilsGetBits(getData(), offsetBytes, 32, 8));
        mParentalRatings[countryCode] = (int)UtilsGetBits(getData(), offsetBytes, 40, 8);
    }
}

std::map<std::string, int>
DvbParentalRatingDescriptor::getRatings() const
{
    return mParentalRatings;
}

int 
DvbParentalRatingDescriptor::getRating(const char* code) 
{
    /*
     * +---------------------+----------------------------------+
     * |        Rating       |      Description                 |
     * |    0x00             |  undefined                       |
     * |    0x01 to 0x0F     |  minimum age = rating + 3 years  |
     * |    0x10 to 0xFF     |  defined by the broadcaster      |
     * +---------------------+----------------------------------+
     */
    std::map<std::string, int>::iterator it = mParentalRatings.find(code);
    if (it != mParentalRatings.end())
        return it->second;
    return 0;
}
