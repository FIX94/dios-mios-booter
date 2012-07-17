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
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef SYS_H_
#define SYS_H_

s32 MagicPatches(s32);

void Sys_Init(void);
bool Sys_Exit(void);

void Open_Inputs(void);
void Close_Inputs(void);
void Input_Reset(void);

#endif //SYS_H_

#ifdef __cplusplus
}
#endif
