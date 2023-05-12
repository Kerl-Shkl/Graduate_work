#include "CRC_image.h"

CRC_image::CRC_image() :
	CRC_general()
{
	get_inst();
#if defined(_DEBUG) || defined(DEBUG_PRINT_BYTESSUM)
	_log.open("log_image.txt", std::ios::trunc);
#endif
}

CRC_image::~CRC_image()
{
#if defined(_DEBUG) || defined(DEBUG_PRINT_BYTESSUM)
	_log.close();
#endif
}

int32_t CRC_image::sec_start(const IMAGE_SECTION_HEADER& sec) const
{
	return sec.VirtualAddress;
}

void CRC_image::get_inst()
{
	_inst = reinterpret_cast<uint8_t*>(GetModuleHandle(NULL));
}
