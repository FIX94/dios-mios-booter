/****************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * for WiiXplorer 2010
 ***************************************************************************/

#include <unistd.h>
#include <string.h>
#include <ogc/mutex.h>
#include <ogc/system.h>
#include <sdcard/gcsd.h>
#include <sdcard/wiisd_io.h>

#include "DeviceHandler.hpp"

void DeviceHandler::Init()
{
	sd = NULL;
	usb = NULL;
}

bool DeviceHandler::MountSD()
{
	if(sd == NULL)
	{
		sd = new PartitionHandle(&__io_wiisd);
		if(sd->GetPartitionCount() < 1)
		{
			sd = NULL;
			return false;
		}
		return sd->MountFAT(DeviceName[SD]);
	}
	return false;
}

void DeviceHandler::UnMountSD()
{
	if(sd != NULL)
		sd->UnMount();
}

bool DeviceHandler::MountUSB()
{
	if(usb == NULL)
	{
		usb = new PartitionHandle(&__io_usbstorage);
		if(usb->GetPartitionCount() < 1)
		{
			usb = NULL;
			return false;
		}
		return usb->MountFAT(DeviceName[USB]);
	}
	return false;
}

void DeviceHandler::UnMountUSB()
{
    if(usb != NULL)
		usb->UnMount();
}
