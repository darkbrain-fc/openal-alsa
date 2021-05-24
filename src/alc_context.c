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

#include <sys/time.h>
#include <stdlib.h>
#include <ansidecl.h>

#include "al_listener.h"
#include "al_source.h"
#include "alc_context.h"
#include "alc_error.h"

AL_context *_alcCurrentContext = 0;

static ALCvoid *_alcThread(ALCcontext *cc)
{
	AL_context *ctx = cc;
	ALCdevice *dev = ctx->device;
	ALuint i;
	struct timeval tv;
	struct timespec ts;
/*	long ns = 1000000000 / dev->refresh;*/
	int ns = 1000000000 / dev->refresh;


	pthread_mutex_lock(&ctx->mutex);
	do
	{
		gettimeofday(&tv, 0);

		for (i = 0; i < dev->subdevs; i++)
		{
			AL_source *src;

			if ((src = ctx->sources[i]))
			{
				_alProcessSource(src);
			}
		}

		ts.tv_sec = tv.tv_sec;
		if ((ts.tv_nsec = (tv.tv_usec * 1000) + ns) >= 1000000000)
		{
			ts.tv_sec += 1;
			ts.tv_nsec -= 1000000000;
		}
	}
	while (pthread_cond_timedwait(&ctx->cond, &ctx->mutex, &ts));
	pthread_mutex_unlock(&ctx->mutex);


	return 0;
}

static ALCboolean _alcCreateContext(AL_context *ctx)
{
	ALCdevice *dev = ctx->device;
	ALuint i;

	pthread_mutex_init(&ctx->mutex, 0);

	if (!(ctx->sources = malloc(dev->subdevs * sizeof(AL_source*))))
	{
		return AL_FALSE;
	}

	for (i = 0; i < dev->subdevs; i++)
	{
		ctx->sources[i] = 0;
	}

	if (!dev->sync)
	{

		pthread_cond_init(&ctx->cond, 0);

		if (pthread_create(&ctx->thread, 0, _alcThread, ctx))
		{
			return AL_FALSE;
		}
	}

	return ALC_TRUE;
}

static ALCvoid _alcDestroyContext(AL_context *ctx)
{
	ALCdevice *dev;
	ALuint i;

	dev = ctx->device;

	for (i = 0; i < dev->subdevs; i++)
	{
		AL_source *src;

		if ((src = ctx->sources[i]))
		{
			_alDeleteSource(src);
		}
	}

	if (ctx->thread)
	{
		pthread_cond_signal(&ctx->cond);

		pthread_join(ctx->thread, 0);

		pthread_cond_destroy(&ctx->cond);
	}

	if (ctx->sources)
	{
		free(ctx->sources);
	}

	pthread_mutex_destroy(&ctx->mutex);

	free(ctx);
}

ALCcontext *alcCreateContext(ALCdevice *dev, ALCint *attrlist)
{
	AL_context *ctx;

	if (!dev)
	{
		_alcSetError(ALC_INVALID_DEVICE);
		return 0;
	}

	if (!(ctx = malloc(sizeof(AL_context))))
	{
		_alcSetError(ALC_OUT_OF_MEMORY);
		return 0;
	}

	ctx->device = dev;

	while (attrlist)
	{
		ALCint value;

		switch (*(attrlist++))
		{
		case ALC_FREQUENCY:
			if ((value = *(attrlist++)) > 0)
			{
				dev->freq = value;
			}
			break;
		case ALC_REFRESH:
			if ((value = *(attrlist++)) > 0)
			{
				fprintf(stderr,"alcCreateContext refresh : %d",value);
				dev->refresh = value;
			}
			break;
		case ALC_SYNC:
			dev->sync = *(attrlist++) ? AL_TRUE: AL_FALSE;
			break;
		default:
			attrlist = 0;
			break;
		}
	}

	if (dev->refresh > dev->freq)
	{
		dev->refresh = dev->freq;
	}

	ctx->sources = 0;
	ctx->thread = 0;

	_alcLoadSpeakers(ctx->speakers);
	_alInitListener(&ctx->listener, ctx->speakers);

	ctx->doppler_factor = 1.0f;
	ctx->doppler_velocity = 1.0f;
	ctx->distance_model = AL_INVERSE_DISTANCE;
	ctx->distance_func = _alDistanceInverse;

	if (_alcCreateContext(ctx))
		return ctx;

	_alcDestroyContext(ctx);

	_alcSetError(ALC_OUT_OF_MEMORY);

	return 0;
}

ALCenum alcDestroyContext(ALCcontext *cc)
{
	AL_context *ctx;

	if (!(ctx = cc))
	{
		_alcSetError(ALC_INVALID_CONTEXT);
		return ALC_INVALID_CONTEXT;
	}

	if (ctx == _alcCurrentContext)
	{
		_alcCurrentContext = 0;
	}

	_alcDestroyContext(ctx);

	return ALC_NO_ERROR;
}

ALCenum alcMakeContextCurrent(ALCcontext *cc)
{
	_alcCurrentContext = cc;

	return ALC_NO_ERROR;
}

ALCcontext *alcGetCurrentContext(ALCvoid)
{
	return _alcCurrentContext;
}

ALCdevice *alcGetContextsDevice(ALCcontext *cc)
{
	AL_context *ctx;

	if (!(ctx = cc))
	{
		_alcSetError(ALC_INVALID_CONTEXT);
		return 0;
	}

	return ctx->device;
}

ALCvoid alcSuspendContext(ALCcontext *cc ATTRIBUTE_UNUSED)
{
	/* FIXME */
}

ALCvoid *alcProcessContext(ALCcontext *cc)
{
	AL_context *ctx;
	ALCdevice *dev;
	ALuint i;

	if (!(ctx = cc))
	{
		_alcSetError(ALC_INVALID_CONTEXT);
		return 0;
	}

	if (ctx->thread)
	{
		return cc;
	}

	_alcLockContext(ctx);

	dev = ctx->device;

	for (i = 0; i < dev->subdevs; i++)
	{
		AL_source *src;

		if ((src = ctx->sources[i]))
		{
			_alProcessSource(src);
		}
	}

	_alcUnlockContext(ctx);

	return cc;
}
