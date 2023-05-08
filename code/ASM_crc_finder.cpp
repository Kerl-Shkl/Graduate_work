#include <iostream>
#include <cstdint>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

HMODULE hInst;

#define MAKE_PTR(type, base, offset) ((type)((BYTE*)(base) + (SIZE_T)(offset)))

#define TEST_MACRO(out_var) __asm \
{ \
	__asm mov eax, hInst \
	__asm jmp l1	\
	__asm mov out_var, eax \
	__asm jmp end \
} \
l1: \
__asm { \
	__asm add eax, 0x200 \
	__asm mov out_var, eax \
} \
end:


#define GET_CRC(reg_A, reg_B, reg_C, reg_D, reg_SI, reg_DI, out_var) __asm \
{ \
		__asm mov e##reg_A##x, hInst /* base load address in eax ## */\
		__asm mov e##reg_B##x, e##reg_A##x\
		__asm add e##reg_B##x, 60 /* sixty is e_lfanew offset */\
		__asm mov e##reg_B##x, [e##reg_B##x] /* e_lfanew value is in ebx*/\
		__asm add e##reg_A##x, e##reg_B##x /* pe header is in eax */\
		__asm xor e##reg_C##x, e##reg_C##x \
		__asm xor e##reg_SI, e##reg_SI \
		__asm mov reg_C##x, [e##reg_A##x + 6] /* number of sections is in ecx */\
		__asm mov reg_SI, [e##reg_A##x + 20] /* Size of optional header is in esi */\
		__asm add e##reg_A##x, 24 /* Optional pe header is in eax */\
		__asm mov e##reg_B##x, e##reg_A##x \
		__asm add e##reg_B##x, 96 /* DataDirectory is in ebx */\
		__asm add e##reg_B##x, 5 * 8 /* Base relocation table field is in ebx */\
		__asm push[e##reg_B##x] /* Add reloc rva in stack */\
		__asm push[e##reg_B##x + 4] /* Add reloc size in stack */\
		__asm add e##reg_A##x, e##reg_SI /* Start of section table in eax */\
		__asm mov e##reg_SI, 1 \
		__asm mov e##reg_D##x, e##reg_A##x \
}\
sections_loop: \
__asm {\
		__asm mov e##reg_B##x, [e##reg_D##x + 36] /* Characteristics of section is in ebx */\
		__asm and e##reg_B##x, 0x20 /* if ebx is not zero, then the section is being executed */\
		__asm jnz section_found  \
		__asm add e##reg_D##x, 40 /* in edx next section table */\
		__asm inc e##reg_SI /* in esi number off current section */\
		__asm cmp e##reg_SI, e##reg_C##x /* if esi and ecx are equal, then there isn't code section */\
		__asm je no_section \
		__asm jmp sections_loop \
}\
section_found: \
__asm {\
		/* In edx code section table /* */\
		__asm push [e##reg_D##x + 12] /* Add code section rva in stack */\
		__asm push [e##reg_D##x + 8] /* Add code section rva in stack */\
		/*============= Main algorithm =============*/\
		__asm mov e##reg_DI, [esp + 12] /* reloc rva is in edi */\
		__asm add e##reg_DI, hInst /* reloc address is in edi. So edi conatains current reloc block */\
		__asm mov e##reg_SI, e##reg_DI /* esi contains current block */\
		__asm add e##reg_SI, 8 /* esi contains current rf */\
		__asm xor e##reg_C##x, e##reg_C##x /* In ecx will be iterator for main loop */\
		__asm xor e##reg_D##x, e##reg_D##x /* In edx will be checksum */\
}\
main_loop: \
__asm {\
		__asm cmp e##reg_C##x, [esp] /* Check is iterator less than code section size */\
		__asm jge main_loop_end \
}\
find_reloc_loop: \
__asm{ \
		/* Check is current rf in relocation table */\
			__asm mov e##reg_B##x, [esp + 12] /* Reloc section rva in edx */\
			__asm add e##reg_B##x, [esp + 8] /* rva of end address reloc table is in edx */\
			__asm add e##reg_B##x, hInst /* end address reloc table is in edx */\
			__asm cmp e##reg_SI, e##reg_B##x \
			__asm jge end_reloc_loop \
		/* Check equals current rf checked byte */\
			__asm mov reg_A##x, [e##reg_SI] /* Place 2 bytes reloc in eax */\
			__asm and e##reg_A##x, 0xFFF /* Remove type of relocation */\
			__asm add e##reg_A##x, [e##reg_DI] /* Add section rva to value from reloc field. Reloc address is in eax */\
			__asm mov e##reg_B##x, [esp + 4] /* code section rva in ebx */\
			__asm add e##reg_B##x, e##reg_C##x /* Current checked byte rva in ebx */\
			__asm cmp e##reg_A##x, e##reg_B##x /* Compare checked byte and current relocation */\
			__asm jg  checked_is_not_in_reloc \
			__asm je  checked_in_reloc \
		/* Check is current rf in current block */\
			__asm mov e##reg_B##x, e##reg_DI /* reloc block is in edx */\
			__asm add e##reg_B##x, [e##reg_B##x + 4] /* end address of relocation block is in edx */\
			__asm cmp e##reg_SI, e##reg_B##x /* compare current rf with end address of relocation block */\
			__asm jl in_old_block \
			__asm add e##reg_DI, [e##reg_DI + 4] /* Switch to next reloc block */\
			__asm lea e##reg_SI, [e##reg_DI + 6] /* Current rf in new block */\
}\
in_old_block: \
__asm{ \
			__asm add e##reg_SI, 2 /* Next rf */\
			__asm jmp find_reloc_loop \
}\
checked_is_not_in_reloc: \
__asm{ \
			__asm xor e##reg_A##x, e##reg_A##x /* if checked byte isn't in reloc, then eax contains 0 */\
			__asm jmp end_reloc_loop \
}\
checked_in_reloc: \
__asm{ \
			__asm mov e##reg_A##x, 1 /* if checked byte is in reloc, then eax contains 1 */\
			__asm jmp end_reloc_loop /* TODO delete this instruction */\
}\
end_reloc_loop: \
__asm{ \
		__asm test e##reg_A##x, e##reg_A##x  \
		__asm jnz skip_byte \
		__asm mov e##reg_B##x, hInst /* Place hInst in ebx */\
		__asm add e##reg_B##x, [esp + 4] /* Add code section rva to hInst */\
		__asm add e##reg_B##x, e##reg_C##x /* Add iterator in ebx. So ebx contains address of checked byte */\
		__asm xor e##reg_A##x, e##reg_A##x \
		__asm mov reg_A##l, byte ptr [e##reg_B##x] /* In eax low byte contains byte from code section for adding in crc */\
		__asm add e##reg_D##x, e##reg_A##x /* Add value of checked byte to CRC */\
		__asm inc e##reg_C##x /* Increment iterator */\
		__asm jmp main_loop \
}\
skip_byte: \
__asm{ \
		__asm add e##reg_C##x, 4 \
		__asm jmp main_loop \
}\
main_loop_end: \
__asm{ \
		__asm mov out_var, e##reg_D##x \
		__asm add esp, 2*4 \
}\
no_section: \
__asm{ \
		__asm add esp, 2*4 /* clear stack */\
}

void find_crc()
{
	/*
	 * e_lfanew: 60
	 * NumberOfSections: 6 from PE header
	 * SizeOfOptionalHeader: 20 from PE header
	 * OptionalHeader: 24 from PE header
	 * DataDirectory: 96 from Optional PE header
	 * IMAGE_DIRECTORY_ENTRY_BASERELOC: 5. Size of DataDirectory field: 8
	 * IMAGE_SCN_CNT_CODE: 0x20
	 * Section Characteristics: 36 from section field
	*/
	unsigned int test;
	__asm {
		mov eax, hInst; // base load address in eax
		mov ebx, eax;
		add ebx, 60; // sixty is e_lfanew offset;
		mov ebx, [ebx]; // e_lfanew value is in ebx;
		add eax, ebx; // pe header is in eax;
		xor ecx, ecx;
		xor esi, esi;
		mov cx, [eax + 6]; // number of sections is in ecx;
		mov si, [eax + 20]; // Size of optional header is in esi;
		add eax, 24; // Optional pe header is in eax;
		mov ebx, eax;
		add ebx, 96; // DataDirectory is in ebx;
		add ebx, 5 * 8; // Base relocation table field is in ebx;
		push[ebx]; // Add reloc rva in stack;
		push[ebx + 4]; // Add reloc size in stack;
		add eax, esi; // Start of section table in eax;
		mov esi, 1;
		mov edx, eax;
	sections_loop:
		mov ebx, [edx + 36]; // Characteristics of section is in ebx;
		and ebx, 0x20;; // if ebx is not zero, then the section is being executed;
		jnz section_found; //;
		add edx, 40; // in edx next section table;
		inc esi; // in esi number off current section;
		cmp esi, ecx; // if esi and ecx are equal, then there isn't code section;
		je no_section;
		jmp sections_loop;
	section_found:
		/* In edx code section table */
		push [edx + 12]; // Add code section rva in stack
		push [edx + 8]; // Add code section rva in stack
		/*============= Main algorithm =============*/

		mov edi, [esp + 12]; // reloc rva is in edi
		add edi, hInst; // reloc address is in edi.
						// So edi conatains current reloc block
		mov esi, edi; // esi contains current block
		add esi, 8; // esi contains current rf
		xor ecx, ecx; // In ecx will be iterator for main loop
		xor edx, edx; // In edx will be checksum

	main_loop:
		cmp ecx, [esp]; // Check is iterator less than code section size
		jge main_loop_end;

		find_reloc_loop:
		/* Check is current rf in relocation table */
			mov ebx, [esp + 12]; // Reloc section rva in edx
			add ebx, [esp + 8]; // rva of end address reloc table is in edx
			add ebx, hInst; // end address reloc table is in edx
			cmp esi, ebx;
			//jge end_reloc_loop;
            jge checked_is_not_in_reloc;
		
		/* Check equals current rf checked byte */
			mov ax, [esi]; // Place 2 bytes reloc in eax
			and eax, 0xFFF; // Remove type of relocation
			add eax, [edi]; // Add section rva to value from reloc field. Reloc address is in eax
			mov ebx, [esp + 4]; // code section rva in ebx
			add ebx, ecx; // Current checked byte rva in ebx
			cmp eax, ebx; // Compare checked byte and current relocation
			jg  checked_is_not_in_reloc;
			je  checked_in_reloc;

		/* Check is current rf in current block */
			mov ebx, edi; // reloc block is in edx
			add ebx, [ebx + 4]; // end address of relocation block is in edx
			cmp esi, ebx; // compare current rf with end address of relocation block
			jl in_old_block;
			add edi, [edi + 4]; // Switch to next reloc block
			lea esi, [edi + 6]; // Current rf in new block
		in_old_block:
			add esi, 2; // Next rf 
			jmp find_reloc_loop;

		checked_is_not_in_reloc:
			xor eax, eax; // if checked byte isn't in reloc, then eax contains 0
			jmp end_reloc_loop;

		checked_in_reloc:
			mov eax, 1; // if checked byte is in reloc, then eax contains 1
			jmp end_reloc_loop; // TODO delete this instruction
			
		end_reloc_loop:

		test eax, eax; 
		jnz skip_byte;
		mov ebx, hInst; // Place hInst in ebx
		add ebx, [esp + 4]; // Add code section rva to hInst
		add ebx, ecx; // Add iterator in ebx. So ebx contains address of checked byte
		xor eax, eax;
		mov al, byte ptr [ebx]; // In eax low byte contains byte from code section for adding in crc
		add edx, eax; // Add value of checked byte to CRC
		inc ecx; // Increment iterator
		jmp main_loop;

	skip_byte:
		add ecx, 4;
		jmp main_loop;

	main_loop_end:
		mov test, edx;
	
		add esp, 2*4
	no_section:
		add esp, 2*4 // clear stack
	}
	std::cout << std::hex << test << std::endl;
}

int main(void)
{
	hInst = GetModuleHandle(NULL);

	std::cout << "hInst: " << hInst << std::endl;
	PIMAGE_DOS_HEADER pdh = (PIMAGE_DOS_HEADER)hInst;
	PIMAGE_NT_HEADERS pPeHeader = MAKE_PTR(PIMAGE_NT_HEADERS, hInst, pdh->e_lfanew);
	std::cout << "Reloc rva: " << std::hex << pPeHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress <<
		"\nReloc size: " << std::hex << pPeHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size << std::endl;
	PIMAGE_SECTION_HEADER pSections = MAKE_PTR(PIMAGE_SECTION_HEADER, &pPeHeader->OptionalHeader, pPeHeader->FileHeader.SizeOfOptionalHeader);
	std::cout << "========================" << std::endl;

	find_crc();
	uint32_t main_crc = 0;
	GET_CRC(b, c, d, a, si, di, main_crc);
	std::cout << "CRC from main: " << std::hex << main_crc << std::endl;
	return 0;
} 
