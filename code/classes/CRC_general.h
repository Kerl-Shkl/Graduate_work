#pragma once
#include <string>
#include <vector>
#include <list>
#include <set>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <iomanip>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define MAKE_PTR(type, base, offset) ((type)((BYTE*)(base) + (SIZE_T)(offset)))

#define DEBUG_PRINT_BYTESSUM
#define DEBUG_PRINT_RELOCS

class CRC_general
{
public:

	CRC_general();
	virtual ~CRC_general();
	uint32_t get_sections_CRC(uint32_t mask);
#if defined(_DEBUG) || defined(DEBUG_PRINT_RELOCS)
	void log_relocs();
#endif

protected:
//public:
	void find_reloc(); 
	void collect_sections(uint32_t mask);
	void collect_relocs();
	bool is_in_relocs(uint32_t rva);
	uint32_t sec_CRC(const IMAGE_SECTION_HEADER& sec);

	PIMAGE_NT_HEADERS get_pe_header();
	virtual int32_t sec_start(const IMAGE_SECTION_HEADER& sec) const = 0;
	virtual void get_inst() = 0;

	std::set<uint32_t> _relocs;
	uint8_t* _inst;
	std::list<IMAGE_SECTION_HEADER> _sec_headers;
	IMAGE_SECTION_HEADER _reloc_section;

#if defined(_DEBUG) || defined(DEBUG_PRINT_BYTESSUM)
	std::ofstream _log;
#endif 
};

