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
#include "Menu.hpp"
#include "defines.h"

string GC_Language_string;
vector<string> GC_Language_strings;

string GC_Video_string;
vector<string> GC_Video_strings;

string DM_Mode_string;
vector<string> DM_Mode_strings;

string DM_Patch_string;
vector<string> DM_Patch_strings;

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

	DM_Mode_strings.push_back("Old"); 
	DM_Mode_strings.push_back("New (DM 2.0-)"); DM_Mode_strings.push_back("New (DM 2.1+)");

	DM_Patch_strings.push_back("None"); DM_Patch_strings.push_back("Auto");
	DM_Patch_strings.push_back("Force");

	OptionNameList.push_back("Boot Method         "); OptionList.push_back(0x424F4F54); //BOOT
	OptionNameList.push_back("Language            "); OptionList.push_back(0x4C414E47); //LANG
	OptionNameList.push_back("Video Mode          "); OptionList.push_back(0x56494445); //VIDE
	OptionNameList.push_back("Booter Drive Reset  "); OptionList.push_back(0x52455345); //RESE
	OptionNameList.push_back("NTSC-J Patch        "); OptionList.push_back(0x4A415041); //JAPA
	OptionNameList.push_back("Video Patching      "); OptionList.push_back(0x464F5243); //FORC

	OptionNameList.push_back("Cheats              "); OptionList.push_back(DML_CFG_CHEATS);
	OptionNameList.push_back("Debugger            "); OptionList.push_back(DML_CFG_DEBUGGER);
	OptionNameList.push_back("Wait for Debugger   "); OptionList.push_back(DML_CFG_DEBUGWAIT);
	OptionNameList.push_back("NMM                 "); OptionList.push_back(DML_CFG_NMM);
	OptionNameList.push_back("NMM Debug           "); OptionList.push_back(DML_CFG_NMM_DEBUG);
	OptionNameList.push_back("Activity LED        "); OptionList.push_back(DML_CFG_ACTIVITY_LED);
	OptionNameList.push_back("Pad Hook            "); OptionList.push_back(DML_CFG_PADHOOK);

	OptionNameList.push_back("No Disc             "); OptionList.push_back(0x44495343); //DISC
	OptionNameList.push_back("Widescreen (DM 2.1+)"); OptionList.push_back(0x57494445); //WIDE
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

	while(!Sys_Exit())
	{
		MainLoop();
		printf("Options\nPress the B Button to return to game selection \nor -/+ (L/R) to switch page.\n");
		printf(" \nPage %i/%i\n", Page, Pages);
		for(u8 i = PageSize * (Page - 1); i < OptionList.size(); i++)
		{
			if(i >= PageSize * (Page))
				break;
			if(verticalselect - 1 == i)
				printf("\x1b[33m");
			else
				printf("\x1b[37m");
			printf(" \n%s           <<< %s >>>\n", OptionNameList.at(i).c_str(), GetOption(OptionList.at(i)));
		}
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_RIGHT) || (PAD_ButtonsDown(0) == PAD_BUTTON_RIGHT))
			SetOption(OptionList.at(verticalselect - 1), 1);
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_LEFT) || (PAD_ButtonsDown(0) == PAD_BUTTON_LEFT))
			SetOption(OptionList.at(verticalselect - 1), 0);
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_UP) || (PAD_ButtonsDown(0) == PAD_BUTTON_UP))
			verticalselect--;
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_DOWN) || (PAD_ButtonsDown(0) == PAD_BUTTON_DOWN))
			verticalselect++;
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_PLUS) || (PAD_ButtonsDown(0) == PAD_TRIGGER_R))
		{
			Page++;
			verticalselect = 1 + PageSize * (Page - 1);
		}
		if((WPAD_ButtonsDown(0) == WPAD_BUTTON_MINUS) || (PAD_ButtonsDown(0) == PAD_TRIGGER_L))
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


string Menu::GetOptionFromString(vector<string> List, string *Option, u8 *Option_num)
{
	bool found = false;
	u8 i;
	for(i = 0; i < List.size(); i++)
	{
		if(strcasecmp(List.at(i).c_str(), (*Option).c_str()) == 0)
		{
			found = true;
			(*Option_num) = i;
			break;
		}
	}
	if(!found)
	{
		(*Option) = List.at(0);
		(*Option_num) = 0;
		i = 0;
	}

	return List.at(i);
}

void Menu::NextOptionFromString(vector<string> List, string *Option, u8 *Option_num)
{
	string current_OptionFromString = GetOptionFromString(List, Option, Option_num);
	for(u8 i = 0; i < List.size(); i++)
	{
		if(strcasecmp(current_OptionFromString.c_str(), List.at(i).c_str()) == 0)
		{
			if(i+1 == (u8)List.size())
				(*Option) = List.front();
			else
				(*Option) = List.at(i+1).c_str();
		}
	}
}

void Menu::PrevOptionFromString(vector<string> List, string *Option, u8 *Option_num)
{
	string current_OptionFromString = GetOptionFromString(List, Option, Option_num);
	for(u8 i = 0; i < List.size(); i++)
	{
		if(strcasecmp(current_OptionFromString.c_str(), List.at(i).c_str()) == 0)
		{
			if(i == 0)
				(*Option) = List.back();
			else
				(*Option) = List.at(i-1).c_str();
		}
	}
}

void Menu::WriteConfig(const char *Domain)
{
	GetOptionFromString(GC_Language_strings, &GC_Language_string, &GC_Language);
	BooterINI.setString(Domain, "Language", GC_Language_string);

	GetOptionFromString(GC_Video_strings, &GC_Video_string, &GC_Video_Mode);
	BooterINI.setString(Domain, "VideoMode", GC_Video_string);

	GetOptionFromString(DM_Mode_strings, &DM_Mode_string, &DM_Mode);
	BooterINI.setString(Domain, "DM_Mode", DM_Mode_string);

	GetOptionFromString(DM_Patch_strings, &DM_Patch_string, &DM_Patch);
	BooterINI.setString(Domain, "DM_Patch", DM_Patch_string);

	BooterINI.setBool(Domain, "No_Disc_Patch", DM_NoDisc);
	BooterINI.setBool(Domain, "Widescreen", DM_ForceWide);
	BooterINI.setBool(Domain, "Drive_Reset", DriveReset);
	BooterINI.setBool(Domain, "NTSCJ_Patch", NTSCJ_Patch);

	BooterINI.setBool(Domain, "ActivityLED", (BooterCFG->Config & DML_CFG_ACTIVITY_LED));
	BooterINI.setBool(Domain, "PadReset", (BooterCFG->Config & DML_CFG_PADHOOK));
	BooterINI.setBool(Domain, "Cheats", (BooterCFG->Config & DML_CFG_CHEATS));
	BooterINI.setBool(Domain, "NMM", (BooterCFG->Config & DML_CFG_NMM));
	BooterINI.setBool(Domain, "NMM_Debug", (BooterCFG->Config & DML_CFG_NMM_DEBUG));
	BooterINI.setBool(Domain, "Debugger", (BooterCFG->Config & DML_CFG_DEBUGGER));
	BooterINI.setBool(Domain, "Wait_for_Debugger", (BooterCFG->Config & DML_CFG_DEBUGWAIT));

	BooterINI.setBool("GENERAL", "usb", currentDev);
	if(DevHandler->SD_Mounted() || DevHandler->USB_Mounted())
		BooterINI.save(false);
}

void Menu::ReadConfig(const char *Domain)
{
	if(BooterINI.loaded())
	{
		//0x4C414E47 LANG
		GC_Language_string = BooterINI.getString(Domain, "Language", GC_Language_strings.at(0));
		GetOptionFromString(GC_Language_strings, &GC_Language_string, &GC_Language);
		//0x56494445 VIDE
		GC_Video_string = BooterINI.getString(Domain, "VideoMode", GC_Video_strings.at(0));
		GetOptionFromString(GC_Video_strings, &GC_Video_string, &GC_Video_Mode);
		//0x424F4F54 BOOT
		DM_Mode_string = BooterINI.getString(Domain, "DM_Mode", DM_Mode_strings.at(2));
		GetOptionFromString(DM_Mode_strings, &DM_Mode_string, &DM_Mode);
		//0x464F5243 FORC
		DM_Patch_string = BooterINI.getString(Domain, "DM_Patch", DM_Patch_strings.at(1));
		GetOptionFromString(DM_Patch_strings, &DM_Patch_string, &DM_Patch);
		//0x44495343 DISC
		DM_NoDisc = BooterINI.getBool(Domain, "No_Disc_Patch", true);
		//0x57494445 WIDE
		DM_ForceWide = BooterINI.getBool(Domain, "Widescreen", false);
		//0x52455345 RESE
		DriveReset = BooterINI.getBool(Domain, "Drive_Reset", false);
		//0x4A415041 JAPA
		NTSCJ_Patch = BooterINI.getBool(Domain, "NTSCJ_Patch", false);

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

		if(BooterINI.getBool(Domain, "Debugger"))
			BooterCFG->Config |= DML_CFG_DEBUGGER;
		else
			BooterCFG->Config &= ~DML_CFG_DEBUGGER;

		if(BooterINI.getBool(Domain, "Wait_for_Debugger"))
			BooterCFG->Config |= DML_CFG_DEBUGWAIT;
		else
			BooterCFG->Config &= ~DML_CFG_DEBUGWAIT;
		currentDev = BooterINI.getBool("GENERAL", "usb", false);
	}
	else //Write Default Config
	{
		//0x4C414E47 LANG
		GC_Language_string = GC_Language_strings.at(0);
		//0x56494445 VIDE
		GC_Video_string = GC_Video_strings.at(0);
		//0x424F4F54 BOOT
		DM_Mode_string = DM_Mode_strings.at(2);
		//0x464F5243 FORC
		DM_Patch_string = DM_Patch_strings.at(1);
		//0x44495343 DISC
		DM_NoDisc = true;
		//0x57494445 WIDE
		DM_ForceWide = false;
		//0x52455345 RESE
		DriveReset = false;
		//0x4A415041 JAPA
		NTSCJ_Patch = false;
		//Everything set, lets write it
		WriteConfig(Domain);
	}
}


const char* Menu::GetOption(u32 Option)
{
	switch(Option)
	{
		case 0x4C414E47: //LANG
			return GetOptionFromString(GC_Language_strings, &GC_Language_string, &GC_Language).c_str();
		case 0x56494445: //VIDE
			return GetOptionFromString(GC_Video_strings, &GC_Video_string, &GC_Video_Mode).c_str();
		case 0x424F4F54: //BOOT
			return GetOptionFromString(DM_Mode_strings, &DM_Mode_string, &DM_Mode).c_str();
		case 0x464F5243: //FORC
			return GetOptionFromString(DM_Patch_strings, &DM_Patch_string, &DM_Patch).c_str();
		case 0x52455345: //RESE
			return (DriveReset == true) ? "On" : "Off";
		case 0x4A415041: //JAPA
			return (NTSCJ_Patch == true) ? "Yes" : "No";
		case 0x44495343: //DISC
			return (DM_NoDisc == true) ? "On" : "Off";
		case 0x57494445: //WIDE
			return (DM_ForceWide == true) ? "On" : "Off";
		default:
			if(BooterCFG->Config & Option)
				return "On";
			else
				return "Off";
	}
}

void Menu::SetOption(u32 Option, u8 direction)
{
	switch(Option)
	{
		case 0x4C414E47: //LANG
			if(direction) //Right
				NextOptionFromString(GC_Language_strings, &GC_Language_string, &GC_Language);
			else //Left
				PrevOptionFromString(GC_Language_strings, &GC_Language_string, &GC_Language);
			break;
		case 0x56494445: //VIDE
			if(direction) //Right
				NextOptionFromString(GC_Video_strings, &GC_Video_string, &GC_Video_Mode);
			else //Left
				PrevOptionFromString(GC_Video_strings, &GC_Video_string, &GC_Video_Mode);
			break;
		case 0x424F4F54: //BOOT
			if(direction) //Right
				NextOptionFromString(DM_Mode_strings, &DM_Mode_string, &DM_Mode);
			else //Left
				PrevOptionFromString(DM_Mode_strings, &DM_Mode_string, &DM_Mode);
			break;
		case 0x464F5243: //FORC
			if(direction) //Right
				NextOptionFromString(DM_Patch_strings, &DM_Patch_string, &DM_Patch);
			else //Left
				PrevOptionFromString(DM_Patch_strings, &DM_Patch_string, &DM_Patch);
			break;
		case 0x52455345: //RESE
			DriveReset = (DriveReset == true) ? false : true;
			break;
		case 0x4A415041: //JAPA
			NTSCJ_Patch = (NTSCJ_Patch == true) ? false : true;
			break;
		case 0x44495343: //DISC
			DM_NoDisc = (DM_NoDisc == true) ? false : true;
			break;
		case 0x57494445: //WIDE
			DM_ForceWide = (DM_ForceWide == true) ? false : true;
			break;
		default:
			if(BooterCFG->Config & Option)
				BooterCFG->Config &= ~Option;
			else
				BooterCFG->Config |= Option;
			break;
	}
}
