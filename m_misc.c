// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// DESCRIPTION:
//      Miscellaneous.
//
//-----------------------------------------------------------------------------

#include "ctype.h"

#include "fat32.h"
#include "doomdef.h"
#include "doomstat.h"

#include "i_swap.h"
#include "i_system.h"
#include "i_video.h"
#include "m_misc.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

// Check if a file exists

boolean M_FileExists(char *filename)
{
	struct FAT32_DIR dir;

	if (fat32_opendir("/", &dir))
		return false;

	if (fat32_fopen(&dir, filename, NULL))
		return false;

	return true;
}

//
// M_WriteFile
//

boolean M_WriteFile(char *name, void *source, int length)
{
#if 0
	FILE *handle;
	int count;

	handle = fopen(name, "wb");

	if (handle == NULL)
		return false;

	count = fwrite(source, 1, length, handle);
	fclose(handle);

	if (count < length)
		return false;

	return true;
#endif
	return false;
}

//
// M_ReadFile
//

int M_ReadFile(char *name, byte ** buffer)
{
#if 0
	FILE *handle;
	int count, length;
	byte *buf;

	handle = fopen(name, "rb");
	if (handle == NULL)
		I_Error("Couldn't read file %s", name);

	// find the size of the file by seeking to the end and
	// reading the current position

	length = M_FileLength(handle);

	buf = Z_Malloc(length, PU_STATIC, NULL);
	count = fread(buf, 1, length, handle);
	fclose(handle);

	if (count < length)
		I_Error("Couldn't read file %s", name);

	*buffer = buf;
	return length;
#endif
	I_Error("TODO");
}
