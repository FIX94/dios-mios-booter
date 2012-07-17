/****************************************************************************
 * DIOS-MIOS Booter - A small and easy DIOS-MIOS (Lite) Game Booter
 * Copyright (C) 2012 FIX94
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#ifndef MENU_HPP_
#define MENU_HPP_

#include <stdio.h>
#include <string.h>
#include <wiiuse/wpad.h>

#include "gc.h"
#include "sys.h"
#include "config.hpp"
#include "GameList/GameList.hpp"
#include "DeviceMounter/DeviceHandler.hpp"

using namespace std;

enum
{
	MENU_MAIN = 0,
	MENU_OPTIONS,
	MENU_GAME_OPTIONS,
	MENU_NONE,
};

class Menu
{
public:
	void Start();
	int BootGame();
	void UnMountDevices();

private:
	/* Main */
	void InitMain();
	void MainMenu();

	bool MainMenuAutoboot;

	/* Options */
	void InitOptions();
	void OptionsMenu();
	void SetOption(u32 Option, u8 direction);
	const char *GetOption(u32 Option);

	string GetOptionFromString(vector<string> List, string *Option, u8 *Option_num);
	void NextOptionFromString(vector<string> List, string *Option, u8 *Option_num);
	void PrevOptionFromString(vector<string> List, string *Option, u8 *Option_num);

	void ReadConfig(const char *Domain);
	void WriteConfig(const char *Domain);

	vector<u32> OptionList;
	vector<string> OptionNameList;

	/* Game Options */
	void GameOptionsMenu();

	bool Autoboot;
	bool DriveReset;
	bool NTSCJ_Patch;

	u8 GC_Video_Mode;
	u8 GC_Language;
	u8 DM_Mode;
	u8 DM_Patch;

	bool DM_NoDisc;
	bool DM_ForceWide;

	/* General */
	int CheckMIOS();
	void MainLoop();
	bool AbortOperation();
	void ReadGameDir();

	Config BooterINI;
	DML_CFG *BooterCFG;
	GameList List;
	DeviceHandler DevHandler;

	char MIOS_Info[256];
	char listlimits[77];
	time_t AutobootTimeout;

	u8 currentDev;
	u8 currentMenu;

	u8 position;
	u8 listposition;
};

#endif
