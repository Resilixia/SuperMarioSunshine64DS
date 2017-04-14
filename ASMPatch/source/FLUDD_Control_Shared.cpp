/*
	Copyright 2014-2017 Fiachra
*/

#include "FLUDD.h"

//////////////////////////////////////////////////////////////////////////////
// Shared Control Modifications 											//
//////////////////////////////////////////////////////////////////////////////

// 020D3C24 reads value at 0209F49C and ANDS 0x800, making subsequent branch 
// always true makes Mario always run with Y having no effect
void repl_020D3C2C_ov_02()
{
	asm
	(
		"ands r0, r0, #0x800	\t\n" // Original instruction
		"cmp r0, r14			\t\n" // Force false for subsequent branch
	);
}

// Based on Skelux's patch to reduce speed at which the player can side-flip, 
// decreases the speed at which the player can perform a side-flip to very 
// low
void repl_020D3EB8_ov_02()
{
	asm
	(
		"mov r1, #0xC000					\t\n"
	);
}

// Based on Skelux's Swap Run and Walk 1/4: Run by default
// IMPORTANT: Any code after this executes will see Y pressed and Y released 
// inverted
void repl_0202C78C()
{
	asm
	(
		"ldrh r0, [r9, #0x04]			\t\n" // Get Keys status
		"mov r1, r0						\t\n"
		"ands r1, #0x800				\t\n" // Check 'Y' pressed
		"moveq r7, #0x00				\t\n"
		"movne r7, #0x01				\t\n"
		"ldr r1, =FLUDD_IsActive		\t\n"
		"strb r7, [r1]					\t\n"
		"y_key_inversion:				\t\n" // Invert whether the 
		"eors r0, r0, #0x800			\t\n" // 'Y' key pressed 
		"strh r0, [r9, #0x04]			\t\n"
	);
}

// Based on Skelux's Swap Run and Walk 2/4: Disable run-up
void repl_020D3040_ov_02()
{
	asm
	(
		"ands r1, r1, #0x800		\t\n" // Original instruction
		
		"cmp r1, r1					\t\n" // Force subsequent branch at 020D3044
	);
}

// Based on Skelux's Swap Run and Walk 3/4: Fix swimming
void repl_020CDA18_ov_02()
{
	asm
	(
		"ldr r1, =FLUDD_IsActive			\t\n"
		"ldrb r1, [r1]						\t\n"
		"cmp r1, #0x01						\t\n"
		"bne no_swim_on_y					\t\n"
		"ldr r1, =FLUDD_TurboBuildUpCount	\t\n"
		"ldrb r1, [r1]						\t\n"
		"cmp r1, #0x00						\t\n"
		"ble no_swim_on_y					\t\n"
		"mov r1, #0x800						\t\n"
		"add r1, #0x02						\t\n"
		"orr r4, #0x800						\t\n"
		"b exit_020CDA18					\t\n"
		"no_swim_on_y:						\t\n"
		"mov r1, #0x02						\t\n"
		"exit_020CDA18:						\t\n"
	);
}