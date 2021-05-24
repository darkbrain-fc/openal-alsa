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

#ifndef _ALC_CONTEXT_H_
#define _ALC_CONTEXT_H_

#include <pthread.h>
#include <semaphore.h>

#include <AL/al.h>
#include <AL/alc.h>

#include "alc_speaker.h"
#include "alc_device.h"
#include "al_source.h"
#include "al_listener.h"

typedef struct _AL_context
{
	ALCdevice *device;
	AL_source **sources;

	pthread_t thread;
	pthread_mutex_t mutex;
	pthread_cond_t cond;

	AL_listener listener;
	AL_speaker speakers[_ALC_NUM_SPEAKERS];

	ALfloat doppler_factor;
	ALfloat doppler_velocity;
	ALenum distance_model;
	ALfloat (*distance_func)(AL_source *, ALfloat);
}
AL_context;

extern AL_context *_alcCurrentContext;

#define _alcLockContext(ctx) pthread_mutex_lock(&ctx->mutex)
#define _alcUnlockContext(ctx) pthread_mutex_unlock(&ctx->mutex)

AL_source *_alFindSource(AL_context *, ALuint);

ALfloat _alDistanceInverse(AL_source *, ALfloat);

#endif
