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
