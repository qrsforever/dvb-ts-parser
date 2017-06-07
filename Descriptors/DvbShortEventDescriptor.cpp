#include "DvbAssertions.h"
#include "DvbShortEventDescriptor.h"
#include "DvbUtils.h"

/* 
 * short_event_descriptor(){
 *     descriptor_tag                          8
 *     descriptor_length                       8
 *     ISO_639_language_code                   24
 *     event_name_length                       8
 *     for (i=0;i<event_name_length;i++) {
 *         event_name_char                     8
 *     }
 *     text_length                             8
 *     for (i=0;i<text_length;i++){
 *         text_char                           8
 *     }
 * }
 *  */

DvbShortEventDescriptor::DvbShortEventDescriptor(const DvbDescriptor& descriptor) : DvbDescriptor(descriptor)
{
    /* 
     * 7 = (8 + 8 + 24 + 8 + 8) / 8
     * descriptor_tag              8
     * descriptor_length           8
     * ISO_639_language_code       24
     * event_name_length           8
     * text_length                 8
     *  */
    if (getLength() < 7) {
        initData();
        return;
    }
    mNameLength = (int)UtilsGetBits(getData(), 0, 40, 8);
    if (mNameLength > (getLength() - 7)) {
        LogDvbDebug("adjust length[%d] vs [%d]\n", mNameLength, (getLength() - 7));
        mNameLength = getLength() - 7;
    } 
    mTextLength = (int)UtilsGetBits(getData(), mNameLength, 48, 8);
    if (mTextLength > (getLength() - 7 - mNameLength)) {
        LogDvbDebug("adjust length[%d] vs [ %d]\n", mTextLength, (getLength() - 7 - mNameLength));
        mTextLength = getLength() - 7 - mNameLength;
    }
}

std::string DvbShortEventDescriptor::getIso639Lan() const
{
    std::string lan("");
    lan.append(1, (uint8_t)UtilsGetBits(getData(), 0, 16, 8));
    lan.append(1, (uint8_t)UtilsGetBits(getData(), 0, 24, 8));
    lan.append(1, (uint8_t)UtilsGetBits(getData(), 0, 32, 8));
    return lan;
}

std::string
DvbShortEventDescriptor::getEventName() const
{
    /* 
     * 6 = (8 + 8 + 24 + 8) / 8 
     * descriptor_tag              8
     * descriptor_length           8
     * ISO_639_language_code       24
     * event_name_length           8
     *  */
    return UtilsCovertText(getData() + 6, mNameLength);
}

std::string 
DvbShortEventDescriptor::getShortText() const
{
    return UtilsCovertText(getData() + 7 + mNameLength, mTextLength);
}
