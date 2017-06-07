#include "DvbAssertions.h"
#include "DvbServiceDescriptor.h"
#include "DvbUtils.h"

/* 
 * service_descriptor(){
 *     descriptor_tag                   8             
 *     descriptor_length                8
 *     service_type                     8
 *     service_provider_name_length     8
 *     for (i=0;i<N;I++){          
 *         Char                         8 
 *     }                       
 *     service_name_length              8                 
 *     for (i=0;i<N;I++){ 
 *         Char                         8
 *     }
 * }
 *  */

DvbServiceDescriptor::DvbServiceDescriptor(const DvbDescriptor& descriptor) : DvbDescriptor(descriptor)
{
    /* 5 = (8 + 8 + 8 + 8 + 8) / 8
     * descriptor_tag                   8
     * descriptor_length                8
     * service_type                     8
     * service_provider_name_length     8
     * service_name_length              8 
     *  */
    if (getLength() < 5) {
       initData();
       return;
    }
    mProviderNameLength = (int)UtilsGetBits(getData(), 0, 24, 8);
    if (mProviderNameLength > (getLength() - 5)) {
        LogDvbWarn("adjust length[%d] vs [%d]\n", mProviderNameLength, (getLength() - 5));
        mProviderNameLength = getLength() - 5;
    }
    mServiceNameLength = (int)UtilsGetBits(getData(), mProviderNameLength, 32, 8);
    if (mServiceNameLength > (getLength() - 5 - mProviderNameLength)) {
        LogDvbWarn("adjust length[%d] vs [%d]\n", mServiceNameLength, (getLength() - 5 - mProviderNameLength));
        mServiceNameLength = getLength() - 5 - mProviderNameLength;
    }
}

int 
DvbServiceDescriptor::getServiceType() const
{
    return (uint16_t)UtilsGetBits(getData(), 0, 16, 8);
}

std::string 
DvbServiceDescriptor::getProviderName() const
{
    return UtilsCovertText(getData() + 4, mProviderNameLength);
}

std::string 
DvbServiceDescriptor::getServiceName() const
{
    return UtilsCovertText(getData() + 5 + mProviderNameLength, mServiceNameLength);
}
