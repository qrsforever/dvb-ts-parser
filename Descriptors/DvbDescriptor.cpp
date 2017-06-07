#include "DvbAssertions.h"
#include "DvbDescriptor.h"
#include "DvbUtils.h"

void
DvbDescriptor::_InitDescriptor(const uint8_t* data, int32_t size)
{
    if (size < 2) {
        initData();
        return;
    }
	int32_t length = (int32_t)UtilsGetBits(data, 0, 8, 8) + 2;
	if (length > size) {
        LogDvbWarn("length > size\n");
        length = size;
	}
	initData(data, length, size);
}

int 
DvbDescriptor::tag() const 
{
    return (int)UtilsGetBits(getData(), 0, 0, 8);
}
