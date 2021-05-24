/*
 *  Copyright (C) 2004 Christopher John Purnell
 *                     cjp@lost.org.uk
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _ALC_DEVICE_H_
#define _ALC_DEVICE_H_

#include <alsa/asoundlib.h>

#include <AL/al.h>
#include <AL/alc.h>

#include "al_source.h"

struct _AL_device
{
	char device[16];
	ALuint subdevs;
/*	snd_ctl_t *ctl;*/
	ALuint count;	
	ALCboolean sync;
	ALuint freq;
	ALuint refresh;
	ALuint channels;
};

ALCboolean _alcOpenSource(AL_source *);
ALCvoid _alcCloseSource(AL_source *);

#endif