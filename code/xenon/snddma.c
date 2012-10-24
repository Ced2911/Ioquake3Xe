/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// snddma_null.c
// all other sound mixing is portable

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "../client/snd_local.h"

static int snd_inited = 0;

qboolean SNDDMA_Init(void)
{
	int dmasize = 0;
	
	if (snd_inited)
		return qtrue;
	
	// signed 16bit
	dma.samplebits = 0x10;  // first byte of format is bits.
	dma.channels = 2;
	dma.samples = 32768;
	dma.submission_chunk = 1;
	dma.speed = 48000;
	dmasize = (dma.samples * (dma.samplebits/8));
	dma.buffer = calloc(1, dmasize);
	
	snd_inited = 1;
	
	return qtrue;
}

int	SNDDMA_GetDMAPos(void)
{
	return 0;
}

void SNDDMA_Shutdown(void)
{
}

void SNDDMA_BeginPainting (void)
{
}

void SNDDMA_Submit(void)
{
}
