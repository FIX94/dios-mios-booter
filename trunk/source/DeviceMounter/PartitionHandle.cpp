 /****************************************************************************
 * Copyright (C) 2010
 * by Dimok
 * modified for Debugging, GPT, WBFS, and EXT by Miigotu
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
 * By Dimok for WiiXplorer 2010
 * By Miigotu for WiiFlow 2010
 ***************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "PartitionHandle.h"
#include "Memory/mem2.hpp"
#include "fat.h"

#define PARTITION_TYPE_DOS33_EXTENDED		0x05 /* DOS 3.3+ extended partition */
#define PARTITION_TYPE_WIN95_EXTENDED		0x0F /* Windows 95 extended partition */
#define PARTITION_TYPE_GPT_TABLE			0xEE /* New Standard */

#define CACHE 32
#define SECTORS 64

extern u32 sector_size;

static inline const char * PartFromType(int type)
{
	switch (type)
	{
		case 0x00: return "Unused"; //Or WBFS
		case 0x01: return "FAT12";
		case 0x04: return "FAT16";
		case 0x05: return "Extended";
		case 0x06: return "FAT16";
		case 0x07: return "NTFS";
		case 0x0b: return "FAT32";
		case 0x0c: return "FAT32";
		case 0x0e: return "FAT16";
		case 0x0f: return "Extended";
		case 0x82: return "LxSWP";
		case 0x83: return "LINUX";
		case 0x8e: return "LxLVM";
		case 0xa8: return "OSX";
		case 0xab: return "OSXBT";
		case 0xaf: return "OSXHF";
		case 0xe8: return "LUKS";
		case 0xee: return "GPT";
		default: return "Unknown";
	}
}

PartitionHandle::PartitionHandle(const DISC_INTERFACE *discio)
{
	interface = discio;

	if(!interface)
		return;

	time_t start = time(0);
	while(time(0) - start < 10) // 10 sec
	{
		if(interface->startup() && interface->isInserted())
			break;
		usleep(200000); // 1/5 sec
	}
	if(!interface->startup() || !interface->isInserted())
		return;

	FindPartitions();
}

bool PartitionHandle::IsMounted()
{
	if(MountName.size() != 0)
		return true;
	return false;
}

bool PartitionHandle::MountFAT(const char *name)
{
	if(!name)
		return false;

	MountName = name;
	for(u32 pos = 0; pos < PartitionList.size(); pos++)
	{
		if(strncmp(GetFSName(pos), "FAT", 3) == 0)
		{
			if(fatMount(name, interface, GetLBAStart(pos), CACHE, SECTORS))
				return true;
		}
	}
	MountName.clear();

	return false;
}

void PartitionHandle::UnMount()
{
	if(!interface)
		return;

	if(MountName.size() == 0)
		return;

	char DeviceName[20];
	snprintf(DeviceName, sizeof(DeviceName), "%s:", MountName.c_str());

	fatUnmount(DeviceName);

	MountName.clear();
}

int PartitionHandle::FindPartitions()
{
	MASTER_BOOT_RECORD *mbr = (MASTER_BOOT_RECORD *)MEM2_alloc(MAX_BYTES_PER_SECTOR);
	if(mbr == NULL)
		return -1;

	// Read the first sector on the device
	if(!interface->readSectors(0, 1, mbr)) 
	{
		MEM2_free(mbr);
		return 0;
	}

	// Check if it's a RAW disc, without a partition table
	if(CheckRAW((VOLUME_BOOT_RECORD *)mbr)) 
	{
		MEM2_free(mbr);
		return 1;
	}
	// Verify this is the device's master boot record
	if(mbr->signature != MBR_SIGNATURE) 
	{
		MEM2_free(mbr);
		return 0;
	}

	for(int i = 0; i < 4; i++)
	{
		PARTITION_RECORD * partition = (PARTITION_RECORD *)&mbr->partitions[i];
		VOLUME_BOOT_RECORD *vbr = (VOLUME_BOOT_RECORD *)MEM2_alloc(MAX_BYTES_PER_SECTOR);
		if(!vbr) 
		{
			MEM2_free(mbr);
			return -1;
		}

		if(le32(partition->lba_start) == 0)
			continue; // Invalid partition

		if(!interface->readSectors(le32(partition->lba_start), 1, vbr))
			continue;

		if(i == 0 && partition->type == PARTITION_TYPE_GPT_TABLE)
			return CheckGPT() ? PartitionList.size() : 0;

		if(vbr->Signature != VBR_SIGNATURE && partition->type != 0x83)
			continue;

		if(partition->type == PARTITION_TYPE_DOS33_EXTENDED || partition->type == PARTITION_TYPE_WIN95_EXTENDED)
		{
			CheckEBR(i, le32(partition->lba_start));
			continue;
		}

		if(le32(partition->block_count) > 0)
		{
			PartitionFS PartitionEntry = {"0",0,0,0,0,0,0};
			PartitionEntry.FSName = PartFromType(partition->type);
			PartitionEntry.LBA_Start = le32(partition->lba_start);
			PartitionEntry.SecCount = le32(partition->block_count);
			PartitionEntry.Bootable = (partition->status == PARTITION_BOOTABLE);
			PartitionEntry.PartitionType = partition->type;
			PartitionEntry.PartitionNum = i;
			PartitionEntry.EBR_Sector = 0;
			PartitionList.push_back(PartitionEntry);
		}
		MEM2_free(vbr);
	}
	MEM2_free(mbr);
	return PartitionList.size();
}

void PartitionHandle::CheckEBR(u8 PartNum, sec_t ebr_lba)
{
	EXTENDED_BOOT_RECORD *ebr = (EXTENDED_BOOT_RECORD *)MEM2_alloc(MAX_BYTES_PER_SECTOR);
	if(ebr == NULL)
		return;
	sec_t next_erb_lba = 0;

	do
	{
		// Read and validate the extended boot record
		if(!interface->readSectors(ebr_lba + next_erb_lba, 1, ebr)) 
		{
			MEM2_free(ebr);
			return;
		}
		if(ebr->signature != EBR_SIGNATURE) 
		{
			MEM2_free(ebr);
			return;
		}
		if(le32(ebr->partition.block_count) > 0)
		{
			PartitionFS PartitionEntry = {"0",0,0,0,0,0,0};
			PartitionEntry.FSName = PartFromType(ebr->partition.type);
			PartitionEntry.LBA_Start = ebr_lba + next_erb_lba + le32(ebr->partition.lba_start);
			PartitionEntry.SecCount = le32(ebr->partition.block_count);
			PartitionEntry.Bootable = (ebr->partition.status == PARTITION_BOOTABLE);
			PartitionEntry.PartitionType = ebr->partition.type;
			PartitionEntry.PartitionNum = PartNum;
			PartitionEntry.EBR_Sector = ebr_lba + next_erb_lba;
			PartitionList.push_back(PartitionEntry);
		}
		// Get the start sector of the current partition
		// and the next extended boot record in the chain
		next_erb_lba = le32(ebr->next_ebr.lba_start);
	}
	while(next_erb_lba > 0);
	MEM2_free(ebr);
}

bool PartitionHandle::CheckGPT(void)
{
	GPT_PARTITION_TABLE *gpt = (GPT_PARTITION_TABLE *)MEM2_alloc(MAX_BYTES_PER_SECTOR);
	if(gpt == NULL)
		return false;
	bool success = false; // To return false unless at least 1 partition is verified

	if(!interface->readSectors(1, 33, gpt))
	{
		MEM2_free(gpt);
		return false;	// To read all 128 possible partitions
	}

	// Verify this is the Primary GPT entry
	if((strncmp(gpt->magic, GPT_SIGNATURE, 8) != 0) 
			|| (le32(gpt->Entry_Size) != 128) 
			|| (le64(gpt->Table_LBA) != 2)
			|| (le64(gpt->Header_LBA) != 1)
			|| (le64(gpt->First_Usable_LBA) != 34)
			|| (gpt->Reserved != 0))
	{
		MEM2_free(gpt);
		return false;
	}

	for(u8 i = 0; i < le32(gpt->Num_Entries) && PartitionList.size() <= 8; i++)
	{
		GUID_PARTITION_ENTRY *entry = (GUID_PARTITION_ENTRY *) &gpt->partitions[i];
		VOLUME_BOOT_RECORD *vbr = (VOLUME_BOOT_RECORD*)MEM2_alloc(MAX_BYTES_PER_SECTOR);
		if(vbr == NULL)
		{
			MEM2_free(gpt);
			return false;
		}

		int Start = le64(entry->First_LBA);

		if(!interface->readSectors(Start, 1, vbr))
			continue;

		PartitionFS PartitionEntry = {"0",0,0,0,0,0,0};
		if(memcmp((u8 *)vbr + BPB_FAT32_ADDR, FAT_SIGNATURE, sizeof(FAT_SIGNATURE)) == 0)
		{
			PartitionEntry.FSName = "FAT32";
			PartitionEntry.PartitionType = 0x0c;
			PartitionEntry.SecCount = le16(vbr->bpb.FatSectors);
			if (PartitionEntry.SecCount == 0)
				PartitionEntry.SecCount = le32(vbr->bpb.Large_Sectors);
		}
		else if(memcmp((u8 *)vbr + BPB_FAT16_ADDR, FAT_SIGNATURE, sizeof(FAT_SIGNATURE)) == 0)
		{
			PartitionEntry.FSName = "FAT16";
			PartitionEntry.PartitionType = 0x0e;
			PartitionEntry.SecCount = le16(vbr->bpb.FatSectors);
			if (PartitionEntry.SecCount == 0)
				PartitionEntry.SecCount = le32(vbr->bpb.Large_Sectors);
		}
		if(PartitionEntry.SecCount != 0 && PartitionEntry.FSName[0] != '0')
		{
			PartitionEntry.LBA_Start = Start;
			PartitionEntry.PartitionNum = i;
			success = true;
			PartitionList.push_back(PartitionEntry);
		}
		MEM2_free(vbr);
	}
	MEM2_free(gpt);

	return success;
}

bool PartitionHandle::CheckRAW(VOLUME_BOOT_RECORD * vbr)
{
	PartitionFS PartitionEntry = {"0",0,0,0,0,0,0};
	if(memcmp((u8 *)vbr + BPB_FAT32_ADDR, FAT_SIGNATURE, sizeof(FAT_SIGNATURE)) == 0)
	{
		PartitionEntry.FSName = "FAT32";
		PartitionEntry.PartitionType = 0x0c;
		PartitionEntry.SecCount = le16(vbr->bpb.FatSectors);
		if (PartitionEntry.SecCount == 0)
			PartitionEntry.SecCount = le32(vbr->bpb.Large_Sectors);
	}
	else if(memcmp((u8 *)vbr + BPB_FAT16_ADDR, FAT_SIGNATURE, sizeof(FAT_SIGNATURE)) == 0)
	{
		PartitionEntry.FSName = "FAT16";
		PartitionEntry.PartitionType = 0x0e;
		PartitionEntry.SecCount = le16(vbr->bpb.FatSectors);
		if (PartitionEntry.SecCount == 0)
			PartitionEntry.SecCount = le32(vbr->bpb.Large_Sectors);
	}
	if(PartitionEntry.FSName[0] != '0')
	{
		PartitionList.push_back(PartitionEntry);
		return true;
	}
	return false;
}


u64 PartitionHandle::le64(u64 x)
{
 	return 
	((x & 0xFF00000000000000) >> 56) |
	((x & 0x00FF000000000000) >> 40) |
	((x & 0x0000FF0000000000) >> 24) |
	((x & 0x000000FF00000000) >> 8 ) |
	((x & 0x00000000FF000000) << 8 ) |
	((x & 0x0000000000FF0000) << 24) |
	((x & 0x000000000000FF00) << 40) |
	((x & 0x00000000000000FF) << 56);
}

u32 PartitionHandle::le32(u32 x)
{
	return
	((x & 0x000000FF) << 24) |
	((x & 0x0000FF00) << 8) |
	((x & 0x00FF0000) >> 8) |
	((x & 0xFF000000) >> 24);
}

u16 PartitionHandle::le16(u16 x)
{
	return
	((x & 0x00FF) << 8) |
	((x & 0xFF00) >> 8);
}
