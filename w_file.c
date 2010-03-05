// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2008 Simon Howard
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
//      WAD I/O functions.
//
//-----------------------------------------------------------------------------

#include "fat32.h"
#include "doomdef.h"
#include "doomtype.h"
#include "m_argv.h"

#include "w_file.h"

#include "z_zone.h"

wad_file_t *W_OpenFile(char *path)
{
	struct FAT32_DIR dir;
	struct FAT32_FILE *f;

	if (fat32_opendir("/", &dir))
		return NULL;

	f = Z_Malloc(sizeof(*f), PU_STATIC, NULL);

	if (fat32_fopen(&dir, path, f))
		return NULL;

	return f;
}

void W_CloseFile(wad_file_t *f)
{
	Z_Free(f);
}

size_t W_Read(wad_file_t *f, unsigned int offset,
	      void *buffer, size_t buffer_len)
{
	fat32_seek(offset, f);
	return fat32_read(buffer, buffer_len, f);
}
