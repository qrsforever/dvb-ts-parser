#include "DvbAssertions.h"
#include "DvbNitParser.h"
#include "DvbSection.h"

/* 
 *          NIT
 *
 * network_information_section(){                           .
 *     table_id                           8        --------/_\------------
 *     section_syntax_indicator           1                 |
 *     reserved_future_use                1                 |
 *     reserved                           2                 |
 *     section_length                     12                |
 *     network_id                         16           SectionHead
 *     reserved                           2                 |
 *     version_number                     5                 |
 *     current_next_indicator             1                 |
 *     section_number                     8                _|_
 *     last_section_number                8        --------\-/------------
 *     reserved_future_use                4                 '
 *     network_descriptors_length         12   
 *     for (i=0; i<N; i++) {                   <|------------------------+
 *         descriptor()                                 Descriptor       |
 *     }                                       <|------------------------+
 *     reserved_future_use                4      
 *     transport_stream_loop_length       12                                     
 *     for(i=0;i<N;i++){                      <|-------------------------+                          
 *         transport_stream_id            16                             |                          
 *         original_network_id            16           *********         |                          
 *         reserved_future_use            4         ***         ***      |                                   
 *         transport_descriptors_length   12        *  Elements   *      |                                   
 *         for(j=0;j<N;j++){                        ***         ***      |                                   
 *             descriptor()                            *********         |                                   
 *         }                                                             |                                   
 *     }                                      <|-------------------------+                                   
 *     CRC_32                             32                                                                 
 * }                                                                                                         
 * 
 *  */

void 
DvbNitElement::_InitNitElement(const uint8_t* data, int32_t size)
{
    /*  6 = (16 + 16 + 4 + 12) / 8
     *  transport_stream_id            16 
     *  original_network_id            16 
     *  reserved_future_use            4  
     *  transport_descriptors_length   12 
     *  */
    if (size < 6) {
        initData();
        return ;
    }
    int32_t elementLen = (int32_t)UtilsGetBits(data, 0, 36, 12) + 6;
    if (elementLen > size) {
        LogDvbWarn("len > size\n");
        elementLen = size;
    }
    initData(data, elementLen, size);
}

DvbDescriptor
DvbNitElement::descriptors()
{
    /*  6 = (16 + 16 + 4 + 12) / 8
     *  transport_stream_id            16 
     *  original_network_id            16 
     *  reserved_future_use            4  
     *  transport_descriptors_length   12 
     *  */
    return DvbDescriptor(getData() + 6, getLength() - 6);
}
                                                                           
DvbNitParser::DvbNitParser(DvbSectionNIT* section)
{
     /*   
     *   3 = (8 + 1 + 1 + 2 + 12) / 8
     *   table_id                         8  
     *   section_syntax_indicator         1  
     *   reserved_future_use              1  
     *   reserved                         2  
     *   section_length                   12 
     *  */
    int32_t len1 =  section->getSectionLength() + 3;
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
    initData(section->getData() + kSectionHeadLength, len1 - len2 , section->getSize() - len2);         

    /*  20 = 4 + 12 + 4
     *  reserved_future_use                4                 '
     *  network_descriptors_length         12   
     *  for (i=0; i<N; i++) {                   <|------------------------+
     *      descriptor()                                 Descriptor       |
     *  }                                       <|------------------------+
     *  reserved_future_use                4      
     *  transport_stream_loop_length       12                                     
     * */
    mDescriptorLength = (uint16_t)UtilsGetBits(getData(), 0, 4, 12);
    mElementsLength   = (uint16_t)UtilsGetBits(getData(), mDescriptorLength, 20, 12);
}

DvbDescriptor
DvbNitParser::descriptors() const
{
    /*  2 = (4 + 12) / 8                                                                                                                                                                                          
     *  reserved_future_use                4                                                                                                                                                       
     *  network_descriptors_length         12                                                                                                                                                      
     *  */
    return DvbDescriptor(getData() + 2, mDescriptorLength);
}

DvbNitElement 
DvbNitParser::elements() const
{
    /*  4 = (4 + 12 + 4 + 12) / 8                                                                                                                                                                                          
     *  reserved_future_use                4                                                                                                                                                       
     *  network_descriptors_length         12                                                                                                                                                      
     *  reserved_future_use                4      
     *  transport_stream_loop_length       12                                     
     *  */
    return DvbNitElement(getData() + 4 + mDescriptorLength, mElementsLength);
}
