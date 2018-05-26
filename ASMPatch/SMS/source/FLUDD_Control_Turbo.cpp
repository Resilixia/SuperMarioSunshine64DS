#include "FLUDD.h"

//////////////////////////////////////////////////////////////////////////////
// Turbo Nozzle-specific behaviour changes									//
//////////////////////////////////////////////////////////////////////////////

// Following hook placed just after 020A0E58 updated with key press 
// state at 0203BD00. If Turbo Nozzle active, release all DPad keys 
// press 'Up'. Also disable pressing 'R'.
void hook_0203BCA0()
{
	if (HAT_CHARACTER == CHAR_Yoshi) return;
	
	if (NozzleTurboBuildingUpOrInUse())
	{
		if (!FLUDD_PlayerSwimming)
		{
			// No DPad button pressed
			*((volatile byte*)FLUDD_KeysAddress) &= 0x0F;  
			// Up button pressed
			*((volatile byte*)FLUDD_KeysAddress) |= 0x40;
		}
		
		*((volatile short int*)FLUDD_KeysAddress) &= 0xFEFF;
	}
}

// If Turbo Nozzle active, increase the running speed (this method called 
// when player moving character) to get the maximum speed
void repl_020D3C64_ov_02()
{
	asm
	(
		"ldr r5, =FLUDD_NozzleTurboActive	\t\n"
		"ldrb r5, [r5]						\t\n"
		"cmp r5, #0x01						\t\n"
		"addeq r0, #0x026000				\t\n"
		
		"mov r5, r0							\t\n" // Original instruction
	);
}

// If Turbo Nozzle active, increase the rate of Mario accelerating from 
// standing still to 8x faster
/*void repl_020D40E0_ov_02()
{
	asm
	(
		"ldr r2, =FLUDD_NozzleTurboActive	\t\n"
		"ldrb r2, [r2]						\t\n"
		"cmp r2, #0x01						\t\n"
		"moveq r2, #0x8000					\t\n"
		"movne r2, #0x1000					\t\n"
	);
}*/

// When Turbo nozzle in use run forward in Mario's current y rotation 
// direction, ignoring the camera angle
void repl_020E50D8_ov_02() 
{
	asm
	(
		"push {r1}										\t\n"
		"ldr r1, =FLUDD_NozzleTurboBuildingUpOrInUse	\t\n"
		"ldrb r1, [r1]									\t\n"
		"cmp r1, #0x01									\t\n"
		"pop {r1}										\t\n"
		"bne original_020E50D8							\t\n"
		"ldr r0, [r5, #0x8E]							\t\n"
		"b exit_020E50D8								\t\n"
		
		"original_020E50D8: 							\t\n"
		"add r0, r1										\t\n"
		
		"exit_020E50D8: 								\t\n"
	);
}

// Don't lean back upon hitting 'down' key whilst in water if using 
// Turbo nozzle. 1/2
void hook_020CD640_ov_02() 
{
	asm
	(
		"ldr r0, =FLUDD_NozzleTurboBuildingUpOrInUse	\t\n"
		"ldrb r0, [r0]									\t\n"
		"cmp r0, #0x01									\t\n"
		"popeq {r0-r12}									\t\n"
		"moveq r3, #0x00								\t\n"
		"pusheq {r0-r12}								\t\n"
	);
}

// Don't lean back upon hitting 'down' key whilst in water if using 
// Turbo nozzle. 2/2
void hook_020CD65C_ov_02() 
{
	asm
	(
		"ldr r0, =FLUDD_NozzleTurboBuildingUpOrInUse	\t\n"
		"ldrb r0, [r0]									\t\n"
		"cmp r0, #0x01									\t\n"
		"popeq {r0-r12}									\t\n"
		"moveq r0, #0x00								\t\n"
		"pusheq {r0-r12}								\t\n"
	);
}

// If crashing into wall that would otherwise be hugged, don't set 
// the forward speed to zero overriding the negative value that's 
// already been assigned to make player bounce back. 
// Turbo wall crash 1/3
void repl_020CF8E4_ov_02() 
{
	asm
	(
		"ldr r1, =FLUDD_NozzleTurboWallCrash	\t\n"
		"ldrb r1, [r1]							\t\n"
		"cmp r1, #0x01							\t\n"
		"strne r0, [r6, #0x98]					\t\n"
	);
}

// If crashing into wall using Turbo nozzle, skip the original logic 
// that would perform action and set forward speed to zero.
// Turbo wall crash 2/3
unsigned int FLUDD_SKIP_WALL_HUG_JUMP_ADDRESS = 0x020CFADC;
void repl_020CFA98_ov_02() 
{
	asm
	(
		"ldr r4, =FLUDD_NozzleTurboWallCrash		\t\n"
		"ldrb r4, [r4]								\t\n"
		"cmp r4, #0x01								\t\n"
		"bne original_020CFA98						\t\n"
		"ldr r4, =FLUDD_SKIP_WALL_HUG_JUMP_ADDRESS	\t\n"
		"ldr r15, [r4]								\t\n"
		
		"original_020CFA98: 						\t\n"
		"mov r4, r0									\t\n"
	);
}

// If crashing into wall from the side whilst using Turbo nozzle, 
// don't increase the forwardDirection by 0x4000.
// Turbo wall crash 3/3
void repl_020CF96C_ov_02() 
{
	asm
	(
		"ldr r7, =FLUDD_NozzleTurboWallCrash	\t\n"
		"ldrb r7, [r7]							\t\n"
		"cmp r7, #0x01							\t\n"
		"bne original_020CF96C					\t\n"
		"cmp r4, r6								\t\n"
		"b exit_020CF96C						\t\n"
		
		"original_020CF96C: 					\t\n"
		"cmp r4, r0								\t\n"
		
		"exit_020CF96C: 						\t\n"
	);
}

// If Turbo Nozzle active, increase the vertical speed with which Mario 
// jumps and if Rocket Nozzle active, increase it even further. Also 
// increase for Hover jump.
void hook_020E245C_ov_02()
{
	asm
	(
		"ldr r0, =FLUDD_NozzleTurboActive	\t\n"
		"ldrb r0, [r0]						\t\n"
		"cmp r0, #0x01						\t\n"
		"beq turbo_jump_vspeed				\t\n"
		
		"ldr r0, =FLUDD_NozzleRocketActive	\t\n"
		"ldrb r0, [r0]						\t\n"
		"cmp r0, #0x01						\t\n"
		"beq rocket_jump_vspeed				\t\n"
		
		"ldr r0, =FLUDD_NozzleHoverActive	\t\n"
		"ldrb r0, [r0]						\t\n"
		"cmp r0, #0x01						\t\n"
		"bne original_020E245C				\t\n"
		
		"hover_jump_vspeed:					\t\n"
		"mov r0, #0x20000					\t\n"
		"b original_020E245C				\t\n"
		
		"rocket_jump_vspeed:				\t\n"
		"mov r0, #0xA0000					\t\n"
		"b original_020E245C				\t\n"
		
		"turbo_jump_vspeed:					\t\n"
		"mov r0, #0x46000					\t\n"
		
		"original_020E245C:					\t\n"
		"streq r0, [r4, #0xA8]				\t\n"
	);
}

// If Turbo Nozzle active, don't reduce the forward speed when jump 
// first pressed
void repl_020E2550_ov_02()
{
	asm
	(
		"ldr r0, =FLUDD_NozzleTurboActive	\t\n"
		"ldrb r0, [r0]						\t\n"
		"cmp r0, #0x01						\t\n"
		"strne r1, [r4, #0x98]				\t\n"
	);
}

// If Turbo Nozzle active, don't gradually reduce the forward speed to 
// 0x01DB26 whilst in the air
void repl_020E2A70_ov_02()
{
	asm
	(
		"ldr r1, =FLUDD_NozzleTurboActive	\t\n"
		"ldrb r1, [r1]						\t\n"
		"cmp r1, #0x01						\t\n"
		"moveq r1, #0x48000					\t\n"
		"movne r1, r4						\t\n" // Original instruction
	);
}

// Below address updates Mario's speed when swimming, when using Turbo nozzle 
// we don't want this update to occur
void repl_020CD250_ov_02()
{
	asm
	(
		"ldr r5, =FLUDD_NozzleTurboActive		\t\n"
		"ldrb r5, [r5]							\t\n"
		"cmp r5, #0x01							\t\n"
		"strne r6, [r2]							\t\n"
	);
}

// Below address sets the speed for the Player_PerformAction call for act 0xA6, 
// (kicking in water)
/*void repl_020CDF00_ov_02()
{
	asm
	(
		"ldr r3, =FLUDD_NozzleTurboActive		\t\n"
		"ldrb r3, [r3]							\t\n"
		"cmp r3, #0x01							\t\n"
		"moveq r3, #0x2000						\t\n"
		"movne r3, #0x1000						\t\n"
	);
}*/

// Below code originally checks if Player has started to descend from 
// jump out of water and if so, slows their forward speed down to 
// 0x12000, want to remove this
void repl_020E13FC_ov_02() { }

// Increase the vertical speed of Mario jumping out of water with the 
// Turbo nozzle
void repl_020E166C_ov_02()
{
	asm
	(
		"ldr r0, =FLUDD_NozzleTurboActive		\t\n"
		"ldrb r0, [r0]							\t\n"
		"cmp r0, #0x01							\t\n"
		"moveq r0, #0x50000						\t\n" // Turbo speed
		"movne r0, #0x2A000						\t\n" // Default speed
	);
}

// Increase the forward speed of Mario jumping out of water with the 
// Turbo nozzle
void repl_020E1674_ov_02()
{
	asm
	(
		"ldr r2, =FLUDD_NozzleTurboActive		\t\n"
		"ldrb r2, [r2]							\t\n"
		"cmp r2, #0x01							\t\n"
		"moveq r2, #0x50000						\t\n"
		"movne r2, #0x12000						\t\n"
	);
}

// Change the act performed when Mario jumps out of water from 1st jump 
// (0x53) to second jump 0x4F
void repl_020E1658_ov_02()
{
	asm
	(
		"ldr r1, =FLUDD_NozzleTurboActive		\t\n"
		"ldrb r1, [r1]							\t\n"
		"cmp r1, #0x01							\t\n"
		"moveq r1, #0x4F						\t\n"
		"movne r1, #0x53						\t\n"
	);
}

// Just before the check that determines whether a side-flip will be 
// allowed, if Turbo nozzle is active we don't want to allow side-flips 
// or skidding to a halt so force that part of code to be skipped.
void repl_020D3EA8_ov_02()
{
	asm
	(
		"ldr r1, =FLUDD_NozzleTurboActive	\t\n"
		"ldrb r1, [r1]						\t\n"
		"cmp r1, #0x01						\t\n"
		"moveq r0, #0x00					\t\n"
		
		// Original instruction
		"cmp r0, #0x6000					\t\n"		
	);
}
