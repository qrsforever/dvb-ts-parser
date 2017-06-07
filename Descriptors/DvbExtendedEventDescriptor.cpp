#include "DvbAssertions.h"
#include "DvbExtendedEventDescriptor.h"
#include "DvbUtils.h"

/* 
 * extended_event_descriptor(){
 *     descriptor_tag                          8
 *     descriptor_length                       8
 *     descriptor_number                       4
 *     last_descriptor_number                  4
 *     ISO_639_language_code                   24
 *     length_of_items                         8
 *     for ( i=0;i<N;i++){
 *         item_description_length             8
 *         for (j=0;j<N;j++){
 *             item_description_char           8
 *         }
 *         item_length                         8
 *         for (j=0;j<N;j++){
 *             item_char                       8
 *         }
 *     }
 *     text_length                             8
 *     for (i=0;i<N;i++){
 *         text_char                           8
 *     }
 * }
 *  */

DvbExtendedEventDescriptor::DvbExtendedEventDescriptor(const DvbDescriptor& descriptor) : DvbDescriptor(descriptor)
{

    /* 8 = (8 + 8 + 4 + 4 + 24 + 8 + 8) / 8
     * descriptor_tag                          8
     * descriptor_length                       8
     * descriptor_number                       4
     * last_descriptor_number                  4
     * ISO_639_language_code                   24
     * length_of_items                         8
     * text_length                             8
     *  */
    if (getLength() < 8) {
        initData();
        return;
    }
    mItemLength = (int)UtilsGetBits(getData(), 0, 48, 8);
    if (mItemLength > (getLength() - 8)) {
        LogDvbWarn("adjust length[%d] vs [%d]\n", mItemLength, (getLength() - 8));
        mItemLength = getLength() - 8;
    }
    mTextLength = (int)UtilsGetBits(getData(), mItemLength, 56, 8);    
    if (mTextLength > (getLength() - 8 - mItemLength)) {
        LogDvbWarn("adjust length[%d] vs [%d]\n", mTextLength, (getLength() - 8 - mItemLength));
        mTextLength = getLength() - 8 - mItemLength;
    }
}

std::string DvbExtendedEventDescriptor::getIso639Lan() const
{
    std::string lan("");
    lan.append(1, (uint8_t)UtilsGetBits(getData(), 0, 24, 8));
    lan.append(1, (uint8_t)UtilsGetBits(getData(), 0, 32, 8));
    lan.append(1, (uint8_t)UtilsGetBits(getData(), 0, 40, 8));
    return lan;
}

std::string DvbExtendedEventDescriptor::getItemDescript() const
{
    //TODO
    return "";
}

std::string 
DvbExtendedEventDescriptor::getExtendedText() const
{
    return UtilsCovertText(getData() + 8 + mItemLength, mTextLength);
}
