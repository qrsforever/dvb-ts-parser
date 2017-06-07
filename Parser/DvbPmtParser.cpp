#include "DvbAssertions.h"
#include "DvbPmtParser.h"
#include "DvbSection.h"

/* 
 *      PMT 
 * program_map_section(){                            .
 *     table_id                    8        --------/_\------------
 *     section_syntax_indicator    1                 |
 *     reserved_future_use         1                 |
 *     reserved                    2                 |
 *     section_length              12                |
 *     network_id                  16           SectionHead
 *     reserved                    2                 |
 *     version_number              5                 |
 *     current_next_indicator      1                 |
 *     section_number              8                _|_
 *     last_section_number         8        --------\-/------------            
 *     reserved                    3                 .  
 *     PCR_PID                     13                          
 *     reserved                    4                           
 *     program_info_length         12                          
 *     for(i=0;i<N;i++){                    <|------------------------+                    
 *         descriptor()                              Descriptor       |                    
 *     }                                    <|------------------------+                    
 *     for(i=0;i<N;i++){                                       
 *         stream_type             8                           
 *         reserved                3                           
 *         elementary_PID          13                          
 *         reserved                4                           
 *         ES_info_length          12                          
 *         for(j=0;j<N;j++){                                   
 *             descriptor()                                    
 *         }                                                   
 *     }                                                                                 
 *     CRC_32                      32                                                     
 * }
 *  */

void 
DvbPmtElement::_InitPmtElement(const uint8_t* data, int32_t size)
{
    /*  5 = (8 + 3 + 13 + 4 + 12) / 8
     *  stream_type             8                           
     *  reserved                3                           
     *  elementary_PID          13                          
     *  reserved                4                           
     *  ES_info_length          12                          
     * */
    if (size < 5) {
        initData();
        return ;
    }
    int elementLen = (int)UtilsGetBits(data, 0, 28, 12) + 5; 
	if (elementLen > size) {
        LogDvbWarn("len > size\n");
		elementLen = size;
	}
	initData(data, elementLen, size);
}

uint8_t 
DvbPmtElement::getStreamType() const
{
    return (uint8_t)UtilsGetBits(getData(), 0, 0, 8);
}

uint16_t 
DvbPmtElement::getElementaryPid() const
{
    return (uint16_t)UtilsGetBits(getData(), 0, 11, 13);
}

DvbDescriptor 
DvbPmtElement::descriptors() const
{
    /*  5 = (8 + 3 + 13 + 4 + 12) / 8
     *  stream_type             8                           
     *  reserved                3                           
     *  elementary_PID          13                          
     *  reserved                4                           
     *  ES_info_length          12                          
     * */
    return DvbDescriptor(getData() + 5, getLength() - 5);
}

DvbPmtParser::DvbPmtParser(DvbSectionPMT* section) 
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

    /*
     *  20 = 3 + 13 + 4
     *  reserved        3   
     *  PCR_PID         13                          
     *  reserved        4                           
     * */
    mDescriptorLength = (uint16_t)UtilsGetBits(getData(), 0, 20, 12);
}

uint16_t 
DvbPmtParser::getPcrPid() const
{
    return (uint16_t)UtilsGetBits(getData(), 0, 3, 13);
}

DvbDescriptor
DvbPmtParser::descriptors() const
{
    /* 4 = (3 + 13 + 4 + 12) / 8
     * reserved                    3                 .  
     * PCR_PID                     13                          
     * reserved                    4                           
     * program_info_length         12                          
     */
    return DvbDescriptor(getData() + 4, mDescriptorLength);
}

DvbPmtElement 
DvbPmtParser::elements() const
{
    return DvbPmtElement(getData() + 4 + mDescriptorLength, getLength() - (4 + mDescriptorLength));
}
