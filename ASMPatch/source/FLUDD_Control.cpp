/*
	Copyright 2014-2017 Fiachra
*/

#include "FLUDD.h"
#include "Buttons.h"

byte FLUDD_WaterLevel = 100;
byte FLUDD_WaterLevelDecrementUnitCounter = 0;
byte FLUDD_WaterLevelIncrementUnitCounter = 0;

FLUDD_Nozzle FLUDD_CurrentNozzle = FLUDD_NozzleNone;

bool FLUDD_IsActive = false;

byte FLUDD_SelectButtonHeldCount = 0;
byte FLUDD_SelectButtonReleasedCount = 0;

bool FLUDD_NozzleTurboActive = false;
byte FLUDD_TurboBuildUpCount = 0;
short int FLUDD_TurboDuration = 0;
byte FLUDD_TurboSoundEffectCounter = 0;
bool FLUDD_NozzleTurboWallCrash = false;
bool FLUDD_NozzleTurboBuildingUpOrInUse = false;

bool FLUDD_NozzleRocketActive = false;
byte FLUDD_RocketBuildUpCount = 0;
byte FLUDD_ROCKET_REQUIRED_BUILD_UP = 50;
byte FLUDD_RocketDuration = 0;
bool FLUDD_NozzleRocketJump = false;

bool FLUDD_NozzleHoverActive = false;
byte FLUDD_HoverDuration = 0;
byte FLUDD_HOVER_DURATION_LIMIT = 120;
bool FLUDD_NozzleHoverSideFlip = false;
byte FLUDD_HoverNextUseWaitTimer = 0x00;
byte FLUDD_HOVER_NEXT_USE_WAIT_LIMIT = 0x04;

bool FLUDD_PlayerStandingInWater = false;
bool FLUDD_PlayerSwimming = false;
bool FLUDD_PlayerOnGround = false;
bool FLUDD_PlayerCollidingWithWall = false;

unsigned int FLUDD_KeysAddress = 0xFFFFFFFF;
bool FLUDD_PlayerUpPressed = false;
bool FLUDD_PlayerLeftPressed = false;
bool FLUDD_PlayerRightPressed = false;
bool FLUDD_PlayerDownPressed = false;
bool FLUDD_PlayerDPadPressed = false;

void NozzleRocketBlastOffEffects();

// Following hook placed when writing to 020A0E58 the current key state
unsigned short keys = 0x0000;
void hook_0203BCE8()
{
	asm
	(
		"ldr r1, =keys				\t\n"
		"strh r0, [r1]				\t\n"
		
		"ldr r1, =FLUDD_KeysAddress	\t\n"
		"str r9, [r1]				\t\n"
	);
	
	FLUDD_PlayerUpPressed = ((keys & 0x40) == 0x40);
	FLUDD_PlayerLeftPressed = ((keys & 0x20) == 0x20);
	FLUDD_PlayerRightPressed = ((keys & 0x10) == 0x10);
	FLUDD_PlayerDownPressed = ((keys & 0x80) == 0x80);
	FLUDD_PlayerDPadPressed = ((keys & 0xF0) != 0x00);
}

// The current nozzle is determined by the current character and "hat" character
void hook_0203BCEC()
{
	if (CHARACTER == CHAR_Yoshi)
	{
		FLUDD_CurrentNozzle = FLUDD_NozzleNone;
	}
	else 
	{
		FLUDD_CurrentNozzle = FLUDD_Nozzle(HAT_CHARACTER);
	}
}

__attribute__((noinline)) void DecrementWaterLevel()
{
	FLUDD_WaterLevelDecrementUnitCounter++;
	if (FLUDD_WaterLevelDecrementUnitCounter >= 30)
	{
		FLUDD_WaterLevelDecrementUnitCounter = 0;
		if (FLUDD_WaterLevel > 0)
		{
			FLUDD_WaterLevel--;
		}
	}
}

__attribute__((noinline)) void IncrementWaterLevel()
{
	FLUDD_WaterLevelIncrementUnitCounter++;
	if (FLUDD_WaterLevelIncrementUnitCounter >= 2)
	{
		FLUDD_WaterLevelIncrementUnitCounter = 0;
		if (FLUDD_WaterLevel < 100)
		{
			FLUDD_WaterLevel++;
		}
	}
}

__attribute__((noinline)) void SubtractFullUnitsFromWaterLevel(byte amount)
{
	byte result = FLUDD_WaterLevel - amount;
	if (result > 0)
	{
		FLUDD_WaterLevel = result;
	}
	else
	{
		FLUDD_WaterLevel = 0;
	}
}

__attribute__((noinline)) bool IsPlayerStandingInWater()
{
	return ( PLAYER->currentTerrainType == 0x03 );
}

__attribute__((noinline)) bool IsPlayerSwimming()
{
	return (PLAYER->currentMovementState == 0x02 );
}

__attribute__((noinline)) bool IsPlayerOnGround()
{
	return (PLAYER->isOnGround);
}

__attribute__((noinline)) bool IsPlayerCollidingWithWall()
{
	return (PLAYER->currentCollisionState == 0x02 || PLAYER->currentCollisionState == 0x03);
}

bool NozzleTurboBuildingUpOrInUse() 
{
	return (FLUDD_CurrentNozzle == FLUDD_NozzleTurbo && 
		(FLUDD_NozzleTurboActive || FLUDD_IsActive));
}

bool NozzleRocketBuildingUpOrInUse() 
{
	return (FLUDD_CurrentNozzle == FLUDD_NozzleRocket && 
		(FLUDD_NozzleRocketActive || FLUDD_IsActive));
}

// The main function responsible for controlling F.L.U.D.D.
void hook_0202C1E8()
{
	if (HAT_CHARACTER == CHAR_Yoshi) return;
	
	FLUDD_PlayerStandingInWater = IsPlayerStandingInWater();
	FLUDD_PlayerSwimming = IsPlayerSwimming();
	FLUDD_PlayerOnGround = IsPlayerOnGround();
	FLUDD_PlayerCollidingWithWall = IsPlayerCollidingWithWall();
	bool isClimbingOrHanging = (Player_CurrentAction >= ACT_ClimbPole && 
		Player_CurrentAction <= ACT_GrabPole);
	bool isGroundPounding = (Player_CurrentAction >= ACT_GroundPoundStart && 
		Player_CurrentAction <= ACT_GroundPoundEnd);
	
	if (!FLUDD_IsActive || FLUDD_WaterLevel <= 0)
	{
		FLUDD_HoverNextUseWaitTimer++;
		
		if (FLUDD_CurrentNozzle == FLUDD_NozzleTurbo)
		{
			// Turbo 
			if (FLUDD_TurboBuildUpCount > 0 && 
				FLUDD_TurboBuildUpCount < FLUDD_TURBO_REQUIRED_BUILD_UP)
			{
				PLAYER->forwardSpeed = 0;
			}
			FLUDD_TurboBuildUpCount = 0;
			FLUDD_NozzleTurboActive = false;
			FLUDD_TurboDuration = 0;
			FLUDD_NozzleTurboBuildingUpOrInUse = false;
			if (!FLUDD_PlayerCollidingWithWall) 
			{
				FLUDD_NozzleTurboWallCrash = false; 
			}
		}
		else if (FLUDD_CurrentNozzle == FLUDD_NozzleRocket)
		{
			// Rocket
			
			// If the 'Y' key was released early during a Rocket 
			// Jump, we still need Mario not to take damage for 
			// ~jump height
			if (FLUDD_RocketDuration > 0 && 
				FLUDD_RocketDuration < FLUDD_ROCKET_DURATION_LIMIT_JUMP)
			{
				FLUDD_RocketDuration++;
				FLUDD_NozzleRocketJump = true;
			}
			else 
			{
				FLUDD_RocketDuration = 0;
				FLUDD_NozzleRocketJump = false;
				FLUDD_RocketBuildUpCount = 0;
				FLUDD_NozzleRocketActive = false;
			}
		}
		else if (FLUDD_CurrentNozzle == FLUDD_NozzleHover)
		{
			// Hover
			
			if (FLUDD_HoverNextUseWaitTimer >= FLUDD_HOVER_NEXT_USE_WAIT_LIMIT)
			{
				FLUDD_NozzleHoverActive = false;
				
				// Player has released 'Y' before hover limit reached
				if (FLUDD_HoverDuration > 0)
				{
					FLUDD_HoverDuration = FLUDD_HOVER_DURATION_LIMIT;
				}
				
				if (FLUDD_PlayerOnGround)
				{
					// Can only Hover again after landing
					FLUDD_HoverDuration = 0;
				}
				else if (Player_PreviousUniqueAction == ACT_LongJump)
				{
					// If player used hover nozzle during long jump, 
					// ensure they don't "float" through air due to 
					// slower ySpeed used during long jump
					PLAYER->unk09C = 0xFFFFC000;
					Player_PerformAction(PLAYER, ACT_StartToFallAfterJump, 
						0x40000000, 0x1000);
				} 
				else if (Player_PreviousUniqueAction == ACT_SideJump) 
				{
					Player_PerformAction(PLAYER, ACT_StartToFallAfterJump, 
						0x40000000, 0x1000);
				}
				
				FLUDD_HoverNextUseWaitTimer = 0x00;
			}
		}
		
		return;
	}
	
	if (FLUDD_PlayerStandingInWater || FLUDD_PlayerSwimming)
	{
		IncrementWaterLevel();
		if ( FLUDD_NozzleTurboActive == false && 
			 FLUDD_WaterLevel < 98 && 
			 FLUDD_WaterLevel % 2 == 0 )
		{
			PlaySoundEffect(FLUDD_REFILL_SOUND_EFFECT);
		}
	}
	
	if (FLUDD_CurrentNozzle == FLUDD_NozzleHover)
	{
		if (!FLUDD_PlayerSwimming && !isGroundPounding && 
			!isClimbingOrHanging)
		{
			if (PLAYER->featherCapTimeRemaining > 0)
			{
				PLAYER->featherCapTimeRemaining = 0;
			}
			
			if (FLUDD_HoverDuration < FLUDD_HOVER_DURATION_LIMIT)
			{
				if ( FLUDD_PlayerOnGround || 
					(!FLUDD_PlayerOnGround && PLAYER->ySpeed != 0) )
				{
					PressBKey();
				}
				
				if (Player_CurrentAction == ACT_SideJump)
				{
					PLAYER->yRot += 0x8000;
					FLUDD_NozzleHoverSideFlip = true;
				}

				Player_PerformAction(PLAYER, ACT_ReachedPeakOfFirstOrSecondJump, 
					0x40000000, 0x1000);
				
				if (FLUDD_HoverDuration % 8 == 0)
				{
					PlaySoundEffect(FLUDD_HOVER_SOUND_EFFECT);
				}
				
				FLUDD_NozzleHoverActive = true;
				FLUDD_HoverDuration++;
				if (PLAYER->ySpeed < FLUDD_HOVER_VERTICAL_SPEED)
				{
					PLAYER->ySpeed = FLUDD_HOVER_VERTICAL_SPEED;
				}
				PLAYER->jumpPeakHeight = PLAYER->yPos;
				PLAYER->currentJumpNumber = 0x01;
				
				if (!FLUDD_PlayerOnGround)
				{
					// This else block replaces the original jump movement method 0x020E28D4	
					
					// Change Mario's Y rotation when left and right buttons pressed
					short int turnIncrement = (!FLUDD_PlayerDownPressed) ? 
						(FLUDD_HOVER_TURN_INCREMENT) : (0 - FLUDD_HOVER_TURN_INCREMENT);
					RotatePlayerOnLeftRightPress(turnIncrement, turnIncrement);
					
					// Set his forward direction to match his Y rotation so that that's 
					// the direction he moves forward in
					if (FLUDD_PlayerUpPressed || !FLUDD_PlayerDownPressed)
					{
						PLAYER->forwardDirection = PLAYER->yRot;
					}
					
					if (FLUDD_PlayerUpPressed)
					{
						if (PLAYER->forwardSpeed >= FLUDD_HOVER_MAX_FORWARD_SPEED)
						{
							PLAYER->forwardSpeed = FLUDD_HOVER_MAX_FORWARD_SPEED;
						}
						else 
						{
							PLAYER->forwardSpeed += FLUDD_HOVER_FORWARD_SPEED_INCREMENT;
						}
					}
					else if (FLUDD_PlayerDownPressed)
					{
						if (PLAYER->forwardSpeed <= 0 - FLUDD_HOVER_MAX_FORWARD_SPEED)
						{
							PLAYER->forwardSpeed = 0 - FLUDD_HOVER_MAX_FORWARD_SPEED;
						}
						else 
						{
							PLAYER->forwardSpeed -= FLUDD_HOVER_FORWARD_SPEED_INCREMENT;
						}
					}
					
					// When neither the up nor down buttons are pressed, gradually 
					// come to a stop
					if (!(FLUDD_PlayerUpPressed || FLUDD_PlayerDownPressed))
					{
						if (PLAYER->forwardSpeed > 0)
						{
							if (PLAYER->forwardSpeed - 0x1000 >= 0)
							{
								PLAYER->forwardSpeed -= 0x1000;
							}
							else 
							{
								PLAYER->forwardSpeed = 0;
							}
						}
						else if (PLAYER->forwardSpeed < 0)
						{
							if (PLAYER->forwardSpeed + 0x1000 <= 0)
							{
								PLAYER->forwardSpeed += 0x1000;
							}
							else 
							{
								PLAYER->forwardSpeed = 0;
							}
						}
					}
				}
				DecrementWaterLevel();
			}
			else
			{
				FLUDD_NozzleHoverActive = false;
				FLUDD_HoverNextUseWaitTimer = 0x00;
			}
		}
		else
		{
			FLUDD_NozzleHoverActive = false;
			FLUDD_HoverNextUseWaitTimer = 0x00;
		}
	}
	else if (FLUDD_CurrentNozzle == FLUDD_NozzleRocket)
	{
		if (!FLUDD_PlayerSwimming && !isClimbingOrHanging)
		{
			if (FLUDD_RocketBuildUpCount < FLUDD_ROCKET_REQUIRED_BUILD_UP)
			{
				FLUDD_RocketBuildUpCount++;
				if (FLUDD_RocketBuildUpCount % 5 == 0)
				{
					PlaySoundEffect(FLUDD_ROCKET_BUILD_UP_SOUND_EFFECT);
				}
				ReleaseBKey();
			}
			else
			{
				if (FLUDD_RocketDuration < FLUDD_ROCKET_DURATION_LIMIT_B_KEY)
				{
					FLUDD_NozzleRocketActive = true;
					PressBKey();
					FLUDD_NozzleRocketJump = true;
					FLUDD_RocketDuration++;
					if (FLUDD_RocketDuration == 1)
					{
						SubtractFullUnitsFromWaterLevel(5);
						PlaySoundEffect(FLUDD_ROCKET_BLAST_OFF_SOUND_EFFECT);
					}
					
					if (FLUDD_RocketDuration <= 10)
					{
						NozzleRocketBlastOffEffects();
					}
				}
				else if (FLUDD_RocketDuration < FLUDD_ROCKET_DURATION_LIMIT_JUMP)
				{
					FLUDD_NozzleRocketActive = false;
					FLUDD_RocketDuration++;
				}
				else 
				{
					FLUDD_NozzleRocketActive = false;
					FLUDD_NozzleRocketJump = false;
					FLUDD_RocketBuildUpCount = 0;
					FLUDD_RocketDuration = 0;
				}
			}
		}
		else 
		{
			FLUDD_NozzleRocketActive = false;
			FLUDD_NozzleRocketJump = false;
			FLUDD_RocketBuildUpCount = 0;
			FLUDD_RocketDuration = 0;
		}
	}
	else if (FLUDD_CurrentNozzle == FLUDD_NozzleTurbo)
	{
		if (FLUDD_TurboBuildUpCount < FLUDD_TURBO_REQUIRED_BUILD_UP)
		{
			if (!FLUDD_PlayerCollidingWithWall && !isClimbingOrHanging) 
			{			
				FLUDD_TurboBuildUpCount++;
				if (!FLUDD_PlayerSwimming)
				{
					// If no directional buttons being pressed, 
					if (!IsDPadKeyPressed())
					{
						PressUpKey();
					}
					
					RotatePlayerOnLeftRightPress(0x0200, 0x0200);
					PLAYER->forwardDirection = PLAYER->yRot;
				}
			}
		}
		else if (FLUDD_PlayerCollidingWithWall || isClimbingOrHanging)
		{
			if (FLUDD_PlayerCollidingWithWall)
			{
				CrashPlayerIntoWall();
			}
			FLUDD_TurboBuildUpCount = 0;
			FLUDD_NozzleTurboActive = false;
			FLUDD_TurboDuration = 0;
		}
		else if (FLUDD_TurboDuration < FLUDD_TURBO_DURATION_LIMIT)
		{
			FLUDD_NozzleTurboActive = true;
			
			if (!FLUDD_PlayerSwimming)
			{
				// Pressing 'Up' is handled in hook_0203BD00
				DecrementWaterLevel();
				FLUDD_TurboDuration++;
			}
			else
			{
				ReleaseUpKey();
				PressDownKey();
			}
			
			if (FLUDD_PlayerOnGround) 
			{
				RotatePlayerOnLeftRightPress(0x0200, 0x0200);
			}
			PLAYER->forwardSpeed = 0x048000;
			PLAYER->forwardDirection = PLAYER->yRot;
			
			// &3 gives modulus 4, &15 mod 16, etc.
			//if (FLUDD_TurboDuration % 8 == 0) // fails in water
			if (FLUDD_TurboSoundEffectCounter >= 8) 
			{
				PlaySoundEffect(FLUDD_TURBO_SOUND_EFFECT);
				FLUDD_TurboSoundEffectCounter = 0;
			}
			else
			{
				FLUDD_TurboSoundEffectCounter++;
			}
		}
		else
		{
			FLUDD_TurboBuildUpCount = 0;
			FLUDD_NozzleTurboActive = false;
			FLUDD_TurboDuration = 0;
		}
		FLUDD_NozzleTurboBuildingUpOrInUse = NozzleTurboBuildingUpOrInUse();
	}
}

int FLUDD_SinRot = 0x00000000;
int FLUDD_CosRot = 0x00007FFF;
short int FLUDD_LastRotAngle = 0x0000;
const int FLUDD_ROCKET_PARTICLE_EFFECT_X_OFFSET = 0x00064000;
__attribute__((noinline)) void NozzleRocketBlastOffEffects() 
{
	short int rocketAngle = (PLAYER->yRot - 0x4000) / 2;
	if (rocketAngle != FLUDD_LastRotAngle)// || FLUDD_LastRotAngle == 0)
	{
		FLUDD_SinRot = sinLerp(rocketAngle);
		FLUDD_CosRot = cosLerp(rocketAngle);
		FLUDD_LastRotAngle = rocketAngle;
	}
	
	int xLoc = FLUDD_CosRot * (0 - (FLUDD_ROCKET_PARTICLE_EFFECT_X_OFFSET / 4096));
	int zLoc = FLUDD_SinRot * (FLUDD_ROCKET_PARTICLE_EFFECT_X_OFFSET / 4096);
	
	ParticleEffect(0x0B, 
		PLAYER->xPos + xLoc, 
		PLAYER->yPos + 0x00260000, 
		PLAYER->zPos + zLoc);
}

__attribute__((noinline)) void CrashPlayerIntoWall() 
{
	if (!FLUDD_PlayerSwimming) 
	{
		Player_HitWall(PLAYER);
	}
	else 
	{
		Player_KnockedBackUnderWater(PLAYER);
	}
	PLAYER->forwardSpeed = 0 - 0x10000;
	ParticleEffect(0x00C, PLAYER->xPos, PLAYER->yPos, PLAYER->zPos); // Red and orange stars
	FLUDD_NozzleTurboWallCrash = true;
	return;
}

__attribute__((noinline)) void RotatePlayerOnLeftRightPress(
	short int left_increment, short int right_increment)
{
	if (FLUDD_PlayerLeftPressed) 
	{
		PLAYER->yRot += left_increment;
	}
	else if (FLUDD_PlayerRightPressed) 
	{
		PLAYER->yRot -= right_increment;
	}
}

__attribute__((noinline)) void PlaySoundEffect(byte sound_effect)
{
	asm
	(
		"push {r0-r3, r14}		\t\n"
	);
	
	Player_PlaySoundEffect(SDAT_MINUS_0x10, 0, sound_effect);
	
	asm
	(
		"pop {r0-r3, r14}		\t\n"
	);
}

// Disable the 'Controller Options' being shown when 'SELECT' button 
// pressed
void repl_0202C158()
{
	asm
	(
		"mov r2, #0xFF					\t\n"
		"select_btn_original:			\t\n"
		"cmp r2, #0x00					\t\n"
	);
}

// If just entered Jump method, Player allowed to use Hover again, 
void hook_020E22C8()
{
	FLUDD_HoverDuration = 0;
}

// If just performed wall jump, Player allowed to use Hover again
void hook_020E1800()
{
	FLUDD_HoverDuration = 0;
}

// If just jumped out of water, Player allowed to use Hover again
void hook_020E162C()
{
	FLUDD_HoverDuration = 0;
}

// If just landing after a side-jump in which the Hover nozzle was 
// used, use the standard landing animation instead of showing the 
// side-jump one in which Mario is rotated 180 degrees about y axis.
void hook_020E0904_ov_02()
{
	if (FLUDD_NozzleHoverSideFlip) 
	{
		asm
		(
			"pop {r0-r12}		\t\n"
			"mov r1, #0x55		\t\n"
			"push {r0-r12}		\t\n"
		);
		
		FLUDD_NozzleHoverSideFlip = !FLUDD_NozzleHoverSideFlip;
	}
}

// If using the Turbo or Rocket nozzles, don't make Mario lean forward 
// whilst running.
void hook_020D41F0_ov_02() 
{
	if (NozzleTurboBuildingUpOrInUse() || NozzleRocketBuildingUpOrInUse())
	{
		asm
		(
			"pop {r0-r12} 		\t\n"
			"mov r1, #0x00		\t\n" // leaning angle of zero
			"push {r0-r12}		\t\n"
		);
	}
}

// If F.L.U.D.D. in use, change camera behaviour so that is fixed on 
// Mario's movement.
void hook_0200C680() 
{
	if (FLUDD_NozzleHoverActive || NozzleTurboBuildingUpOrInUse() 
		|| NozzleRocketBuildingUpOrInUse())
	{
		asm
		(
			"pop {r0-r12} 		\t\n"
			"mov r0, #0x00		\t\n"
			"push {r0-r12}		\t\n"
		);
	}
}

// If F.L.U.D.D. active, always use jump type 1 - 
// second jump
void repl_020E234C_ov_02()
{
	asm
	(
		"ldr r0, =FLUDD_NozzleTurboActive	\t\n"
		"ldrb r0, [r0]						\t\n"
		"cmp r0, #0x01						\t\n"
		"beq jump_type_1_020E234C			\t\n"
		
		"ldr r0, =FLUDD_NozzleRocketActive	\t\n"
		"ldrb r0, [r0]						\t\n"
		"cmp r0, #0x01						\t\n"
		"beq jump_type_1_020E234C			\t\n"
		
		"ldr r0, =FLUDD_NozzleHoverActive	\t\n"
		"ldrb r0, [r0]						\t\n"
		"cmp r0, #0x01						\t\n"
		"bne original_020E234C				\t\n"
		
		"jump_type_1_020E234C:				\t\n"
		"mov r12, #0x01						\t\n"
		"b exit_020E234C					\t\n"
		
		"original_020E234C:					\t\n"
		"strb r12, [r4, #0x6E1]				\t\n"
		
		"exit_020E234C:						\t\n"
	);
}

// If Rocket Nozzle build-up has begun, use sneak animation instead of 
// running animation
void repl_020D4B78_ov_02()
{
	asm
	(
		"ldr r1, =FLUDD_RocketBuildUpCount			\t\n"
		"ldrb r1, [r1]								\t\n"
		"ldr r2, =FLUDD_ROCKET_REQUIRED_BUILD_UP	\t\n"
		"ldrb r2, [r2]								\t\n"
		"cmp r1, #0x00								\t\n"
		"beq run_020D4B78							\t\n"
		"cmp r1, r2									\t\n"
		"bge run_020D4B78							\t\n"
		"mov r1, #0x61								\t\n"
		"b exit_020D4B78							\t\n"
		
		"run_020D4B78:								\t\n"
		"mov r1, #0x3F								\t\n"
		
		"exit_020D4B78:								\t\n"
	);
}

// If Rocket Nozzle build-up has begun, use 
// ACT_ReachedPeakOfFirstOrSecondJump = 0x4F animation instead of 
// wait animation if Mario standing still every frame
void repl_020D31C4_ov_02()
{
	asm
	(
		"ldr r1, =FLUDD_RocketBuildUpCount			\t\n"
		"ldrb r1, [r1]								\t\n"
		"ldr r3, =FLUDD_ROCKET_REQUIRED_BUILD_UP	\t\n"
		"ldrb r3, [r3]								\t\n"
		"cmp r1, #0x00								\t\n"
		"beq wait_020D31C0							\t\n"
		"cmp r1, r3									\t\n"
		"bge wait_020D31C0							\t\n"
		"mov r1, #0x4F								\t\n"
		"mov r3, #0x1000							\t\n"
		"mov r2, #0x40000000						\t\n" // No loop
		"b exit_020D31C0							\t\n"
		
		"wait_020D31C0:								\t\n"
		"mov r1, #0x47								\t\n"
		"mov r3, #0x1000							\t\n"
		
		"exit_020D31C0:								\t\n"
	);
}

// If during a Rocket Nozzle jump, don't update the peak position of 
// the jump used to determine how hard a fall it'll be
void repl_020E51E0_ov_02()
{
	asm
	(
		"ldr r1, =FLUDD_NozzleRocketJump	\t\n"
		"ldrb r1, [r1]						\t\n"
		"cmp r1, #0x01						\t\n"
		"beq exit_020E51E0					\t\n"
		"ldr r1, =FLUDD_NozzleHoverActive	\t\n"
		"ldrb r1, [r1]						\t\n"
		"cmp r1, #0x01						\t\n"
		"beq exit_020E51E0					\t\n"
		
		"original_020E51E0:					\t\n"
		"str r0, [r5, #0x684]				\t\n"
		
		"exit_020E51E0:						\t\n"
	);
}

// 020D477C reads value at 0209F49C and ANDS 0x800, forcing subsequent moveq 
// to always false will change Mario's state to run but still moves at walk 
// speed, 
void repl_020D4784_ov_02()
{
	asm
	(
		"mov r4, #0x00		\t\n"
	);
}

// Below code loads the number of the current jump and uses this to 
// determine the act to perform. If Turbo nozzle active, use 2nd jump 
// act, otherwise read the appropriate one from Player +0x6E1
void repl_020E2478_ov_02()
{
	asm
	(
		"ldr r2, =FLUDD_NozzleTurboActive		\t\n"
		"ldrb r2, [r2]							\t\n"
		"cmp r2, #0x01							\t\n"
		"beq second_jump_type_020E2478			\t\n"
		"ldr r2, =FLUDD_NozzleRocketActive		\t\n"
		"ldrb r2, [r2]							\t\n"
		"cmp r2, #0x01							\t\n"
		"beq second_jump_type_020E2478			\t\n"
		"b original_020E2478					\t\n"
		
		"second_jump_type_020E2478:				\t\n"
		"b exit_020E2478						\t\n" // r2 already 0x01
		
		"original_020E2478:						\t\n"
		"ldrb r2, [r4, #0x6E1]					\t\n"
		
		"exit_020E2478:							\t\n"
	);
}
