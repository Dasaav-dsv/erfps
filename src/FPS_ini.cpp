#include "FPS_ini.h"

void IniParseLineB(mINI::INIStructure& ini, const char* section, const char* key, bool& value)
{
	if (ini[section].has(key))
	{
		value = std::string("true").compare(ini[section][key]) == 0 ? true : false;
	}
	else
	{
		ini[section][key] = value ? "true" : "false";
	}
}

void IniParseLineF(mINI::INIStructure& ini, const char* section, const char* key, float& value)
{
	if (ini[section].has(key))
	{
		value = std::stof(ini[section][key].data());
	}
	else
	{
		ini[section][key] = std::to_string(value);
	}
}

void IniParseLineI(mINI::INIStructure& ini, const char* section, const char* key, uint32_t& value)
{
	if (ini[section].has(key))
	{
		value = atoi(ini[section][key].data());
	}
	else
	{
		ini[section][key] = std::to_string(value);
	}
}

void IniParseLineX(mINI::INIStructure& ini, const char* section, const char* key, char& value)
{
	if (ini[section].has(key))
	{
		value = toupper(*ini[section][key].data());
	}
	else
	{
		ini[section][key] = &value;
	}
}

void IniParseLineInd(mINI::INIStructure& ini, const char* section, const char* key, IndType* indicatorType)
{
	if (ini[section].has(key))
	{
		indicatorType->Set(ini[section][key].c_str());
	}
	else
	{
		ini[section][key] = indicatorType->GetStr();
	}
}

extern void ParseFPSIni()
{
	IniSettings* pIniSet = &iniSet;

	char iniPath[MAX_PATH];
	strcpy_s(iniPath, dllDir);
	strcat_s(iniPath, "erfps_settings.ini");

	mINI::INIFile file(iniPath);
	mINI::INIStructure ini;
	if (file.read(ini))
	{
		IniParseLineI(ini, "GENERAL", "Base FOV", pIniSet->fpFOV);
		IniParseLineF(ini, "GENERAL", "Walk Smoothing Strength", pIniSet->smoothingStr);
		IniParseLineF(ini, "GENERAL", "Aim Assist Snap Angle", pIniSet->aimAssistAng);
		IniParseLineF(ini, "GENERAL", "Sensitivity Scale", pIniSet->sensScalesistAng);
		IniParseLineB(ini, "GENERAL", "New Arm Positioning", pIniSet->pIniBool->isArmReposEnable);
		IniParseLineB(ini, "GENERAL", "Stronger Kick", pIniSet->pIniBool->isDMMMKicks);
		IniParseLineB(ini, "GENERAL", "True FPS Movement", pIniSet->pIniBool->isTrueFPS);

		IniParseLineB(ini, "TRUE FPS MOVEMENT", "Air Control", pIniSet->pIniBool->isAirCtrl);

		IniParseLineB(ini, "HEAD TRACKING", "Track Dodges", pIniSet->pIniBool->isTrackDodge);
		IniParseLineB(ini, "HEAD TRACKING", "Track on Hit", pIniSet->pIniBool->isTrackHit);

		IniParseLineB(ini, "HUD", "Aim Assist Indicator", pIniSet->pIniBool->isShowAutoaim);
		IniParseLineB(ini, "HUD", "Attack Indicators", pIniSet->pIniBool->isShowAttacks);
		IniParseLineInd(ini, "HUD", "Attack Indicator Type", pIniSet->indicatorType);

		IniParseLineB(ini, "CROSSHAIR", "Enable Crosshair", pIniSet->pIniBool->isCrosshairEnable);
		IniParseLineB(ini, "CROSSHAIR", "Dynamic Crosshair", pIniSet->pIniBool->isCrosshairDynamic);
		IniParseLineX(ini, "CROSSHAIR", "Crosshair Type", pIniSet->crosshairType);
		IniParseLineF(ini, "CROSSHAIR", "Size", pIniSet->crosshairSize);
		IniParseLineF(ini, "CROSSHAIR", "Spread", pIniSet->crosshairSpread);

		IniParseLineB(ini, "DEBUG", "Disable Startup Logos", pIniSet->pIniBool->isDisableLogos);
		IniParseLineB(ini, "DEBUG", "Debug Console Output", pIniSet->pIniBool->isDbgOut);
		IniParseLineB(ini, "DEBUG", "Use VirtualAlloc2", pIniSet->pIniBool->useVAlloc2);
		IniParseLineB(ini, "DEBUG", "Hook DirectX", pIniSet->pIniBool->isDXHook);

		file.write(ini, true);
	}
	else
	{
		ini["GENERAL"]["; first person field of view in degrees:"];
		ini["GENERAL"]["; ! can be further adjusted in-game on an English game localization !"];
		ini["GENERAL"]["Base FOV"] = std::to_string(pIniSet->fpFOV);
		ini["GENERAL"]["; movement headbobbing reduction strength in range 0.0 to 1.0:"];
		ini["GENERAL"]["Walk Smoothing Strength"] = std::to_string(pIniSet->smoothingStr);
		ini["GENERAL"]["; aim assist targetting angle in degrees in range 2.5 to 10.0:"];
		ini["GENERAL"]["Aim Assist Snap Angle"] = std::to_string(pIniSet->aimAssistAng);
		ini["GENERAL"]["; first person input sensitivity multiplier:"];
		ini["GENERAL"]["Sensitivity Scale"] = std::to_string(pIniSet->sensScalesistAng);
		ini["GENERAL"]["; reposition player's arms closer to the camera in first person:"];
		ini["GENERAL"]["New Arm Positioning"] = pIniSet->pIniBool->isArmReposEnable ? "true" : "false";
		ini["GENERAL"]["; kicks send small humanoid enemies flying - like Dark Messiah of Might and Magic:"];
		ini["GENERAL"]["Stronger Kick"] = pIniSet->pIniBool->isDMMMKicks ? "true" : "false";
		ini["GENERAL"]["; allow non-vanilla first person movement enchancements:"];
		ini["GENERAL"]["True FPS Movement"] = pIniSet->pIniBool->isTrueFPS ? "true" : "false";

		ini["TRUE FPS MOVEMENT"]["; enable precise movement control when jumping in first person:"];
		ini["TRUE FPS MOVEMENT"]["Air Control"] = pIniSet->pIniBool->isAirCtrl ? "true" : "false";

		ini["HEAD TRACKING"]["; enable camera head tracking for rolls:"];
		ini["HEAD TRACKING"]["; ! overriden by an in-game setting on an English game localization !"];
		ini["HEAD TRACKING"]["Track Dodges"] = pIniSet->pIniBool->isTrackDodge ? "true" : "false";
		ini["HEAD TRACKING"]["; enable camera head tracking for player staggers and knockdowns:"];
		ini["HEAD TRACKING"]["Track on Hit"] = pIniSet->pIniBool->isTrackHit ? "true" : "false";

		ini["HUD"]["; enable an indicator for aim assist:"];
		ini["HUD"]["Aim Assist Indicator"] = pIniSet->pIniBool->isShowAutoaim ? "true" : "false";
		ini["HUD"]["; enable indicators for incoming attacks:"];
		ini["HUD"]["Attack Indicators"] = pIniSet->pIniBool->isShowAttacks ? "true" : "false";
		ini["HUD"]["; attack indicator types:"];
		ini["HUD"][";   LARGE - crosshair-centered, default"];
		ini["HUD"][";   SMALL - crosshair-centered, smaller than LARGE"];
		ini["HUD"][";   ITEMS - centered around the item HUD, useful for casters"];
		ini["HUD"]["Attack Indicator Type"] = pIniSet->indicatorType->GetStr();

		ini["CROSSHAIR"]["; enable first person crosshair:"];
		ini["CROSSHAIR"]["Enable Crosshair"] = pIniSet->pIniBool->isCrosshairEnable ? "true" : "false";
		ini["CROSSHAIR"]["; crosshair reacts to player movement:"];
		ini["CROSSHAIR"]["Dynamic Crosshair"] = pIniSet->pIniBool->isCrosshairDynamic ? "true" : "false";
		ini["CROSSHAIR"]["; crosshair types:"];
		ini["CROSSHAIR"][";   A - angled: < >"];
		ini["CROSSHAIR"][";   B - basic:   +"];
		ini["CROSSHAIR"][";   C - circle:  o"];
		ini["CROSSHAIR"][";   D - dot:     ."];
		ini["CROSSHAIR"]["Crosshair Type"] = &pIniSet->crosshairType;
		ini["CROSSHAIR"]["; crosshair size multiplier:"];
		ini["CROSSHAIR"]["Size"] = std::to_string(pIniSet->crosshairSize);
		ini["CROSSHAIR"]["; crosshair spread multiplier:"];
		ini["CROSSHAIR"]["Spread"] = std::to_string(pIniSet->crosshairSpread);

		ini["DEBUG"]["; disable intro logos on game startup:"];
		ini["DEBUG"]["Disable Startup Logos"] = pIniSet->pIniBool->isDisableLogos ? "true" : "false";
		ini["DEBUG"]["; enable console output for debugging:"];
		ini["DEBUG"]["Debug Console Output"] = pIniSet->pIniBool->isDbgOut ? "true" : "false";
		ini["DEBUG"]["; use the VirtualAlloc2 memory allocation function:"];
		ini["DEBUG"]["Use VirtualAlloc2"] = pIniSet->pIniBool->useVAlloc2 ? "true" : "false";
		ini["DEBUG"]["; hook DirectX - required for all custom HUD features:"];
		ini["DEBUG"]["Hook DirectX"] = pIniSet->pIniBool->isDXHook ? "true" : "false";

		file.generate(ini, true);
	}
}
