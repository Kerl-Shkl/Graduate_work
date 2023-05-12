#include "CRC_general.h"

CRC_general::CRC_general() :
	_relocs(),
	_inst(nullptr),
	_sec_headers()
{
}

CRC_general::~CRC_general()
{
}

void CRC_general::find_reloc() 
{
	PIMAGE_NT_HEADERS pPeHeader = get_pe_header();
	if (pPeHeader == nullptr)
		return;
	uint32_t rvaFromDirectorys = pPeHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
	if (rvaFromDirectorys == 0)
		return;
	DWORD dwSectionsCount = pPeHeader->FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER pSections = MAKE_PTR(PIMAGE_SECTION_HEADER, pPeHeader, sizeof(IMAGE_NT_HEADERS));
	for (INT i = 0; i < dwSectionsCount; ++i) {
		if (pSections->VirtualAddress == rvaFromDirectorys)
		{
			_reloc_section = *pSections;
			return;
		}
		++pSections;
	}
}

void CRC_general::collect_sections(uint32_t mask)
{
	if (!_sec_headers.empty()) {
		_sec_headers.clear();
	}
	PIMAGE_NT_HEADERS pPeHeader = get_pe_header();
	if (pPeHeader == nullptr)
		return;
	DWORD sections_count = pPeHeader->FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER pSections = MAKE_PTR(PIMAGE_SECTION_HEADER, pPeHeader, sizeof(IMAGE_NT_HEADERS));
	for (int i = 0; i < sections_count; ++i) {
		if (pSections->Characteristics & mask) {
			_sec_headers.push_back(*pSections);
		}
		++pSections;
	}
}

void CRC_general::collect_relocs()
{
	_relocs.clear();
	PIMAGE_BASE_RELOCATION table = MAKE_PTR(PIMAGE_BASE_RELOCATION, _inst, sec_start(_reloc_section));
	auto tables_end = [&]() { return table >= MAKE_PTR(PIMAGE_BASE_RELOCATION, _inst, sec_start(_reloc_section)+_reloc_section.Misc.VirtualSize); };
	const uint32_t preamble_size = 8;
	while (!tables_end()) {
		for (uint32_t i = preamble_size; i < table->SizeOfBlock; i += 2) {
			uint32_t cell = *(MAKE_PTR(uint32_t*, table, i)) & 0xFFF;
			if (!(i != preamble_size && cell == 0)) {
				_relocs.insert(table->VirtualAddress + cell);
			}
		}
		table = MAKE_PTR(PIMAGE_BASE_RELOCATION, table, table->SizeOfBlock);
	}
}

bool CRC_general::is_in_relocs(uint32_t rva)
{
	return _relocs.find(rva) != _relocs.end();
}

uint32_t CRC_general::get_sections_CRC(uint32_t mask)
{
	collect_sections(mask);
	if (_sec_headers.empty()) {
		return 0;
	}
	find_reloc();
	collect_relocs();

	uint32_t result = 0;
	for (const auto& sec : _sec_headers) {
		result += sec_CRC(sec);
	}

	return result;
}

uint32_t CRC_general::sec_CRC(const IMAGE_SECTION_HEADER& sec)
{
#if defined(DEBUG_PRINT_BYTESSUM)
	_log << "Section: " << sec.Name << std::endl;
#endif
	uint32_t result = 0;
	uint8_t* pb;
	uint32_t i = 0;
	while (i < sec.Misc.VirtualSize) {
		pb = MAKE_PTR(uint8_t*, _inst, sec_start(sec) + i);
		if (is_in_relocs(sec.VirtualAddress + i)) {
			i += 4;
			continue;
		}
		else {
#if defined(DEBUG_PRINT_BYTESSUM)	
			_log << "0x" << std::setfill('0') << std::setw(8) << std::hex << (sec.VirtualAddress + i) << ": " 
				 << "0x" << std::setfill('0') << std::setw(2) << std::hex << int(*pb) << std::endl;
#endif
			result += *pb;
		}
		++i;
	}
	return result;
}

#if defined(_DEBUG) || defined(DEBUG_PRINT_RELOCS)
void CRC_general::log_relocs()
{
	_log << "Relocs:" << std::endl;
	for (auto it = _relocs.begin(); it != _relocs.end(); ++it) {
		_log << "0x" << std::setw(8) << std::setfill('0') << std::hex << (*it) << std::endl;
	}
}
#endif

PIMAGE_NT_HEADERS CRC_general::get_pe_header() 
{
	get_inst();
	PIMAGE_DOS_HEADER pDosHeader = MAKE_PTR(PIMAGE_DOS_HEADER, _inst, 0);
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
		return nullptr;
	}
	PIMAGE_NT_HEADERS pPeHeader = MAKE_PTR(PIMAGE_NT_HEADERS, _inst, pDosHeader->e_lfanew);
	if (pPeHeader->Signature != IMAGE_NT_SIGNATURE) {
		return nullptr;
	}
	return pPeHeader;
}


