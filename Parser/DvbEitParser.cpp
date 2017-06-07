#include "DvbAssertions.h"
#include "DvbEitParser.h"
#include "DvbSection.h"

/* 
 *          EIT
 *
 * network_information_section(){
 *     table_id                         8 <|----------------------------+
 *     section_syntax_indicator         1                               |
 *     reserved_future_use              1                               |                                             
 *     reserved                         2             *********         |                                                        
 *     section_length                   12        ****         ****     |                                                        
 *     service_id                       16        *  SectionHead  *     |                                                        
 *     reserved                         2         ****         ****     |                                                        
 *     version_number                   5             *********         |                                                        
 *     current_next_indicator           1                               |                                                        
 *     section_number                   8                               |                                                        
 *     last_section_number              8 <|----------------------------+                            
 *     transport_stream_id              16  
 *     original_network_id              16  
 *     segment_last_section_number      8  
 *     last_table_id                    8  
 *     for(i=0;i<N;i++){                  <|----------------------------+---->getData()
 *         event_id                     16                              |
 *         start_time                   40                              |                                  
 *         duration                     24           *********          |                                  
 *         running_status               3        ****         ****      |                                  
 *         free_CA_mode                 1        *    Element    *      |                                             
 *         descriptors_loop_length      12       ****         ****      |                                             
 *         for(j=0;j<N;j++){                         *********          |                                             
 *             descriptor()                                             |                                             
 *         }                                                            |                                             
 *     }                                  <|----------------------------+                                             
 *     CRC_32                           32                                                                            
 * }                                                                                                                  
 * 
 *  */

void 
DvbEitElement::_InitEitElement(const uint8_t* data, int32_t size)
{
    /*     
     * 12 = (16 + 40 + 24 + 3 + 1 + 12) / 8
     * event_id                     16  
     * start_time                   40  
     * duration                     24  
     * running_status               3   
     * free_CA_mode                 1   
     * descriptors_loop_length      12  
     *  */

    if (size < 12) {
        initData();
        return ;
    }
    int32_t elementLen = (int32_t)UtilsGetBits(data, 0, 84, 12) + 12;
    if (elementLen > size) {
        LogDvbWarn("len > size\n");
        elementLen = size;
    }
    initData(data, elementLen, size);
}

uint64_t 
DvbEitElement::getStartTime() const
{
    uint32_t year = 0, mon = 0, day = 0, hour = 0 , min = 0, sec = 0;
    UtilsCovertTimeMJD((uint32_t)UtilsGetBits(getData(), 0, 16, 16), &year, &mon, &day);
    UtilsCovertTimeBCD((uint32_t)UtilsGetBits(getData(), 0, 32, 24), &hour, &min, &sec);
    return UtilsMakeTime(year, mon, day, hour, min, sec);
}

uint64_t 
DvbEitElement::getDuration() const
{
    uint32_t hour = 0, min = 0, sec = 0;
    UtilsCovertTimeBCD((uint32_t)UtilsGetBits(getData(), 0, 56, 24), &hour, &min, &sec);
    return UtilsMakeTime(0, 0, 0, hour, min, sec);
}

DvbDescriptor 
DvbEitElement::descriptors() const
{
    /*     
     * 12 = (16 + 40 + 24 + 3 + 1 + 12) / 8
     * event_id                     16  
     * start_time                   40  
     * duration                     24  
     * running_status               3   
     * free_CA_mode                 1   
     * descriptors_loop_length      12  
     *  */
    return DvbDescriptor(getData() + 12, getLength() - 12);
}

DvbEitParser::DvbEitParser(DvbSectionEIT* section) 
{
     /*   
     *   3 = (8 + 1 + 1 + 2 + 12) / 8
     *   table_id                         8  
     *   section_syntax_indicator         1  
     *   reserved_future_use              1  
     *   reserved                         2  
     *   section_length                   12 
     *  */
    int len1 =  section->getSectionLength() + 3;
    if (len1 > section->getSize()) {
        LogDvbWarn("length > section->getSize()\n");
        len1 = section->getSize();
    }
    int len2 = kSectionHeadLength + kSectionCrcLength;
    if (len1 < len2) {
        LogDvbError("data length is invalid\n");
        initData();
        return ;
    }
    initData(section->getData() + kSectionHeadLength, len1 - len2 , section->getSize() - len2);   
}

DvbEitElement 
DvbEitParser::elements() const
{
    /* 
     * 6 = (16 + 16 + 8 + 8) / 8
     * transport_stream_id              16  
     * original_network_id              16  
     * segment_last_section_number      8  
     * last_table_id                    8  
     *  */
    return DvbEitElement(getData() + 6, getLength() - 6);
}
