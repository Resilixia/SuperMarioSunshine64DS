/* Modify BK_TOWER so that a different BMD and KCL is loaded when the 
 * current Star ID is 6. Used in Gelato Beach for the final Star.
 * 
 * The following files are replaced with the second BMD and KCL:
 * 		1816: data/stage/desert_py/desert_py_01_isc.bin
 * 		1817: data/stage/desert_py/desert_py_02_isc.bin
 */

short int const OBJ_203_SECOND_BMD_INTERNAL_ID = 1816; // 0x718
short int const OBJ_203_SECOND_KCL_INTERNAL_ID = 1817; // 0x719

// If the Star ID is not 6, load the original BMD otherwise use the new one.
void repl_0214D758_ov_66()
{
	asm
	(
		"mov r1, #0x02000000		\t\n"
		"add r1, #0x9F000			\t\n"
		"add r1, #0x220				\t\n"
		"ldrb r1, [r1]				\t\n"
		"cmp r1, #0x6				\t\n"
		"beq second_bmd_203			\t\n"
		"original_bmd_203:			\t\n"
		"ldr r1, =0x59D				\t\n"
		"bx r14						\t\n"
		"second_bmd_203:			\t\n"
		"mov r1, #0x710				\t\n"
		"add r1, #0x8				\t\n"
	);
}

// If the Star ID is not 6, load the original KCL otherwise use the new one.
void repl_0214D800_ov_66()
{
	asm
	(
		"mov r1, #0x02000000		\t\n"
		"add r1, #0x9F000			\t\n"
		"add r1, #0x220				\t\n"
		"ldrb r1, [r1]				\t\n"
		"cmp r1, #0x6				\t\n"
		"beq second_kcl_203			\t\n"
		"original_kcl_203:			\t\n"
		"ldr r1, =0x59E				\t\n"
		"bx r14						\t\n"
		"second_kcl_203:			\t\n"
		"mov r1, #0x710				\t\n"
		"add r1, #0x9				\t\n"
	);
}
