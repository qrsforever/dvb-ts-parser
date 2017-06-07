#include "DvbSatelliteDescriptor.h"
#include "DvbAssertions.h"
#include "DvbUtils.h"

/* 
 * satellite_delivery_system_descriptor(){
 *     descriptor_tag                      8
 *     descriptor_length                   8
 *     frequency                           32
 *     orbital_position                    16
 *     west_east_flag                      1
 *     polarization                        2
 *     If (modulation_system == "1") {
 *         roll off                        2
 *     } else {
 *         "00"                            2
 *     }
 *     modulation_system                   1
 *     modulation_type                     2
 *     symbol_rate                         28
 *     FEC_inner                           4
 * }
 * 
 *  */

DvbSatelliteDescriptor::DvbSatelliteDescriptor(const DvbDescriptor& descriptor) : DvbDescriptor(descriptor)
{
    //13 = (8 + 8 + 32 + 16 + 1 + 2 + 2 + 1 + 2 + 28 + 4) / 8
    if (getLength() < 13) {
       initData();
       return;
    }
}

uint32_t 
DvbSatelliteDescriptor::getFrequency() const
{
    /* 
     *  frequency: The frequency is a 32-bit field giving the 4-bit BCD values specifying 8 characters of the frequency value.
     *  For the satellite_delivery_system_descriptor the frequency is coded in GHz, where the decimal point occurs after the
     *  third character (e.g. 011,75725 GHz).
     * */
    uint32_t frequency = (uint32_t)UtilsGetBits(getData(), 0, 16, 32);
    return ((frequency & 0x0000000f) +
        ((frequency & 0x000000f0) >>  4) * 10 +
        ((frequency & 0x00000f00) >>  8) * 100 +
        ((frequency & 0x0000f000) >> 12) * 1000 +
        ((frequency & 0x000f0000) >> 16) * 10000 +
        ((frequency & 0x00f00000) >> 20) * 100000 + 
        ((frequency & 0x0f000000) >> 24) * 1000000 +
        ((frequency & 0xf0000000) >> 28) * 10000000) * 10; //KHz
}

uint16_t 
DvbSatelliteDescriptor::getOrbitalPostion() const
{
    /* 
     *  orbital_position: The orbital_position is a 16-bit field giving the 4-bit BCD values specifying 4 characters of the
     *  orbital position in degrees where the decimal point occurs after the third character (e.g. 019,2°).
     * */
    uint16_t postion = (uint16_t)UtilsGetBits(getData(), 0, 48, 16);
    return ((postion & 0x000f) + 
        ((postion & 0x00f0) >>  4) * 10 + 
        ((postion & 0x0f00) >>  8) * 100 +
        ((postion & 0xf000) >> 12) * 1000);
}

uint8_t 
DvbSatelliteDescriptor::getWestEastFlag() const 
{
    /* 
     *  west_east_flag: The west_east_flag is a 1-bit field indicating if the satellite position is in the western or eastern part of
     *  the orbit. A value "0" indicates the western position and a value "1" indicates the eastern position.
     *  */
    return (uint8_t)UtilsGetBits(getData(), 0, 64, 1);
}

uint8_t 
DvbSatelliteDescriptor::getPolarization() const 
{
    /* 
     *  polarization: The polarization is a 2-bit field specifying the polarization of the transmitted signal. The first bit defines
     *  whether the polarization is linear or circular (see table 37).
     *                                    +-----------------------------------------+                        
     *                                    |   Polarization  |    Description        |                        
     *                                    |       00        | linear - horizontal   |                        
     *                                    |       01        | linear - vertical     |                        
     *                                    |       10        | Circular - left       |                        
     *                                    |       11        | Circular - right      |                        
     *                                    +-----------------------------------------+                        
     * */                                                                                                   
    return (uint8_t)UtilsGetBits(getData(), 0, 65, 2);
}                                                                                                            

uint8_t 
DvbSatelliteDescriptor::getModulationType() const 
{
    /* 
     *  +--------------------------------------------+
     *  | modulation type       Description          |
     *  |     00                   Auto              |
     *  |     01                   QPSK              |
     *  |     10                   8PSK              |
     *  |     11             16-QAM (n/a for DVB-S2) |
     *  +--------------------------------------------+
     *  */
    return (uint8_t)UtilsGetBits(getData(), 0, 70, 2);
}

uint32_t
DvbSatelliteDescriptor::getSymbolRate() const 
{
    /*                                                                                                                                                                                                
     *  symbol_rate: The symbol_rate is a 28-bit field giving the 4-bit BCD values specifying 7 characters of the symbol_rate                                                                          
     *  in Msymbol/s where the decimal point occurs after the third character (e.g. 027,4500).                                                                                                         
     * */
    uint32_t symbolrate = (uint32_t)UtilsGetBits(getData(), 0, 72, 28);
    return ((symbolrate & 0x0000000f) + 
        ((symbolrate & 0x000000f0) >>  4) * 10 +
        ((symbolrate & 0x00000f00) >>  8) * 100 +
        ((symbolrate & 0x0000f000) >> 12) * 1000 + 
        ((symbolrate & 0x000f0000) >> 16) * 10000 +
        ((symbolrate & 0x00f00000) >> 20) * 100000 +
        ((symbolrate & 0x0f000000) >> 24) * 1000000) / 10; //Ksymbol/s
}
