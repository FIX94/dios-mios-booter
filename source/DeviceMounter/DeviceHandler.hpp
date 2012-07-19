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
#ifndef DEVICE_HANDLER_HPP_
#define DEVICE_HANDLER_HPP_

#include "PartitionHandle.h"

/**
 * libogc device names.
 */
enum
{
	SD = 0,
	USB,
	MAXDEVICES
};

/**
 * libogc device names.
 */
const char DeviceName[MAXDEVICES][8] =
{
	"sd",
	"usb",
};

class DeviceHandler
{
	public:
		DeviceHandler();
		~DeviceHandler();

		//! Individual Mounts/UnMounts...
		bool MountSD();
		bool MountUSB();

		bool SD_Inserted() { if(sd) return sd->IsInserted(); return false; }
		bool USB_Inserted() { if(usb) return usb->IsInserted(); return false; }

		bool SD_Mounted() { if(sd) return sd->IsMounted(); return false; }
		bool USB_Mounted() { if(usb) return usb->IsMounted(); return false; }

		void UnMountSD() { if(sd) delete sd; sd = NULL; }
		void UnMountUSB() { if(usb) delete usb; usb = NULL; }

		PartitionHandle * GetSDHandle() const { return sd; }
		PartitionHandle * GetUSBHandle() const { return usb; }

		const DISC_INTERFACE *GetUSBInterface() { return &__io_usbstorage; }
		u16 GetUSBPartitionCount();
	private:
		PartitionHandle *sd;
		PartitionHandle *usb;
};

extern DeviceHandler *DevHandler;

#endif
