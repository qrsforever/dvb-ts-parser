#include "DvbAssertions.h"
#include "DvbTotParser.h"
#include "DvbSection.h"
#include "DvbSection.h"

/* 
 *         TOT
 * time_offset_section(){
 *     table_id                    8
 *     section_syntax_indicator    1
 *     reserved_future_use         1
 *     reserved                    2
 *     section_length              12
 *     UTC_time                    40  
 *     reserved                    4
 *     descriptors_loop_length     12
 *     for(i=0;i<N;i++){
 *         descriptor()        
 *     }
 *     CRC_32                      32
 * }
 *  */
DvbTotParser::DvbTotParser(DvbSectionTOT* section)
{
    int32_t len1 =  section->getSectionLength() + 3; // 3 = (8 + 1 + 1 + 2 + 12) / 8
    if (len1 > section->getSize()) {
        LogDvbWarn("length > section->getSize()\n");
        len1 = section->getSize();
    }
    /*  8 = (8 + 1 + 1 + 2 + 12 + 40) / 8
     *  table_id                    8
     *  section_syntax_indicator    1
     *  reserved_future_use         1
     *  reserved                    2
     *  section_length              12
     *  UTC_time                    40
     *  */
    int32_t len2 = 8 + kSectionCrcLength;
    if (len1 < len2) {
        LogDvbError("data length is invalid\n");
        initData();
        return ;
    }
    initData(section->getData() + 8, len1 - len2 , section->getSize() - len2);           
}

DvbDescriptor
DvbTotParser::descriptors() const
{
    /*  2 = (4 + 12) / 8
     *  reserved                    4
     *  descriptors_loop_length     12
     * */
    return DvbDescriptor(getData() + 2, getLength() - 2);
}
