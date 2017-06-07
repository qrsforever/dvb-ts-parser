#include "DvbAssertions.h"
#include "DvbPatParser.h"
#include "DvbSection.h"

/*                                                       
 *             PAT
 *                                                      .
 * program_association_section(){                      /_\
 *     table_id                          8  ------------+------------
 *     section_syntax_indicator          1              |                         
 *     reserved_future_use               1              |            
 *     reserved                          2              |            
 *     section_length                    12             |             
 *     transport_stream_id               16         SectionHead                 
 *     reserved                          2              |            
 *     version_number                    5              |            
 *     current_next_indicator            1              |                    
 *     section_number                    8             _|_           
 *     last_section_number               8    ---------\ /-----------      
 *                                                      .
 *     for(i=0;i<N;i++){                    <|----------------------+    
 *         program_number                16                         |    
 *         reserved                      3            *******       |                                                                   
 *         if(program_number == 0){                ***       ***    |                                                                   
 *             network_PID               13        *  Elements *    |                                                                   
 *         else{                                   ***       ***    |                                                                   
 *             program_map_PID           13           *******       |                                                                   
 *         }                                                        |                                
 *     }                                    <|----------------------+                                
 *     CRC_32                            32                                                          
 * }                                                                                          
 *  */                                                               

void
DvbPatElement::_InitPatElement(const uint8_t* data, int32_t size)
{
    /* 
     *   4 = (16 + 3 + 13) / 8
     *   program_number             16 
     *   reserved                   3  
     *   program_map_PID            13 
     *  */
    if (size < 4) {
        initData();
        return ;
    }
    initData(data, 4, size);
}

DvbPatParser::DvbPatParser(DvbSectionPAT* section)
{
     /*  3 = (8 + 1 + 1 + 2 + 12) / 8
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
}

DvbPatElement 
DvbPatParser::elements() const
{
    return DvbPatElement(getData(), getLength());
}
