/****************************************************************************
 * DIOS-MIOS Booter - A small and easy DIOS-MIOS (Lite) Game Booter
 * Copyright (C) 2012  FIX94
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
#include "Menu.hpp"
#include "defines.h"

string GC_Language_string;
vector<string> GC_Language_strings;

string GC_Video_string;
vector<string> GC_Video_strings;

void Menu::InitOptions()
{
	GC_Language_strings.push_back("Wii"); GC_Language_strings.push_back("English");
	GC_Language_strings.push_back("German"); GC_Language_strings.push_back("French");
	GC_Language_strings.push_back("Spanish"); GC_Language_strings.push_back("Italian");
	GC_Language_strings.push_back("Dutch");

	GC_Video_strings.push_back("Auto"); GC_Video_strings.push_back("PAL50");
	GC_Video_strings.push_back("NTSC480i"); GC_Video_strings.push_back("PAL60");
	GC_Video_strings.push_back("NTSC480p");  GC_Video_strings.push_back("PAL480p");
	GC_Video_strings.push_back("NTSC480p (patched)");  GC_Video_strings.push_back("PAL480p (patched)");

	OptionNameList.push_back("Boot Method       "); OptionList.push_back(0x424F4F54); //BOOT
	OptionNameList.push_back("Language          "); OptionList.push_back(0x4C414E47); //LANG 
	OptionNameList.push_back("Video Mode        "); OptionList.push_back(0x56494445); //VIDE
	OptionNameList.push_back("Booter Drive Reset"); OptionList.push_back(0x52455345); //RESE
	OptionNameList.push_back("NTSC-J Patch      "); OptionList.push_back(0x4A415041); //JAPA

	OptionNameList.push_back("Cheats            ");	OptionList.push_back(DML_CFG_CHEATS);
	OptionNameList.push_back("Debugger          ");	OptionList.push_back(DML_CFG_DEBUGGER);
	OptionNameList.push_back("Wait for Debugger ");	OptionList.push_back(DML_CFG_DEBUGWAIT);
	OptionNameList.push_back("NMM               ");	OptionList.push_back(DML_CFG_NMM);
	OptionNameList.push_back("NMM Debug         ");	OptionList.push_back(DML_CFG_NMM_DEBUG);
	OptionNameList.push_back("Activity LED      ");	OptionList.push_back(DML_CFG_ACTIVITY_LED);
	OptionNameList.push_back("Pad Hook          ");	OptionList.push_back(DML_CFG_PADHOOK);
	OptionNameList.push_back("No Disc Patch     ");	OptionList.push_back(DML_CFG_NODISC);
}

void Menu::OptionsMenu()
{
	if(!Autoboot)
		ReadConfig("GENERAL");
	else
		ReadConfig(AUTOBOOT_GAME_ID);

	u8 verticalselect = 1;
	u8 Page = 1;
	u8 Pages = 3;
	u8 PageSize = 6;
	u8 GeneralOptions = 5;

	while(!Sys_Exit())
	{
		MainLoop();
		printf("Options\nPress the B Button to return to game selection \nor -/+ (L/R) to switch page.\n");
		if(!OldDML)
			printf(" \nPage %i/%i\n", Page, Pages);
		else
			printf(" \n \n");
		for(u8 i = PageSize * (Page - 1); i < OptionList.size(); i++)
		{
			if(i >= PageSize * (Page) || (i == GeneralOptions && OldDML))
				break;
			if(verticalselect - 1 == i)
				printf("\x1b[33m");
			else
				printf("\x1b[37m");
			printf(" \n%s           <<< %s >>>\n", OptionNameList.at(i).c_str(), GetOption(OptionList.at(i)));
		}
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_RIGHT) || (PAD_ButtonsDown(0) == PAD_BUTTON_RIGHT))
			SetOption(OptionList.at(verticalselect - 1), 9);
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_LEFT) || (PAD_ButtonsDown(0) == PAD_BUTTON_LEFT))
			SetOption(OptionList.at(verticalselect - 1), 1);
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_UP) || (PAD_ButtonsDown(0) == PAD_BUTTON_UP))
			verticalselect--;
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_DOWN) || (PAD_ButtonsDown(0) == PAD_BUTTON_DOWN))
			verticalselect++;
		if(!OldDML && ((WPAD_ButtonsDown(0) == WPAD_BUTTON_PLUS) || (PAD_ButtonsDown(0) == PAD_TRIGGER_R)))
		{
			Page++;
			verticalselect = 1 + PageSize * (Page - 1);
		}
		if(!OldDML && ((WPAD_ButtonsDown(0) == WPAD_BUTTON_MINUS) || (PAD_ButtonsDown(0) == PAD_TRIGGER_L)))
		{
			Page--;
			verticalselect = 1 + PageSize * (Page - 1);
		}
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_B) || (PAD_ButtonsDown(0) == PAD_BUTTON_B))
			break;
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_HOME) || (PAD_ButtonsDown(0) == PAD_BUTTON_START))
			Input_Reset();
		if(Page == 0)
		{
			Page = Pages;
			verticalselect = 1 + PageSize * (Page - 1);
		}
		if(Page > Pages)
		{
			Page = 1;
			verticalselect = 1 + PageSize * (Page - 1);
		}
		if(OldDML && verticalselect == 0)
			verticalselect = GeneralOptions;
		else if(OldDML && verticalselect > GeneralOptions)
			verticalselect = 1;
		else if(verticalselect == PageSize * (Page - 1))
			verticalselect = PageSize * (Page);
		else if(verticalselect > PageSize * (Page))
			verticalselect = 1 + PageSize * (Page - 1);
	}
	/* Set new Options */
	if(!Autoboot)
		WriteConfig("GENERAL");
	else
		WriteConfig(AUTOBOOT_GAME_ID);
	currentMenu = MENU_MAIN;
}


string Menu::GetLanguage()
{
	bool found = false;
	u8 i;
	for(i = 0; i < GC_Language_strings.size(); i++)
	{
		if(strcasecmp(GC_Language_strings.at(i).c_str(), GC_Language_string.c_str()) == 0)
		{
			found = true;
			GC_Language = i;
			break;
		}
	}
	if(!found)
	{
		GC_Language_string = GC_Language_strings.at(0);
		GC_Language = 0;
		i = 0;
	}

	return GC_Language_strings.at(i);
}

string Menu::GetVideoMode()
{
	bool found = false;
	u8 i;
	for(i = 0; i < GC_Video_strings.size(); i++)
	{
		if(strcasecmp(GC_Video_strings.at(i).c_str(), GC_Video_string.c_str()) == 0)
		{
			found = true;
			GC_Video_Mode = i;
			break;
		}
	}
	if(!found)
	{
		GC_Video_string = GC_Video_strings.at(0);
		GC_Video_Mode = 0;
		i = 0;
	}

	return GC_Video_strings.at(i);
}

void Menu::WriteConfig(const char *Domain)
{
	GetLanguage();
	GetVideoMode();
	BooterINI.setBool(Domain, "ActivityLED", (BooterCFG->Config & DML_CFG_ACTIVITY_LED));
	BooterINI.setBool(Domain, "PadReset", (BooterCFG->Config & DML_CFG_PADHOOK));
	BooterINI.setBool(Domain, "Cheats", (BooterCFG->Config & DML_CFG_CHEATS));
	BooterINI.setBool(Domain, "NMM", (BooterCFG->Config & DML_CFG_NMM));
	BooterINI.setBool(Domain, "NMM_Debug", (BooterCFG->Config & DML_CFG_NMM_DEBUG));
	BooterINI.setBool(Domain, "No_Disc_Patch", (BooterCFG->Config & DML_CFG_NODISC));
	BooterINI.setBool(Domain, "Debugger", (BooterCFG->Config & DML_CFG_DEBUGGER));
	BooterINI.setBool(Domain, "Wait_for_Debugger", (BooterCFG->Config & DML_CFG_DEBUGWAIT));
	BooterINI.setBool(Domain, "Drive_Reset", DriveReset);
	BooterINI.setBool(Domain, "OldDML", OldDML);
	BooterINI.setBool(Domain, "NTSCJ_Patch", NTSCJ_Patch);
	BooterINI.setBool("GENERAL", "usb", currentDev);
	BooterINI.setString(Domain, "Language", GC_Language_string);
	BooterINI.setString(Domain, "VideoMode", GC_Video_string);
	if(DevHandler.SD_Mounted() || DevHandler.USB_Mounted())
		BooterINI.save(false);
}

void Menu::NextLanguage()
{
	string current_GC_Language_string = GetLanguage();
	for(u8 i = 0; i < GC_Language_strings.size(); i++)
	{
		if(strcasecmp(current_GC_Language_string.c_str(), GC_Language_strings.at(i).c_str()) == 0)
		{
			if(i+1 == (u8)GC_Language_strings.size())
				GC_Language_string = GC_Language_strings.front();
			else
				GC_Language_string = GC_Language_strings.at(i+1).c_str();
		}
	}
}

void Menu::PrevLanguage()
{
	string current_GC_Language_string = GetLanguage();
	for(u8 i = 0; i < GC_Language_strings.size(); i++)
	{
		if(strcasecmp(current_GC_Language_string.c_str(), GC_Language_strings.at(i).c_str()) == 0)
		{
			if(i == 0)
				GC_Language_string = GC_Language_strings.back();
			else
				GC_Language_string = GC_Language_strings.at(i-1).c_str();
		}
	}
}

void Menu::NextVideoMode()
{
	string current_GC_Video_string = GetVideoMode();
	for(u8 i = 0; i < GC_Video_strings.size(); i++)
	{
		if(strcasecmp(current_GC_Video_string.c_str(), GC_Video_strings.at(i).c_str()) == 0)
		{
			if(i+1 == (u8)GC_Video_strings.size())
				GC_Video_string = GC_Video_strings.front();
			else
				GC_Video_string = GC_Video_strings.at(i+1).c_str();
		}
	}
}

void Menu::PrevVideoMode()
{
	string current_GC_Video_string = GetVideoMode();
	for(u8 i = 0; i < GC_Video_strings.size(); i++)
	{
		if(strcasecmp(current_GC_Video_string.c_str(), GC_Video_strings.at(i).c_str()) == 0)
		{
			if(i == 0)
				GC_Video_string = GC_Video_strings.back();
			else
				GC_Video_string = GC_Video_strings.at(i-1).c_str();
		}
	}
}

void Menu::ReadConfig(const char *Domain)
{
	if(BooterINI.loaded())
	{
		if(BooterINI.getBool(Domain, "ActivityLED"))
			BooterCFG->Config |= DML_CFG_ACTIVITY_LED;
		else
			BooterCFG->Config &= ~DML_CFG_ACTIVITY_LED;
		if(BooterINI.getBool(Domain, "PadReset"))
			BooterCFG->Config |= DML_CFG_PADHOOK;
		else
			BooterCFG->Config &= ~DML_CFG_PADHOOK;
		if(BooterINI.getBool(Domain, "Cheats"))
			BooterCFG->Config |= DML_CFG_CHEATS;
		else
			BooterCFG->Config &= ~DML_CFG_CHEATS;
		if(BooterINI.getBool(Domain, "NMM"))
			BooterCFG->Config |= DML_CFG_NMM;
		else
			BooterCFG->Config &= ~DML_CFG_NMM;
		if(BooterINI.getBool(Domain, "NMM_Debug"))
			BooterCFG->Config |= DML_CFG_NMM_DEBUG;
		else
			BooterCFG->Config &= ~DML_CFG_NMM_DEBUG;
		if(BooterINI.getBool(Domain, "No_Disc_Patch"))
			BooterCFG->Config |= DML_CFG_NODISC;
		else
			BooterCFG->Config &= ~DML_CFG_NODISC;
		if(BooterINI.getBool(Domain, "Debugger"))
			BooterCFG->Config |= DML_CFG_DEBUGGER;
		else
			BooterCFG->Config &= ~DML_CFG_DEBUGGER;
		if(BooterINI.getBool(Domain, "Wait_for_Debugger"))
			BooterCFG->Config |= DML_CFG_DEBUGWAIT;
		else
			BooterCFG->Config &= ~DML_CFG_DEBUGWAIT;
		GC_Language_string = BooterINI.getString(Domain, "Language", "Wii");
		GetLanguage();
		GC_Video_string = BooterINI.getString(Domain, "VideoMode", "Auto");
		GetVideoMode();
		DriveReset = BooterINI.getBool(Domain, "Drive_Reset", true);
		OldDML = BooterINI.getBool(Domain, "OldDML", false);
		NTSCJ_Patch = BooterINI.getBool(Domain, "NTSCJ_Patch", false);
		currentDev = BooterINI.getBool("GENERAL", "usb", false);
	}
	else
		WriteConfig(Domain);
}


const char* Menu::GetOption(u32 Option)
{
	if(Option == 0x4C414E47) //LANG
		return GetLanguage().c_str();
	else if(Option == 0x56494445) //VIDE
		return GetVideoMode().c_str();
	else if(Option == 0x52455345) //RESE
		return (DriveReset == true) ? "On" : "Off";
	else if(Option == 0x424F4F54) //BOOT
		return (OldDML == true) ? "Old" : "New";
	else if(Option == 0x4A415041) //JAPA
		return (NTSCJ_Patch == true) ? "Yes" : "No";
	else if(BooterCFG->Config & Option)
		return "On";
	else
		return "Off";
}

void Menu::SetOption(u32 Option, u8 direction)
{
	if(Option == 0x4C414E47) //LANG
	{
		if(direction == 9) //Right
			NextLanguage();
		else if(direction == 1) //Left
			PrevLanguage();
		return;
	}
	else if(Option == 0x56494445) //VIDE
	{
		if(direction == 9) //Right
			NextVideoMode();
		else if(direction == 1) //Left
			PrevVideoMode();
		return;
	}
	else if(Option == 0x52455345) //RESE
	{
		DriveReset = (DriveReset == true) ? false : true;
		return;
	}
	else if(Option == 0x424F4F54) //BOOT
	{
		OldDML = (OldDML == true) ? false : true;
		return;
	}
	else if(Option == 0x4A415041) //JAPA
	{
		NTSCJ_Patch = (NTSCJ_Patch == true) ? false : true;
		return;
	}
	else if(BooterCFG->Config & Option)
		BooterCFG->Config &= ~Option;
	else
		BooterCFG->Config |= Option;
}
