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
// DESCRIPTION:  none
//
//-----------------------------------------------------------------------------

#include "i_system.h"
#include "timer.h"
#include "fat32.h"

#include "doomfeatures.h"

#include "io.h"
#include "system.h"

#include "doomstat.h"
#include "doomdef.h"

#include "sounds.h"
#include "s_sound.h"

#include "m_random.h"
#include "m_argv.h"

#include "p_local.h"
#include "w_wad.h"
#include "z_zone.h"

#define NORM_PRIORITY 64

#define S_CLOSE_DIST (100 * FRACUNIT)
#define S_CLIP_DIST (1000 * FRACUNIT)

/* Sound stuff */
static sfxinfo_t *sfx_playing;
static mobj_t *sfx_origin;
static unsigned char *sfx_data;
static unsigned int samplerate, length, playpos;

/* Music stuff */
static boolean music_playing, music_looping;
static struct FAT32_FILE music_file;
static unsigned char musicbuf[4096];
static unsigned int musicpos;

// Maximum volume of a sound effect.
// Internal default is max out of 0-15.

int sfxVolume = 8;

// Maximum volume of music. 

int musicVolume = 8;

// Number of channels to use

int numChannels = 8;

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//

void S_Init(int sfxVolume, int musicVolume)
{
	int i;

	S_SetSfxVolume(sfxVolume);
	S_SetMusicVolume(musicVolume);

	sfx_playing = NULL;
	music_playing = false;

	// Note that sounds have not been cached (yet).
	for (i = 1; i < NUMSFX; i++) {
		S_sfx[i].lumpnum = S_sfx[i].usefulness = -1;
	}
}

void S_Shutdown(void)
{
}

//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//

void S_Start(void)
{
	static const char *names[] = {
		"map1-15.pcm",
		"map2-11-17.pcm",
		"map3-21.pcm",
		"map4.pcm",
		"map5-13.pcm",
		"map6-12-24.pcm",
		"map7-19-29.pcm",
		"map8-14-22.pcm",
		"map9.pcm",
		"map10-16.pcm",
		"map2-11-17.pcm",
		"map6-12-24.pcm",
		"map5-13.pcm",
		"map8-14-22.pcm",
		"map1-15.pcm",
		"map10-16.pcm",
		"map2-11-17.pcm",
		"map18-27.pcm",
		"map7-19-29.pcm",
		"map20.pcm",
		"map3-21.pcm",
		"map8-14-22.pcm",
		NULL,
		"map6-12-24.pcm",
		NULL,
		NULL,
		"map18-27.pcm",
		NULL,
		"map7-19-29.pcm",
		NULL,
		NULL,
		NULL
	};

	if (sfx_playing) {
		W_ReleaseLumpNum(sfx_playing->lumpnum);
		sfx_playing = NULL;
	}

	if (gamemode == commercial) {
		S_ChangeMusic(names[gamemap-1], true);
	}
}

void S_StopSound(mobj_t * origin)
{
	if (sfx_playing && sfx_origin == origin) {
		W_ReleaseLumpNum(sfx_playing->lumpnum);
		sfx_playing = NULL;
	}
}

static int S_AdjustSoundPriority(mobj_t * listener, mobj_t * source)
{
	fixed_t approx_dist;
	fixed_t adx;
	fixed_t ady;

	if (!source) return 4;

	// calculate the distance to sound origin
	//  and clip it if necessary
	adx = abs(listener->x - source->x);
	ady = abs(listener->y - source->y);

	// From _GG1_ p.428. Appox. eucledian distance fast.
	approx_dist = adx + ady - ((adx < ady ? adx : ady) >> 1);

	if (approx_dist < S_CLOSE_DIST) {
		return 4;
	} else if (approx_dist < S_CLOSE_DIST*2) {
		return 3;
	} else if (approx_dist > S_CLIP_DIST) {
		return 0;
	}

	return 2;
}


void S_StartSound(void *origin_p, int sfx_id)
{
	char namebuf[15];
	sfxinfo_t *sfx;
	mobj_t *origin;
	int prio, cur_prio, lumplen;

	origin = (mobj_t *) origin_p;

	// check for bogus sound #
	if (sfx_id < 1 || sfx_id > NUMSFX) {
		I_Error("Bad sfx #: %d", sfx_id);
	}

	sfx = &S_sfx[sfx_id];

	if (sfx_playing) {
		/* Kumpi on tärkeämpi ääni? */

		prio = sfx->priority
			* S_AdjustSoundPriority(players[consoleplayer].mo,
				origin);

		cur_prio = sfx_playing->priority
			* S_AdjustSoundPriority(players[consoleplayer].mo,
				sfx_origin);

		if (prio < cur_prio)
			return;

		W_ReleaseLumpNum(sfx_playing->lumpnum);
		sfx_playing = NULL;
	}

	// Get lumpnum if necessary

	if (sfx->lumpnum < 0) {
		strcpy(namebuf, "ds");
		strcat(namebuf, sfx->name);
		sfx->lumpnum = W_GetNumForName(namebuf);
	}

	sfx_playing = sfx;
	sfx_origin = origin;

	sfx_data = W_CacheLumpNum(sfx->lumpnum, PU_STATIC);
	lumplen = W_LumpLength(sfx->lumpnum);

	// Check the header, and ensure this is a valid sound

	if (lumplen < 8 || sfx_data[0] != 0x03 || sfx_data[1] != 0x00) {
		// Invalid sound
		I_Error("Invalid sound");
	}

	// 16 bit sample rate field, 32 bit length field

	playpos = 8;
	samplerate = (sfx_data[3] << 8) | sfx_data[2];
	length = 8 + ((sfx_data[7] << 24) | (sfx_data[6] << 16) |
			(sfx_data[5] << 8) | sfx_data[4]);

	if (length > lumplen) {
		I_Error("Invalid sound");
	}
}

void S_FeedAudio(void)
{
	unsigned int len, towrite;
	int nowtime;
	static int prevtime = 0;
	int sample;

	nowtime = get_ticks();
	if (prevtime)
		towrite = (nowtime - prevtime) * 11025 / HZ;
	else
		towrite = 0;
	prevtime = nowtime;
	if (!towrite)
		return;

	if (towrite > 1000)
		towrite = 1000;

	/* Not pretty but fast! */

	if (music_playing) {
		if (musicpos + towrite > sizeof(musicbuf)) {
			memcpy(musicbuf, &musicbuf[musicpos],
			       sizeof(musicbuf) - musicpos);

			/* read some music.. */
			len = fat32_read(&musicbuf[sizeof(musicbuf) - musicpos],
				         musicpos, &music_file);
			if (len < musicpos) {
				/* fixme.. */
				fat32_seek(0, &music_file);
				if (!music_looping)
					music_playing = false;
			}
			musicpos = 0;
		}

		if (sfx_playing) {
			len = length - playpos;
			if (len > towrite)
				len = towrite;

			towrite -= len;
			/* music + sfx: slowest case! */
			while (len--) {
				sample = (((int)sfx_data[playpos] - 128) << 6) +
					(((int)musicbuf[musicpos] - 128) << 6);
				IOWR(AUDIO_FIFO, 0, (sample & 0xffff) | (sample << 16));
				IOWR(AUDIO_FIFO, 0, (sample & 0xffff) | (sample << 16));
				IOWR(AUDIO_FIFO, 0, (sample & 0xffff) | (sample << 16));
				IOWR(AUDIO_FIFO, 0, (sample & 0xffff) | (sample << 16));
				playpos++;
				musicpos++;
			}

			if (playpos == length) {
				W_ReleaseLumpNum(sfx_playing->lumpnum);
				sfx_playing = NULL;
			}
		}

		/* just music */
		while (towrite--) {
			sample = ((int)musicbuf[musicpos] - 128) << 6;
			IOWR(AUDIO_FIFO, 0, (sample & 0xffff) | (sample << 16));
			IOWR(AUDIO_FIFO, 0, (sample & 0xffff) | (sample << 16));
			IOWR(AUDIO_FIFO, 0, (sample & 0xffff) | (sample << 16));
			IOWR(AUDIO_FIFO, 0, (sample & 0xffff) | (sample << 16));
			musicpos++;
		}

	} else {
		/* NO MUSIC */

		if (sfx_playing) {
			len = length - playpos;
			if (len > towrite)
				len = towrite;

			towrite -= len;
			while (len--) {
				sample = ((int)sfx_data[playpos] - 128) << 6;
				IOWR(AUDIO_FIFO, 0, (sample & 0xffff) | (sample << 16));
				IOWR(AUDIO_FIFO, 0, (sample & 0xffff) | (sample << 16));
				IOWR(AUDIO_FIFO, 0, (sample & 0xffff) | (sample << 16));
				IOWR(AUDIO_FIFO, 0, (sample & 0xffff) | (sample << 16));
				playpos++;
			}

			if (playpos == length) {
				W_ReleaseLumpNum(sfx_playing->lumpnum);
				sfx_playing = NULL;
			}
		}

		/* silence */
		while (towrite--) {
			IOWR(AUDIO_FIFO, 0, 0);
			IOWR(AUDIO_FIFO, 0, 0);
			IOWR(AUDIO_FIFO, 0, 0);
			IOWR(AUDIO_FIFO, 0, 0);
		}
	}
}

//
// Stop and resume music, during game PAUSE.
//

void S_PauseSound(void)
{
}

void S_ResumeSound(void)
{
}

//
// Updates music & sounds
//

void S_UpdateSounds(mobj_t * listener)
{
}

void S_SetMusicVolume(int volume)
{
	if (volume < 0 || volume > 127) {
		I_Error("Attempt to set music volume at %d", volume);
	}
}

void S_SetSfxVolume(int volume)
{
	if (volume < 0 || volume > 127) {
		I_Error("Attempt to set sfx volume at %d", volume);
	}
}

void S_ChangeMusic(const char *name, boolean looping)
{
	struct FAT32_DIR dir;

	music_playing = false;
	music_looping = looping;

	if (!name) return;

	I_Print("playing: ");
	I_Print(name);
	I_Print("\n");

	if (fat32_opendir("/", &dir))
		return;

	if (fat32_fopen(&dir, name, &music_file))
		return;

	fat32_read(musicbuf, sizeof(musicbuf), &music_file);

	music_playing = true;
	musicpos = 0;
}

boolean S_MusicPlaying(void)
{
	return false;
}

