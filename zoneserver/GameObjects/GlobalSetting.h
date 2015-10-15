#pragma once

class CGlobalSetting
{
public:
	static void LoadSetting(const char* szFileName);
	static float GetActorMoveSpeedFactor() { return ms_fActorMoveSpeedFactor; }

private:
	static float ms_fActorMoveSpeedFactor;
};