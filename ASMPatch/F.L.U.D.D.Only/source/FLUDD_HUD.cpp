/*
	Copyright 2014-2017 Fiachra
*/

#include "FLUDD.h"

// Drawing Water Meter:
byte FLUDD_Digits[3] = { 0xFF, 0xFF, 0xFF };

byte FLUDD_MeterXPos = 240;
byte FLUDD_MeterYPos = 170;

OAMSettings * OAM_CounterChar_x = (OAMSettings*)0x020AB9C8;

OAMSettings OAM_FLUDD_NOZZLE_HOSE_PART_01 = 
{
	0x00,
	0x40, 
	0xF0, 
	0x81, 
	0x0C, // Tile ID: 12
	0xD1, // Bits 5 - 8 - Palette Row: 4 (9 is 0)
	0xFF, 
	0xFF 
};

void DrawWaterMeter();

////////////// Drawing Water Meter //////////////

void hook_020FC828_ov_02()
{
	DrawWaterMeter();
}

__attribute__((noinline)) void DrawWaterMeter()
{
	asm
	(
		"push {r0-r3, r14}				\t\n"
		
		"mov r6, #0x02000000			\t\n"
		"add r6, #0x00092000			\t\n"
		"add r6, #0x00000128			\t\n" // HAT_CHARACTER 0x02092128
		"ldrb r6, [r6]					\t\n"
		"cmp r6, #0x03					\t\n" // Yoshi
		"beq exit						\t\n"
		
		"ldr r6, =FLUDD_WaterLevel		\t\n"
		"ldrb r6, [r6]					\t\n"
		"ldr r5, =OAM_ThinNumbers		\t\n"
		"ldr r7, =FLUDD_Digits			\t\n"
		"ldr r11, =FLUDD_MeterXPos		\t\n"
		"ldrb r11, [r11]				\t\n"
		"ldr r4, =FLUDD_MeterYPos		\t\n"
		"ldrb r4, [r4]					\t\n"
		
		"hundreds:						\t\n" // If W.L. >= 100
		"mov r8, #0x00					\t\n" // Num. 100's = 0
		"cmp r6, #0x64					\t\n" // if (WaterLevel < 100)
		"blt end_hundreds_loop			\t\n" // set digits[0] = r8 (0)
		"sub_100:						\t\n"
		"sub r6, #0x64					\t\n" // W.L. -= 100
		"add r8, #0x01					\t\n" // Num. 100's ++
		"cmp r6, #0x64					\t\n" 
		"bge sub_100					\t\n" // if (W.L. >= 100), b sub_100
		"end_hundreds_loop:				\t\n"
		"strb r8, [r7]					\t\n" // else, digits[0] = Num. 100's
		"b tens							\t\n"
		
		"tens:							\t\n" // If W.L. >= 10
		"mov r8, #0x00					\t\n" // Num. 10's = 0
		"cmp r6, #0x0A					\t\n"
		"blt end_tens_loop				\t\n"
		"sub_10:						\t\n"
		"sub r6, #0x0A					\t\n" // W.L. -= 10
		"add r8, #0x01					\t\n" // Num. 10's ++
		"cmp r6, #0x0A					\t\n"
		"bge sub_10						\t\n" // if (W.L. >= 10), b sub_10
		"end_tens_loop:					\t\n"
		"strb r8, [r7, #0x01]			\t\n" // else, digits[1] = Num. 10's
		"b singles						\t\n"
		
		"singles:						\t\n"
		"strb r6, [r7, #0x02]			\t\n" // digits[2] = W.L.
		
		"mov r8, #0x02					\t\n" // i = 2
		"digits_draw_loop:				\t\n"
		"cmp r8, #0x00					\t\n" // if (i < 0), 
		"blt draw_x						\t\n" // exit loop
		"ldrb r6, [r7, r8]				\t\n" // r6 = digits[i]
		"cmp r6, #0xFF					\t\n" // if (r6 == 0xFF)
		"beq continue_digits_draw_loop	\t\n"
		"mvn r0, #0x00					\t\n"
		"str r0, [r13]					\t\n"
		"mov r0, #0x01					\t\n"
		"str r0, [r13, #0x04]			\t\n"
		"mov r0, #0x00					\t\n"
		"str r0, [r13, #0x08]			\t\n"
		"mov r0, #0x00					\t\n" // bottom_screen: false
		"mov r9, #0x04					\t\n"
		"mul r1, r6, r9					\t\n" 
		"ldr r1, [r5, r1]				\t\n" // OAMSettings* : OAM_ThinNumbers[digits[i]]
		"mov r2, r11					\t\n"
		"mov r3, r4						\t\n"
		"bl DrawOAM						\t\n"
		"sub r11, #0x09					\t\n" // X Pos -= 9
		"continue_digits_draw_loop: 	\t\n"
		"sub r8, #0x01					\t\n"
		"b digits_draw_loop				\t\n"
		
		"draw_x:						\t\n"
		"mvn r0, #0x00					\t\n"
		"str r0, [r13]					\t\n"
		"mov r0, #0x01					\t\n"
		"str r0, [r13, #0x04]			\t\n"
		"mov r0, #0x00					\t\n"
		"str r0, [r13, #0x08]			\t\n"
		"mov r0, #0x00					\t\n"
		"ldr r1, =OAM_CounterChar_x		\t\n"
		"ldr r1, [r1]					\t\n"
		"mov r2, r11					\t\n" 
		"mov r3, r4						\t\n"
		"add r3, #0x08					\t\n"
		"bl DrawOAM						\t\n"
		"sub r11, #0x16					\t\n"
		"b draw_nozzle_hose				\t\n"
		
		"draw_nozzle_hose:							\t\n"
		"mvn r0, #0x00								\t\n"
		"str r0, [r13]								\t\n"
		"mov r0, #0x01								\t\n"
		"str r0, [r13, #0x04]						\t\n"
		"mov r0, #0x00								\t\n"
		"str r0, [r13, #0x08]						\t\n"
		"ldr r1, =OAM_FLUDD_NOZZLE_HOSE_PART_01		\t\n"
		"mov r2, r11								\t\n"
		"mov r3, r4									\t\n"
		"sub r3, #0x02								\t\n"
		"bl DrawOAM									\t\n"
		"b exit										\t\n"
		
		"exit:							\t\n"
		"pop {r0-r3, r14}				\t\n"
		"bx r14							\t\n"
	);
}
