#include "RiccoHarbour.h"

byte RH_LevelID = LEVEL_JollyRogerBay;
byte RH_ActSelectorID = 0xFF;

byte RH_ActivateCranesMissionStarID = 1;
bool RH_AreCranesActivated = false;

void hook_0214EAC0_ov_6F()
{
	RH_ActSelectorID = *((volatile byte*)(ACT_SELECTOR_ID_TABLE_ADDRESS + RH_LevelID));
	
	if (STAR_ID != RH_ActivateCranesMissionStarID)
	{
		RH_AreCranesActivated = AreCannonsActivatedForCurrentLevel();
	}
	else
	{
		RH_AreCranesActivated = false;
	}
}
