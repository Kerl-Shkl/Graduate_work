#include <iostream>
#include "CRC_exe.h"
#include "CRC_image.h"

static volatile uint32_t CRC = 0x4C52454B;

int main()
{
	std::string path = "H:\\work\\C++\\CRC_class\\Release\\CRC_class.exe";
	CRC_exe crc_finder;
	std::cout << crc_finder.read_file(path) << std::endl;
	uint32_t crc_exe = crc_finder.get_sections_CRC(IMAGE_SCN_CNT_CODE);
	crc_finder.log_relocs();
	std::cout << "CRC from exe file: \t" << std::hex << crc_exe << std::endl;

#ifdef _DEBUG
	crc_finder.log_relocs();
#endif

#ifndef _DEBUG
	CRC_image im_crc;
	uint32_t crc_im = im_crc.get_sections_CRC(IMAGE_SCN_CNT_CODE);
	std::cout << "CRC from memmory: \t" << std::hex << crc_im << std::endl;
	im_crc.log_relocs();
#endif
	std::cout << "Before replace" << std::endl;
	std::cout << "\tThis exe contains key: " << std::boolalpha << crc_finder.any_key(CRC) << std::endl;
	crc_finder.replace_all_key(CRC, 0x59425542);
	crc_finder.write_file("replaced_file.exe");
	std::cout << "After replace" << std::endl;
	std::cout << "\tThis exe contains key: " << std::boolalpha << crc_finder.any_key(CRC) << std::endl;
	std::cout << "in global var CRC: " << std::string((char*)(&CRC), 4) << std::endl;
}
