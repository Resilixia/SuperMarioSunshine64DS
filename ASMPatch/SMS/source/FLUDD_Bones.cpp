#include "FLUDD.h"

byte FLUDD_BCA_DUMMY_ANIMATION_HEADER[36] = 
{
	// Byte 1: Uses interpolation, Byte 2: Index increments each frame, Byte 3 - 4, u16: Starting index
	0x00, 0x01, 0x00, 0x00, // Sx
	0x00, 0x01, 0x00, 0x00, // Sy
	0x00, 0x01, 0x00, 0x00, // Sz
	0x00, 0x01, 0x00, 0x00, // Rx
	0x00, 0x01, 0x00, 0x00, // Ry
	0x00, 0x01, 0x00, 0x00, // Rz
	0x00, 0x01, 0x00, 0x00, // Tx
	0x00, 0x01, 0x00, 0x00, // Ty
	0x00, 0x01, 0x00, 0x00, // Tz
};

// UPDATING TRANSLATION FOR ANY BONE OTHER THAN SKL_ROOT DOES NOT WORK - 
// REASON UNKNOWN

byte FLUDD_CHARACTER_FIRST_FLUDD_BONE_INDEX = 0x10;

unsigned int FLUDD_CurrentBoneSRTValuesLocation = 0xFFFFFFFF;
unsigned int FLUDD_BCA_CurrentR9ValueAnimatingModel = 0xFFFFFFFF;
byte FLUDD_BCA_CurrentBone = 0xFF;

short int FLUDD_TurboBladesRotation = 0x0000;
short int FLUDD_HoverWaterBoostRotation = 0x0000;

void EnableBone(unsigned int current_bone_srt_values_location);
void DisableBone(unsigned int current_bone_srt_values_location);
void SetBoneRotation(unsigned int current_bone_srt_values_location, short int xRot, short int yRot, short int zRot);

////////////// Model Modification  //////////////

// The address at which the offset to the current bone's (r0) animation header 
// address is calculated
void hook_020453F8()
{
	asm
	(
		"ldr r1, =FLUDD_BCA_CurrentBone		\t\n"
		"strb r0, [r1]						\t\n"
	);
}

// Just after start of method 0204547C, if the current bone is one of the ones 
// we've added for F.L.U.D.D. then point it to use a dummy animation header, 
// the values'll be set at the end
void repl_02045484()
{
	asm
	(
		"ldr r8, =PLAYER								\t\n" // PLAYER object
		"ldr r8, [r8]									\t\n"
		"cmp r8, #0x00									\t\n" // Not in game yet
		"beq original_02045484							\t\n"
		
		"cmp r8, r9										\t\n" // If r9 not pointing to 
		"bne original_02045484							\t\n" // PLAYER object
		
		"mov r8, #0x02000000							\t\n"
		"add r8, #0x00092000							\t\n"
		"add r8, #0x00000128							\t\n" // HAT_CHARACTER 0x02092128
		"ldrb r8, [r8]									\t\n"
		"cmp r8, #0x03									\t\n"
		"beq original_02045484							\t\n"
		
		"ldr r8, =FLUDD_BCA_CurrentBone					\t\n"
		"ldrb r8, [r8]									\t\n"
		"ldr r5, =FLUDD_CHARACTER_FIRST_FLUDD_BONE_INDEX			\t\n"
		"ldrb r5, [r5]									\t\n"
		"cmp r8, r5										\t\n"
		"blt original_02045484							\t\n"
		
		"ldr r1, =FLUDD_BCA_DUMMY_ANIMATION_HEADER		\t\n"
		
		"original_02045484:								\t\n"
		"mov r8, r0										\t\n"
	);
}

// Just before the end of method 0204547C, here we set the S, R and T 
// values to either show or hide various bones
void hook_02045690()
{
	asm
	(
		"ldr r1, =FLUDD_CurrentBoneSRTValuesLocation		\t\n"
		"str r5, [r1]										\t\n"
		"ldr r1, =FLUDD_BCA_CurrentR9ValueAnimatingModel	\t\n"
		"str r9, [r1]										\t\n"
	);
	
	if ((MarioActor*)FLUDD_BCA_CurrentR9ValueAnimatingModel != PLAYER) return;
	
	if (HAT_CHARACTER == CHAR_Yoshi) return;
	
	// If Yoshi wearing a hat, disable F.L.U.D.D.
	if (CHARACTER == CHAR_Yoshi)
	{
		if (FLUDD_BCA_CurrentBone >= FLUDD_CHARACTER_FIRST_FLUDD_BONE_INDEX) 
		{
			DisableBone(FLUDD_CurrentBoneSRTValuesLocation);
		}
		return;
	} 
	
	/*if (FLUDD_BCA_CurrentBone == FLUDD_CHARACTER_FIRST_FLUDD_BONE_INDEX && !FLUDD_Unlocked)
	{
		DisableBone(FLUDD_CurrentBoneSRTValuesLocation);
		return;
	}*/
	
	if (FLUDD_CurrentNozzle != FLUDD_NozzleNone)
	{
		if (FLUDD_IsActive || Player_CurrentAction == ACT_ReachedPeakOfFirstOrSecondJump) 
		{
			if (FLUDD_BCA_CurrentBone == FLUDD_CHARACTER_FACE_BONE_INDEX) 
			{
				// Stop head going too far back when FLUDD being worn
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + 
					FLUDD_BCA_ROTATION_Z_OFFSET)) = 0x4000;
			}
		}
	}
	
	if (FLUDD_CurrentNozzle == FLUDD_NozzleNone)
	{
		if (FLUDD_BCA_CurrentBone >= FLUDD_CHARACTER_FIRST_FLUDD_BONE_INDEX) 
		{ 
			DisableBone(FLUDD_CurrentBoneSRTValuesLocation);
		}
	}
	else if (FLUDD_CurrentNozzle == FLUDD_NozzleHover)
	{
		if (FLUDD_BCA_CurrentBone == FLUDD_CHARACTER_HANDLE_LEFT_BONE_INDEX)
		{
			EnableBone(FLUDD_CurrentBoneSRTValuesLocation);
			
			if (FLUDD_NozzleHoverActive)
			{
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Y_OFFSET)) = 
					0x0000;
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Z_OFFSET)) = 
					0x0000;
			}
			else 
			{
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Y_OFFSET)) = 
					0xF000;
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Z_OFFSET)) = 
					0x2000;
			}
		}
		else if (FLUDD_BCA_CurrentBone == FLUDD_CHARACTER_HANDLE_RIGHT_BONE_INDEX)
		{
			EnableBone(FLUDD_CurrentBoneSRTValuesLocation);
			
			if (FLUDD_NozzleHoverActive)
			{
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Y_OFFSET)) = 
					0x0000;
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Z_OFFSET)) = 
					0x0000;
			}
			else 
			{
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Y_OFFSET)) = 
					0x1000;
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Z_OFFSET)) = 
					0x2000;
			}
		}
		else if (FLUDD_BCA_CurrentBone == FLUDD_MARIO_WATERBOOST_L_BONE_INDEX)
		{
			if (FLUDD_NozzleHoverActive)
			{
				EnableBone(FLUDD_CurrentBoneSRTValuesLocation);
				
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_SCALE_X_OFFSET)) = 
					0x2000;
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_SCALE_Y_OFFSET)) = 
					0x2000;
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_SCALE_Z_OFFSET)) = 
					0x2000;
				
				if (FLUDD_IsActive)
				{
					FLUDD_HoverWaterBoostRotation = 
						(FLUDD_HoverWaterBoostRotation + (FLUDD_HoverDuration * 0xD0)) % 0x10000;
				}
				
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_X_OFFSET)) = 
					FLUDD_HoverWaterBoostRotation;
			}
			else
			{
				DisableBone(FLUDD_CurrentBoneSRTValuesLocation);
			}
		}
		else if (FLUDD_BCA_CurrentBone == FLUDD_MARIO_WATERBOOST_R_BONE_INDEX)
		{
			if (FLUDD_NozzleHoverActive)
			{
				EnableBone(FLUDD_CurrentBoneSRTValuesLocation);
				
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_SCALE_X_OFFSET)) = 
					0x2000;
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_SCALE_Y_OFFSET)) = 
					0x2000;
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_SCALE_Z_OFFSET)) = 
					0x2000;
				
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_X_OFFSET)) = 
					0 - FLUDD_HoverWaterBoostRotation;
			}
			else
			{
				DisableBone(FLUDD_CurrentBoneSRTValuesLocation);
			}
		}
		else if (FLUDD_BCA_CurrentBone == FLUDD_MARIO_WATER_NOZZLE_L_BONE_INDEX)
		{
			EnableBone(FLUDD_CurrentBoneSRTValuesLocation);
			
			if (FLUDD_NozzleHoverActive)
			{
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Z_OFFSET)) = 
					0 - FLUDD_HOVER_TURN_WATER_NOZZLE_ROTATION;
				
				if ( (FLUDD_PlayerLeftPressed && !FLUDD_PlayerDownPressed) || 
					(FLUDD_PlayerRightPressed && FLUDD_PlayerDownPressed) )
				{
					*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Z_OFFSET)) -= 
						FLUDD_HOVER_TURN_WATER_NOZZLE_ROTATION;
				}
				else if ( (FLUDD_PlayerRightPressed && !FLUDD_PlayerDownPressed) || 
					(FLUDD_PlayerLeftPressed && FLUDD_PlayerDownPressed) )
				{
					*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Z_OFFSET)) += 
						FLUDD_HOVER_TURN_WATER_NOZZLE_ROTATION;
				}
			}
		}
		else if (FLUDD_BCA_CurrentBone == FLUDD_MARIO_WATER_NOZZLE_R_BONE_INDEX)
		{
			EnableBone(FLUDD_CurrentBoneSRTValuesLocation);
			
			if (FLUDD_NozzleHoverActive)
			{
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Z_OFFSET)) = 
					0 - FLUDD_HOVER_TURN_WATER_NOZZLE_ROTATION;
				
				if ( (FLUDD_PlayerRightPressed && !FLUDD_PlayerDownPressed) || 
					(FLUDD_PlayerLeftPressed && FLUDD_PlayerDownPressed) )
				{
					*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Z_OFFSET)) -= 
						FLUDD_HOVER_TURN_WATER_NOZZLE_ROTATION;
				}
				else if ( (FLUDD_PlayerLeftPressed && !FLUDD_PlayerDownPressed) || 
					(FLUDD_PlayerRightPressed && FLUDD_PlayerDownPressed) )
				{
					*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Z_OFFSET)) += 
						FLUDD_HOVER_TURN_WATER_NOZZLE_ROTATION;
				}
			}
		}
		else if (FLUDD_BCA_CurrentBone >= FLUDD_CHARACTER_FIRST_FLUDD_BONE_INDEX) 
		{
			EnableBone(FLUDD_CurrentBoneSRTValuesLocation);
		}
	}
	else if (FLUDD_CurrentNozzle == FLUDD_NozzleRocket)
	{
		if (FLUDD_BCA_CurrentBone == FLUDD_CHARACTER_HANDLE_LEFT_BONE_INDEX)
		{
			EnableBone(FLUDD_CurrentBoneSRTValuesLocation);
			
			if (FLUDD_IsActive || (FLUDD_NozzleRocketActive || FLUDD_NozzleRocketJump))
			{
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Y_OFFSET)) = 
					0x0000;
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Z_OFFSET)) = 
					(FLUDD_PlayerDPadPressed) ? 0xF400 : 0xFC00;
			}
			else 
			{
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Y_OFFSET)) = 
					0xF000;
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Z_OFFSET)) = 
					0x2000;
			}
		}
		else if (FLUDD_BCA_CurrentBone == FLUDD_CHARACTER_HANDLE_RIGHT_BONE_INDEX)
		{
			EnableBone(FLUDD_CurrentBoneSRTValuesLocation);
			
			if (FLUDD_IsActive || (FLUDD_NozzleRocketActive || FLUDD_NozzleRocketJump))
			{
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Y_OFFSET)) = 
					0x0000;
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Z_OFFSET)) = 
					(FLUDD_PlayerDPadPressed) ? 0xF400 : 0xFC00;
			}
			else 
			{
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Y_OFFSET)) = 
					0x1000;
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Z_OFFSET)) = 
					0x2000;
			}
		}
		else if (FLUDD_BCA_CurrentBone == FLUDD_LUIGI_WATERBOOST_1_BONE_INDEX)
		{
			if (FLUDD_NozzleRocketActive)
			{
				if (FLUDD_RocketDuration < FLUDD_ROCKET_DURATION_LIMIT_B_KEY)
				{
					EnableBone(FLUDD_CurrentBoneSRTValuesLocation);
					
					*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_SCALE_X_OFFSET)) = 
						0x4000;
					*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_SCALE_Y_OFFSET)) = 
						0x4000;
					*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_SCALE_Z_OFFSET)) = 
						0x4000;
						
					*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_ROTATION_Z_OFFSET)) = 
						0x1555;
				}
				else
				{
					DisableBone(FLUDD_CurrentBoneSRTValuesLocation);
				}
			}
			else
			{
				DisableBone(FLUDD_CurrentBoneSRTValuesLocation);
			}
		}
		else if (FLUDD_BCA_CurrentBone >= FLUDD_CHARACTER_FIRST_FLUDD_BONE_INDEX) 
		{
			EnableBone(FLUDD_CurrentBoneSRTValuesLocation);
		}
	}
	else if (FLUDD_CurrentNozzle == FLUDD_NozzleTurbo)
	{
		if (FLUDD_BCA_CurrentBone == FLUDD_CHARACTER_SKL_ROOT_BONE_INDEX)
		{
			if ((FLUDD_IsActive || FLUDD_NozzleTurboActive))// && !FLUDD_PlayerSwimming)
			{
				SetBoneRotation(FLUDD_CurrentBoneSRTValuesLocation, 0x4000, 0x0000, 0x4000);
			}
		} 
		else if (FLUDD_BCA_CurrentBone == FLUDD_CHARACTER_SPIN_BONE_INDEX)
		{
			if ((FLUDD_IsActive || FLUDD_NozzleTurboActive))
			{
				SetBoneRotation(FLUDD_CurrentBoneSRTValuesLocation, 0x0000, 0x0000, 0x0000);
			}
		}
		else if (FLUDD_BCA_CurrentBone == FLUDD_CHARACTER_ARM_L1_BONE_INDEX)
		{
			if ((FLUDD_IsActive || FLUDD_NozzleTurboActive))
			{
				SetBoneRotation(FLUDD_CurrentBoneSRTValuesLocation, 0xDC74, 0xC000, 0xAAB0);
			}
		}
		else if (FLUDD_BCA_CurrentBone == FLUDD_CHARACTER_ARM_L2_BONE_INDEX)
		{
			if ((FLUDD_IsActive || FLUDD_NozzleTurboActive))
			{
				SetBoneRotation(FLUDD_CurrentBoneSRTValuesLocation, 0x8000, 0x2000, 0x0000);
			}
		}
		else if (FLUDD_BCA_CurrentBone == FLUDD_CHARACTER_WRIST_L1_BONE_INDEX)
		{
			if ((FLUDD_IsActive || FLUDD_NozzleTurboActive))
			{
				SetBoneRotation(FLUDD_CurrentBoneSRTValuesLocation, 0xE390, 0xF1C8, 0x0E38);
			}
		}
		else if (FLUDD_BCA_CurrentBone == FLUDD_CHARACTER_ARM_R1_BONE_INDEX)
		{
			if ((FLUDD_IsActive || FLUDD_NozzleTurboActive))
			{
				SetBoneRotation(FLUDD_CurrentBoneSRTValuesLocation, 0x238C, 0x4000, 0xAAB0);
			}
		}
		else if (FLUDD_BCA_CurrentBone == FLUDD_CHARACTER_ARM_R2_BONE_INDEX)
		{
			if ((FLUDD_IsActive || FLUDD_NozzleTurboActive))
			{
				SetBoneRotation(FLUDD_CurrentBoneSRTValuesLocation, 0x8000, 0xE000, 0x0000);
			}
		}
		else if (FLUDD_BCA_CurrentBone == FLUDD_CHARACTER_WRIST_R1_BONE_INDEX)
		{
			if ((FLUDD_IsActive || FLUDD_NozzleTurboActive))
			{
				SetBoneRotation(FLUDD_CurrentBoneSRTValuesLocation, 0x1C70, 0x0E38, 0x0E38);
			}
		}
		if (FLUDD_BCA_CurrentBone == FLUDD_CHARACTER_HANDLE_LEFT_BONE_INDEX)
		{
			EnableBone(FLUDD_CurrentBoneSRTValuesLocation);
			
			if (FLUDD_IsActive || FLUDD_NozzleTurboActive)
			{
				SetBoneRotation(FLUDD_CurrentBoneSRTValuesLocation, 0x0000, 0x0000, 0x0000);
			}
			else 
			{
				SetBoneRotation(FLUDD_CurrentBoneSRTValuesLocation, 0x0000, 0xF000, 0x2000);
			}
		}
		else if (FLUDD_BCA_CurrentBone == FLUDD_CHARACTER_HANDLE_RIGHT_BONE_INDEX)
		{
			EnableBone(FLUDD_CurrentBoneSRTValuesLocation);
			
			if (FLUDD_IsActive || FLUDD_NozzleTurboActive)
			{
				SetBoneRotation(FLUDD_CurrentBoneSRTValuesLocation, 0x0000, 0x0000, 0x0000);
			}
			else 
			{
				SetBoneRotation(FLUDD_CurrentBoneSRTValuesLocation, 0x0000, 0x1000, 0x2000);
			}
		}
		else if (FLUDD_BCA_CurrentBone == FLUDD_WARIO_WATERBOOST_1_BONE_INDEX)
		{
			if (FLUDD_NozzleTurboActive)
			{
				EnableBone(FLUDD_CurrentBoneSRTValuesLocation);
				
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_SCALE_X_OFFSET)) = 
					0x4000 + (FLUDD_TurboBladesRotation >> 3);
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_SCALE_Y_OFFSET)) = 
					0x4000;
				*((volatile short int*)(FLUDD_CurrentBoneSRTValuesLocation + FLUDD_BCA_SCALE_Z_OFFSET)) = 
					0x4000;

				SetBoneRotation(FLUDD_CurrentBoneSRTValuesLocation, FLUDD_TurboBladesRotation, 0x0000, 0x4000);
			}
			else
			{
				DisableBone(FLUDD_CurrentBoneSRTValuesLocation);
			}
		}
		else if (FLUDD_BCA_CurrentBone == FLUDD_WARIO_TURBO_BLADES_BONE_INDEX)
		{
			EnableBone(FLUDD_CurrentBoneSRTValuesLocation);
			
			if (FLUDD_IsActive)
			{
				FLUDD_TurboBladesRotation = 
					(FLUDD_TurboBladesRotation + (FLUDD_TurboBuildUpCount * 0xD0)) % 0x10000;
			}
			
			SetBoneRotation(FLUDD_CurrentBoneSRTValuesLocation, 0x0000, FLUDD_TurboBladesRotation, 0x0000);
		}
		else if (FLUDD_BCA_CurrentBone >= FLUDD_CHARACTER_FIRST_FLUDD_BONE_INDEX) 
		{
			EnableBone(FLUDD_CurrentBoneSRTValuesLocation);
		}
	}
}

__attribute__((noinline)) void EnableBone(unsigned int current_bone_srt_values_location)
{
	*((volatile int*)(current_bone_srt_values_location + FLUDD_BCA_SCALE_X_OFFSET)) = 0x1000;
	*((volatile int*)(current_bone_srt_values_location + FLUDD_BCA_SCALE_Y_OFFSET)) = 0x1000;
	*((volatile int*)(current_bone_srt_values_location + FLUDD_BCA_SCALE_Z_OFFSET)) = 0x1000;
	
	SetBoneRotation(current_bone_srt_values_location, 0x0000, 0x0000, 0x0000);
	
	*((volatile int*)(current_bone_srt_values_location + FLUDD_BCA_TRANSLATION_X_OFFSET)) = 0x0000;
	*((volatile int*)(current_bone_srt_values_location + FLUDD_BCA_TRANSLATION_Y_OFFSET)) = 0x0000;
	*((volatile int*)(current_bone_srt_values_location + FLUDD_BCA_TRANSLATION_Z_OFFSET)) = 0x0000;
}

__attribute__((noinline)) void DisableBone(unsigned int current_bone_srt_values_location)
{
	*((volatile int*)(current_bone_srt_values_location + FLUDD_BCA_SCALE_X_OFFSET)) = 0x0000;
	*((volatile int*)(current_bone_srt_values_location + FLUDD_BCA_SCALE_Y_OFFSET)) = 0x0000;
	*((volatile int*)(current_bone_srt_values_location + FLUDD_BCA_SCALE_Z_OFFSET)) = 0x0000;
	
	SetBoneRotation(current_bone_srt_values_location, 0x0000, 0x0000, 0x0000);
	
	*((volatile int*)(current_bone_srt_values_location + FLUDD_BCA_TRANSLATION_X_OFFSET)) = 0x0000;
	*((volatile int*)(current_bone_srt_values_location + FLUDD_BCA_TRANSLATION_Y_OFFSET)) = 0x0000;
	*((volatile int*)(current_bone_srt_values_location + FLUDD_BCA_TRANSLATION_Z_OFFSET)) = 0x0000;
}

void SetBoneRotation(unsigned int current_bone_srt_values_location, 
	short int xRot, short int yRot, short int zRot)
{
	*((volatile short int*)(current_bone_srt_values_location + FLUDD_BCA_ROTATION_X_OFFSET)) = xRot;
	*((volatile short int*)(current_bone_srt_values_location + FLUDD_BCA_ROTATION_Y_OFFSET)) = yRot;
	*((volatile short int*)(current_bone_srt_values_location + FLUDD_BCA_ROTATION_Z_OFFSET)) = zRot;
}
