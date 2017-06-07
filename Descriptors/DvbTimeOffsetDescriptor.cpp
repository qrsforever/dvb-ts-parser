#include "DvbAssertions.h"
#include "DvbTimeOffsetDescriptor.h"
#include "DvbUtils.h"

/* 
 * local_time_offset_descriptor(){
 *     descriptor_tag                  8
 *     descriptor_length               8
 *     for(i=0;i<N;i++){
 *         country_code                24
 *         country_region_id           6
 *         reserved                    1
 *         local_time_offset_polarity  1
 *         local_time_offset           16
 *         time_of_change              40
 *         next_time_offset            16
 *     }
 * }
 *  */

DvbTimeOffsetDescriptor::DvbTimeOffsetDescriptor(const DvbDescriptor& descriptor) : DvbDescriptor(descriptor)
{
    /*
     *  descriptor_tag              8
     *  descriptor_length           8
     *  country_code                24
     *  country_region_id           6
     *  reserved                    1
     *  local_time_offset_polarity  1
     *  local_time_offset           16
     *  time_of_change              40
     *  next_time_offset            16
     * */
    if (getLength() < 15) { // 15 is the length of the local_time_offset_descriptor at least
       initData();
       return;
    }
    uint32_t year = 0, mon = 0, day = 0, hour = 0 , min = 0, sec = 0;
    uint32_t offsetBytes = 0;
    uint32_t count = (getLength() - 2) / 13;
    for (std::size_t i = 0; i < count; ++i) {
        offsetBytes = i * 13;
        std::string countryCode("");
        countryCode.append(1, (uint8_t)UtilsGetBits(getData(), offsetBytes, 16, 8));
        countryCode.append(1, (uint8_t)UtilsGetBits(getData(), offsetBytes, 24, 8));
        countryCode.append(1, (uint8_t)UtilsGetBits(getData(), offsetBytes, 32, 8));

        TimeOffsetInfo_t timeOffset;
        timeOffset.nCountryRegionId = (uint8_t)UtilsGetBits(getData(), offsetBytes, 40, 6);
        /* 
         *  Polarity : 
         *          If this bit is set to "0" the polarity is positive and the local time is ahead of UTC. 
         *          If this bit is set to "1" the polarity is negative and the local time is behind UTC
         *  */
        timeOffset.nOffsetPolarity  = (uint8_t)UtilsGetBits(getData(), offsetBytes, 47, 1);

        timeOffset.nTimeOffset = (uint16_t)UtilsGetBits(getData(), offsetBytes, 48, 16);
        UtilsCovertTimeMJD((uint32_t)UtilsGetBits(getData(), offsetBytes, 64, 16), &year, &mon, &day);
        UtilsCovertTimeBCD((uint32_t)UtilsGetBits(getData(), offsetBytes, 80, 24), &hour, &min, &sec);
        timeOffset.nTimeChange = UtilsMakeTime(year, mon, day, hour, min, sec);
        timeOffset.nNextOffset = UtilsGetBits(getData(), offsetBytes, 104, 16);

        mTimeOffsets[countryCode] = timeOffset;
    }
}

int 
DvbTimeOffsetDescriptor::getOffsetPolar(const char* code) const
{
    std::map<std::string, TimeOffsetInfo_t>::const_iterator it = mTimeOffsets.find(code);
    if (it == mTimeOffsets.end())
        return 0;
    return it->second.nOffsetPolarity;
}

int 
DvbTimeOffsetDescriptor::getTimeOffset(const char* code) const
{
    std::map<std::string, TimeOffsetInfo_t>::const_iterator it = mTimeOffsets.find(code);
    if (it == mTimeOffsets.end())
        return 0;

    /* These 16-bits are coded as 4-digits in 4-bit BCD in the order hour tens, hour, minute tens and minutes */
    uint32_t hour = 0, minute = 0, second = 0;
    UtilsCovertTimeBCD((uint32_t)(it->second.nTimeOffset << 8), &hour, &minute, &second);
    if (it->second.nOffsetPolarity)
        return -1 * (hour * 3600 + minute * 60);
    return hour * 3600 + minute * 60;
}

int 
DvbTimeOffsetDescriptor::getNextOffset(const char* code) const
{
    std::map<std::string, TimeOffsetInfo_t>::const_iterator it = mTimeOffsets.find(code);
    if (it == mTimeOffsets.end())
        return 0;

    /* These 16-bits are coded as 4-digits in 4-bit BCD in the order hour tens, hour, minute tens and minutes */
    uint32_t hour = 0, minute = 0, second = 0;
    UtilsCovertTimeBCD((uint32_t)(it->second.nNextOffset << 8), &hour, &minute, &second);
    if (it->second.nOffsetPolarity)
        return -1 * (hour * 3600 + minute * 60);
    return hour * 3600 + minute * 60;
}
 
uint64_t DvbTimeOffsetDescriptor::getTimeChange(const char* code) const
{
    std::map<std::string, TimeOffsetInfo_t>::const_iterator it = mTimeOffsets.find(code);
    if (it == mTimeOffsets.end())
        return 0;
    return it->second.nTimeChange;
}
