#include "DvbAssertions.h"
#include "DvbLinkageDescriptor.h"
#include "DvbUtils.h"

/* 
 * linkage_descriptor() {
 *     descriptor_tag           8
 *     descriptor_length        8
 *     transport_stream_id      16
 *     original_network_id      16
 *     service_id               16
 *     linkage_type             8
 *     for (i=0;i<N;i++){
 *         private_data_byte    8
 *     }
 * }
 *  */

DvbLinkageDescriptor::DvbLinkageDescriptor(const DvbDescriptor& descriptor) : DvbDescriptor(descriptor)
{
    /* 9 = (8 + 8 + 16 + 16 + 16 + 8) / 8
     * descriptor_tag           8
     * descriptor_length        8
     * transport_stream_id      16
     * original_network_id      16
     * service_id               16
     * linkage_type             8
     *  */
    if (getLength() < 9) {
       initData();
       return;
    }
}

uint16_t 
DvbLinkageDescriptor::getTransportStreamId() const
{
    return (uint16_t)UtilsGetBits(getData(), 0, 16, 16);
}

uint16_t 
DvbLinkageDescriptor::getOriginalNetworkId() const
{
    return (uint16_t)UtilsGetBits(getData(), 0, 32, 16);
}

uint16_t 
DvbLinkageDescriptor::getServiceId() const
{
    return (uint16_t)UtilsGetBits(getData(), 0, 48, 16);
}

uint8_t 
DvbLinkageDescriptor::getLinkageType() const
{
    return (uint8_t)UtilsGetBits(getData(), 0, 64, 8);
}

const uint8_t* 
DvbLinkageDescriptor::getLinkageData() const
{
    return getData() + 9;
}

uint32_t 
DvbLinkageDescriptor::getLinkageLength() const
{
    return (uint32_t)(getLength() - 9);
}
