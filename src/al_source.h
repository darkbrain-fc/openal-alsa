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

#ifndef _AL_SOURCE_H_
#define _AL_SOURCE_H_

#include <alsa/asoundlib.h>

#include <AL/al.h>
#include <AL/alc.h>

#include "al_buffer.h"

typedef struct _AL_queue 
{
	struct _AL_queue *next;
	AL_buffer *buffer;
	ALenum state;
}
AL_queue;

typedef struct _AL_source
{
	ALCcontext *context;

	snd_pcm_t *handle;
	ALint subdev;
	ALuint freq;
	ALuint periods;

	snd_pcm_sframes_t period_size;
	double phase;
	int first;	

	ALenum state;
	ALboolean playing;
	AL_buffer *buffer;
	ALfloat index;

	AL_queue *first_q;
	AL_queue **last_q;
	AL_queue *current_q;

	ALboolean relative;
	ALboolean looping;
	ALboolean conic;

	ALfloat position[3];
	ALfloat direction[3];
	ALfloat velocity[3];

	ALfloat pitch;
	ALfloat gain;
	ALfloat min_gain;
	ALfloat max_gain;
	ALfloat reference_distance;
	ALfloat rolloff_factor;
	ALfloat max_distance;
	ALfloat cone_inner_angle;
	ALfloat cone_outer_angle;
	ALfloat cone_outer_gain;
	ALfloat	volume[8];
	int 	channels;		
}
AL_source;

ALvoid _alDeleteSource(AL_source *);
ALvoid _alProcessSource(AL_source *);

extern ALvoid _alSourceStop(AL_source *src);

#endif
