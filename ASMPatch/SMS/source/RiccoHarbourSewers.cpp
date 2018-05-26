#include "RiccoHarbourSewers.h"

byte RHS_LevelID = LEVEL_JollyRogerBay_Ship;

byte RHS_SewerRaceStarID = 4;
int RHS_SewerRaceTimeLimit = 45;

byte RHS_NumRaceMarkersPassed = 0;
byte RHS_TotalNumRaceMarkers = 0;

bool RHS_KoopaShellDestroyed = false;

void RHS_EndRace();

void hook_0214EAC0_ov_70()
{
	RHS_NumRaceMarkersPassed = 0;
	RHS_TotalNumRaceMarkers = 0;
	
	STR_RaceStarted = false;
	STR_RaceFinished = false;
	STR_RaceWon = false;
	
	RHS_KoopaShellDestroyed = true;
}

void RHS_EndRace()
{
	STR_ShouldOverrideBehaviour = true;
	STR_OverridePlayerPlus664Value = STR_STOP_TIMER_BEHAVIOUR;
	
	STR_ShouldOverrideStarID = true;
	STR_OverrideStarID = RHS_SewerRaceStarID;
	
	STR_ShouldOverrideMaxTime = true;
	STR_OverrideMaxTimeSeconds = (RHS_NumRaceMarkersPassed == RHS_TotalNumRaceMarkers) ? RHS_SewerRaceTimeLimit : 0;
}
