#include "DvbAssertions.h"
#include "DvbSdtParser.h"
#include "DvbSection.h"

/* 
 *             SDT
 *
 * service_description_section(){                       .          
 *    table_id                         8     ----------/_\--------
 *    section_syntax_indicator         1                |          
 *    reserved_future_use              1                |          
 *    reserved                         2                |          
 *    section_length                   12               |          
 *    transport_stream_id              16           SectionHead (8Bytes)       
 *    reserved                         2                |          
 *    version_number                   5                |          
 *    current_next_indicator           1                |          
 *    section_number                   8               _|_         
 *    last_section_number              8     ----------\ /-------- 
 *    original_network_id              16               . 
 *    reserved_future_use              8
 *    for (i=0;i<N;i++){                 <|--------------------------+
 *         service_id                  16                            |
 *         reserved_future_use         6                             |
 *         EIT_schedule_flag           1                             |                                    
 *         EIT_present_following_flag  1           *********         |                                    
 *         running_status              3        ***         ***      |                                    
 *         free_CA_mode                1        *   Element   *      |                                    
 *         descriptors_loop_length     12       ***         ***      |                                    
 *         for (j=0;j<N;j++){                      *********         |
 *             descriptor()                                          |
 *         }                                                         |
 *    }                                  <|--------------------------+
 *    CRC_32                           32
 * }
 *  */

void
DvbSdtElement::_InitSdtElement(const uint8_t* data, int32_t size)
{
    /* 
     *   5 = (16 + 6 + 1 + 1 + 3 + 1 + 12) / 8
     *   service_id                  16 
     *   reserved_future_use         6  
     *   EIT_schedule_flag           1  
     *   EIT_present_following_flag  1  
     *   running_status              3  
     *   free_CA_mode                1  
     *   descriptors_loop_length     12 
     *  */
    if (size < 5) {
        initData();
        return ;
    }
    int32_t elementLen = (int)UtilsGetBits(data, 0, 28, 12) + 5;
    if (elementLen > size) {
        LogDvbWarn("len > size\n");
        elementLen = size;
    }
    initData(data, elementLen, size);
}

DvbDescriptor 
DvbSdtElement::descriptors() const
{
    /* 
     *   5 = (16 + 6 + 1 + 1 + 3 + 1 + 12) / 8
     *   service_id                  16 
     *   reserved_future_use         6  
     *   EIT_schedule_flag           1  
     *   EIT_present_following_flag  1  
     *   running_status              3  
     *   free_CA_mode                1  
     *   descriptors_loop_length     12 
     *  */
    return DvbDescriptor(getData() + 5, getLength() - 5);
}

DvbSdtParser::DvbSdtParser(DvbSectionSDT* section) 
{
    /*   
     *   3 = (8 + 1 + 1 + 2 + 12) / 8
     *   table_id                         8  
     *   section_syntax_indicator         1  
     *   reserved_future_use              1  
     *   reserved                         2  
     *   section_length                   12 
     *  */
    int32_t len1 = section->getSectionLength() + 3;
    if (len1 > section->getSize()) {
        LogDvbWarn("length > section->getSize()\n");
        len1 = section->getSize();
    }
    int32_t len2 = kSectionHeadLength + kSectionCrcLength;
    if (len1 < len2) {
        LogDvbError("data length is invalid\n");
        initData();
        return ;
    }
    initData(section->getData() + kSectionHeadLength, len1 - len2, section->getSize() - len2); 
}

DvbSdtElement 
DvbSdtParser::elements() const
{
    /* 
     *   3 = (16 + 8) / 8
     *   original_network_id  16 
     *   reserved_future_use  8  
     *  */
    return DvbSdtElement(getData() + 3, getLength() - 3);
}
