#ifndef SHARED_TIME_RACE
#define SHARED_TIME_RACE

#include "SM64DS.h"

extern bool STR_ShouldOverrideBehaviour;
extern int STR_OverridePlayerPlus664Value;
extern bool STR_ShouldOverrideStarID;
extern byte STR_OverrideStarID;
extern bool STR_ShouldOverrideMaxTime;
extern int STR_OverrideMaxTimeSeconds;

extern int STR_START_TIMER_BEHAVIOUR;
extern int STR_STOP_TIMER_BEHAVIOUR;

// Should be reset by the level using them
extern bool STR_RaceStarted;
extern bool STR_RaceFinished;
extern bool STR_RaceWon;

#endif