#include "SM64DS.h"

// The following hook is placed within the method for checking objects' draw distance and determining whether it should be drawn. 
// If it's an object we always want drawn, set it to always draw.
// The unique address can be obtained from many locations, 020110EC being one.

void repl_020110E8()
{
	asm
	(
		"mov r7, #0x02100000	\n\t" // BK_BOTAOSI (198): 02 11 44 20
		"add r7, #0x14400		\n\t"
		"add r7, #0x20			\n\t"
		"ldr r8, [r4]			\n\t"
		"cmp r7, r8				\n\t"
		"beq always_draw		\n\t"
		"mov r7, #0x02100000	\n\t" // BOMB_SEESAW (173): 02 13 74 FC
		"add r7, #0x37000		\n\t"
		"add r7, #0x4F0			\n\t"
		"add r7, #0xC			\n\t"
		"cmp r7, r8				\n\t"
		"beq always_draw		\n\t"
		"mov r7, #0x02100000	\n\t" // CT_MECHA07 (093): 02 11 D3 B4
		"add r7, #0x1D000		\n\t"
		"add r7, #0x3B0			\n\t"
		"add r7, #0x4			\n\t"
		"cmp r7, r8				\n\t"
		"beq always_draw		\n\t"
		"mov r7, #0x02100000	\n\t" // CT_MECHA05 (091): 02 11 D2 B4
		"add r7, #0x1D000		\n\t"
		"add r7, #0x2B0			\n\t"
		"add r7, #0x4			\n\t"
		"cmp r7, r8				\n\t"
		"beq always_draw		\n\t"
		"b other_objects		\n\t" // Any other object
		"always_draw:			\n\t"
		"mov r0, #0x0			\n\t"
		"bx r14					\n\t"
		"other_objects:			\n\t"
		"cmp r0, r2				\n\t"
	);
}
