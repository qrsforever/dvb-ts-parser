#ifndef __DvbTotParser__H_
#define __DvbTotParser__H_

#include "DvbDataParser.h" 
#include "DvbDescriptor.h"
#include "DvbUtils.h"

class DvbSectionTOT;
class DvbTotParser : public DvbDataParser {
public: 
    DvbTotParser(DvbSectionTOT* section);
    ~DvbTotParser() {
    } 
    DvbDescriptor descriptors() const;
};

#endif
