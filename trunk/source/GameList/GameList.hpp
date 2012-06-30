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
#ifndef GAMELIST_HPP_
#define GAMELIST_HPP_

#include <gccore.h>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

struct DirEnt
{
	string Path;
	string Name;
	string GameID;
};

class GameList
{
public:
	void AddEntry(string path, string gameid, string name);
	void ClearEntries();
	void SortEntries();
	const char *GetEntryPath(u32 position);
	const char *GetEntryName(u32 position);
	const char *GetEntryID(u32 position);
	u32 GetEntrySize();
private:
	vector<DirEnt> DirEntries;
};

#endif
