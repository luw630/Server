#include "Stdafx.h"
#include "GlobalSetting.h"

float CGlobalSetting::ms_fActorMoveSpeedFactor = 1.0f;

void CGlobalSetting::LoadSetting(const char* szFileName)
{
	IniFile ini;
	if (!ini.Load(szFileName))
		return;

	ini.GetFloat("ACTOR", "MoveSpeedFactor", 1.0f, &ms_fActorMoveSpeedFactor);
}