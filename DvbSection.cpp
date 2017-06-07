#include "DvbAssertions.h"
#include "DvbSection.h"
#include "DvbUtils.h"

DvbSection::DvbSection(const uint8_t* data, uint16_t size) : mSourceData(data), mSourceSize(size), mSectionLength(0), mTableId(0xEF)
{
    if (mSourceSize < 3)
        return;
    _DecodeHeader();
}

DvbSection::~DvbSection()
{
}

bool 
DvbSection::_CheckCrc()
{
    return true;
}

void
DvbSection::_DecodeHeader()
{
    mTableId           = mSourceData[0];
    mSectionLength     = (uint16_t)UtilsGetBits(mSourceData, 0, 12, 12);
    if (mSourceSize > 8) {
        mUnion.mDataId     = (uint16_t)UtilsGetBits(mSourceData, 0, 24, 16);
        mVersionNumber     = (uint8_t)UtilsGetBits(mSourceData, 0, 42, 5);
        mSectionNumber     = (uint8_t)UtilsGetBits(mSourceData, 0, 48, 8);
        mLastSectionNumber = (uint8_t)UtilsGetBits(mSourceData, 0, 56, 8);
    }
}

uint64_t 
DvbSectionTDT::getUtcTime() const 
{
    /* 
     * time_date_section(){
     *     table_id 8
     *     section_syntax_indicator 1
     *     reserved_future_use 1
     *     reserved 2
     *     section_length 12
     *     UTC_time 40
     * }
     *
     * UTC_time: This 40-bit field contains the current time and date in UTC and MJD (see annex C). This field is coded as
     *              16 bits giving the 16 LSBs of MJD followed by 24 bits coded as 6 digits in 4-bit BCD.
     *  */
    uint32_t year = 0, mon = 0, day = 0, hour = 0 , min = 0, sec = 0;
    UtilsCovertTimeMJD((uint32_t)UtilsGetBits(getData(), 0, 24, 16), &year, &mon, &day);
    UtilsCovertTimeBCD((uint32_t)UtilsGetBits(getData(), 0, 40, 24), &hour, &min, &sec);
    return UtilsMakeTime(year, mon, day, hour, min, sec);
}

uint64_t 
DvbSectionTOT::getUtcTime() const 
{
    uint32_t year = 0, mon = 0, day = 0, hour = 0 , min = 0, sec = 0;
    UtilsCovertTimeMJD((uint32_t)UtilsGetBits(getData(), 0, 24, 16), &year, &mon, &day);
    UtilsCovertTimeBCD((uint32_t)UtilsGetBits(getData(), 0, 40, 24), &hour, &min, &sec);
    return UtilsMakeTime(year, mon, day, hour, min, sec);
}
