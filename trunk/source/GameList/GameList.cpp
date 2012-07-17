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
#include <stdio.h>
#include <string.h>
#include <algorithm>

#include "GameList.hpp"

const char* emptyChar = (const char*)" ";

void GameList::AddEntry(string path, string gameid, string name)
{
	DirEnt entry;
	entry.Path = path;
	entry.GameID = gameid;
	entry.Name = name;
	DirEntries.push_back(entry);
}

void GameList::ClearEntries()
{
	DirEntries.clear();
}

static bool SortCompare(const DirEnt& One, const DirEnt& Two)
{
	return One.Name < Two.Name;
}

void GameList::SortEntries()
{
	if(DirEntries.size() > 2)
		sort(DirEntries.begin()+1, DirEntries.end(), SortCompare); //+1 for dvd as first
}

u32 GameList::GetEntrySize()
{
	return DirEntries.size();
}

const char *GameList::GetEntryPath(u32 position)
{
	if(position < DirEntries.size())
		return DirEntries.at(position).Path.c_str();
	return emptyChar;
}

const char *GameList::GetEntryName(u32 position)
{
	if(position < DirEntries.size())
		return DirEntries.at(position).Name.c_str();
	return emptyChar;
}

const char *GameList::GetEntryID(u32 position)
{
	if(position < DirEntries.size())
		return DirEntries.at(position).GameID.c_str();
	return emptyChar;
}
