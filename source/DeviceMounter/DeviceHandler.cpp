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
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <ogc/mutex.h>
#include <ogc/system.h>
#include <sdcard/gcsd.h>
#include <sdcard/wiisd_io.h>
#include "DeviceHandler.hpp"

DeviceHandler *DevHandler;

DeviceHandler::DeviceHandler()
{
	sd = NULL;
	usb = NULL;
}

DeviceHandler::~DeviceHandler()
{
	UnMountSD();
	UnMountUSB();
}

bool DeviceHandler::MountSD()
{
    if(!sd)
	{
		sd = new PartitionHandle(&__io_wiisd);
		if(sd->GetPartitionCount() < 1)
		{
			delete sd;
			sd = NULL;
			return false;
		}
    }

	//! Mount only one SD Partition
	return sd->Mount(0, DeviceName[SD], true);
}

bool DeviceHandler::MountUSB()
{
	bool deviceAvailable = false;
	u8 timeout = 0;
	const DISC_INTERFACE *handle = GetUSBInterface();
	while(!deviceAvailable && timeout++ != 20)
	{
		deviceAvailable = (handle->startup() && handle->isInserted());
		if(deviceAvailable)
			break;
		usleep(50000);
	}
	usb = new PartitionHandle(handle);
	if(usb->GetPartitionCount() < 1)
	{
		delete usb;
		usb = NULL;
		return false;
	}

	int partCount = GetUSBPartitionCount();
	for(int i = 0; i < partCount; i++)
	{
		if(usb->Mount(i, DeviceName[USB], true))
			return true;
	}

	return false;
}

u16 DeviceHandler::GetUSBPartitionCount()
{
	u16 partCount = 0;
	if(usb)
		partCount = usb->GetPartitionCount();
	return partCount;
}
