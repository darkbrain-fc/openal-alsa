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
#include <math.h>
#include <alloca.h>
#include <string.h>

#include "al_source.h"
#include "al_error.h"
#include "al_vector.h"
#include "alc_device.h"
#include "alc_context.h"


static ALfloat _alCalculateGainAndPitch(AL_source *src)
{
	AL_context *ctx = src->context;
	ALCdevice *dev = ctx->device;
	ALfloat position[3];
	ALfloat gain = ctx->listener.gain;	
	

	position[0] = src->position[0];
	position[1] = src->position[1];
	position[2] = src->position[2];

	if (!src->relative)
	{
		position[0] -= ctx->listener.position[0];
		position[1] -= ctx->listener.position[1];
		position[2] -= ctx->listener.position[2];
	}

	/* Master Gain */
	{
		ALfloat dist = _alVectorMagnitude(position);

		gain *= ctx->distance_func(src, dist);

		if (dist)
		{
			position[0] /= dist;
			position[1] /= dist;
			position[2] /= dist;
		}

		if (src->conic)
		{
			ALfloat a;

			a = _alVectorDotProduct(position, src->direction);

			a = acos(-a) * 360.0 / M_PI;

			if (a > src->cone_inner_angle)
			{
				if (a >= src->cone_outer_angle)
				{
					gain *= src->cone_outer_gain;
				}
				else
				{
					a -= src->cone_inner_angle;
					a *= (src->cone_outer_gain - 1.0f);
					a /= (src->cone_outer_angle -
					      src->cone_inner_angle);
					gain *= (1.0f + a);
				}
			}
		}

		if (gain > src->max_gain)
		{
			gain = src->max_gain;
		}
		else if (gain < src->min_gain)
		{
			gain = src->min_gain;
		}		
	}

	/* Speaker Gains */
	{
		ALuint i;

		for (i = 0; i < src->channels; i++)
		{
			AL_speaker *speaker = &ctx->listener.speakers[i];

			src->volume[i] = ( gain * ((_alVectorDotProduct(position, 
						 			speaker->position) + 1.0f)
						  			* 0.5f * speaker->gain));
		}
	}

	/* Pitch */
	{
		ALfloat vl, vs;
		ALfloat pitch = src->pitch;

		if (ctx->doppler_factor)
		{
			vl = _alVectorDotProduct(ctx->listener.velocity,
						 position);
			vs = _alVectorDotProduct(src->velocity, position);

			vl *= ctx->doppler_factor;
			vs *= ctx->doppler_factor;

			vl += ctx->doppler_velocity;
			vs += ctx->doppler_velocity;

			pitch *= vl / vs;
		}

		if (pitch < 0.0f)
		{
			pitch = 0.0f;
		}

		return pitch;
	}
}

snd_pcm_uframes_t _alWriteData(AL_source *src, ALfloat pitch,
				      const snd_pcm_channel_area_t *area, snd_pcm_uframes_t frames,
				      snd_pcm_uframes_t offset)
{
	AL_queue *que;
	AL_buffer *buf;
	ALfloat inc, acc;
	ALuint 	i;
	ALfloat  j;
	ALfloat vol[8];
	int32_t *dest[8];
	int c;


	if (!src->playing)
	{
		return 0;
	}

	if ((que = src->current_q) ||
	    ((que = src->first_q) && (que->state == AL_PENDING)))
	{
		buf = que->buffer;
	}
	else if ((buf = src->buffer))
	{
		que = 0;
	}
	else
	{
		src->playing = AL_FALSE;
		return 0;
	}

	inc = pitch * (ALfloat)buf->freq / (ALfloat)src->freq ;
			
	for (c=0;c<src->channels;c++){
		vol[c] = src->volume[c];
		if (c % 2 != 0) vol[c] *= 65536.0f;		// Right channels
		else 
			dest[c] = area[c].addr + ((area[c].first + area[c].step * offset) >> 3);
	}
	
	acc = 0;
	i = 0;
	
	j = src->index;
	if (buf->mono)	
	{
		ALfloat left;

		while (i < frames)
		{
			if ((ALuint)j >= buf->size)				
			{
				j = 0;
	
				if (que)
				{
					que->state = AL_PROCESSED;
					src->current_q = que->next;
				}
				else if(!src->looping)
				{
					src->playing = AL_FALSE;
				}
	
				break;
			}

			left = buf->data[(ALuint)j];
			for (c=0;c<src->channels ;c+=2){
				dest[c][i] = ((((ALint)(left *  vol[c])   & 0x0000FFFF)) | 
						       ((ALint)(left *  vol[c+1]) & 0xFFFF0000));		
			}
			i++;
			j += inc;		
		}
	}
	else
	{
		ALfloat left,right;
		__int32_t *p=(__int32_t*)buf->data;
		

		while (i < frames)
		{
			if ((ALuint)j >= buf->size)				
			{
				j = 0;
	
				if (que)
				{
					que->state = AL_PROCESSED;
					src->current_q = que->next;
				}
				else if(!src->looping)
				{
					src->playing = AL_FALSE;
				}
	
				break;
			}


			left   = (ALshort)(p[(ALuint)j] & 0x0000FFFF);			
			right  = (ALshort)((p[(ALuint)j] & 0xFFFF0000)>>16);
			
			for (c=0;c<src->channels ;c+=2){
				dest[c][i] = ((((ALint)(left  *  vol[c])   & 0x0000FFFF)) | 
						       ((ALint)(right *  vol[c+1]) & 0xFFFF0000));		
			}
		    i++;		          	     
			j += inc;		
		}

	}

	src->index = j;	 	

	return i;
}


ALvoid _alProcessSource(AL_source *src)
{
	const snd_pcm_channel_area_t *area;
 	snd_pcm_sframes_t avail;
	ALfloat pitch;
	int state;

	
	if (src->state != AL_PLAYING)
	{
		return;
	}

	snd_pcm_hwsync(src->handle);

	state = snd_pcm_state(src->handle);

	if (state != SND_PCM_STATE_RUNNING)
	{
		if (src->playing)
		{
			snd_pcm_prepare(src->handle);
		}
		else
		{
			src->state = AL_STOPPED;
			return;
		}
	}

	pitch = _alCalculateGainAndPitch(src);

	avail = snd_pcm_avail_update(src->handle);

	while (avail > 0)
	{
		snd_pcm_uframes_t offset;
		snd_pcm_uframes_t frames = avail;
		snd_pcm_uframes_t written = 0;
		int32_t *map,*map1;

		if (snd_pcm_mmap_begin(src->handle, &area, &offset, &frames))
		{
			return;
		}

		avail -= frames;

		while (frames)
		{
			snd_pcm_uframes_t f;

			if (!(f = _alWriteData(src, pitch, area, frames, offset)))
			{
/*				bzero(area, frames << 2);*/
				avail = 0;
				break;
			}

			map  += f;
			map1 += f;			
			written += f;
			frames -= f;
		}

		snd_pcm_mmap_commit(src->handle, offset, written);
	}

	if (state != SND_PCM_STATE_RUNNING)
	{
		if (!snd_pcm_delay(src->handle, &avail))
		{
			if (avail)
			{
				snd_pcm_start(src->handle);
			}
		}
	}

	if (!src->playing)
	{
		snd_pcm_drain(src->handle);
	}
}

static ALvoid _alSourcePlay(AL_source *src)
{	
	switch(src->state)
	{
	case AL_PAUSED:
		snd_pcm_pause(src->handle, 0);
		break;
	case AL_PLAYING:
		src->index = 0;
		break;
	}
	src->state = AL_PLAYING;
	src->playing = AL_TRUE;
}

ALvoid _alSourceStop(AL_source *src)
{
	switch (src->state)
	{
	case AL_PAUSED:
		snd_pcm_pause(src->handle, 0);
	case AL_PLAYING:
		snd_pcm_drop(src->handle);
	}
	src->state = AL_STOPPED;
	src->index = 0;
}

static ALvoid _alSourcePause(AL_source *src)
{
	if (src->state == AL_PLAYING)
	{
		snd_pcm_pause(src->handle, 1);
		src->state = AL_PAUSED;
	}
}

static ALvoid _alSourceRewind(AL_source *src)
{
	switch (src->state)
	{
	case AL_PAUSED:
		snd_pcm_pause(src->handle, 0);
	case AL_PLAYING:
		snd_pcm_drop(src->handle);
	}
	src->state = AL_INITIAL;
	src->index = 0;
}

ALvoid alSourcePlay(ALuint sid)
{
	AL_context *ctx;
	AL_source *src;


	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}

	_alcLockContext(ctx);
	
	if (!(src = _alFindSource(ctx, sid)))
	{
		_alSetError(AL_INVALID_NAME);
	}
	else
	{
		_alSourcePlay(src);
	}

	_alcUnlockContext(ctx);
}

ALvoid alSourceStop(ALuint sid)
{
	AL_context *ctx;
	AL_source *src;

	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}

	_alcLockContext(ctx);

	if (!(src = _alFindSource(ctx, sid)))
	{
		_alSetError(AL_INVALID_NAME);
	}
	else
	{
		_alSourceStop(src);
	}

	_alcUnlockContext(ctx);
}

ALvoid alSourcePause(ALuint sid)
{
	AL_context *ctx;
	AL_source *src;

	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}
	
	_alcLockContext(ctx);

	if (!(src = _alFindSource(ctx, sid)))
	{
		_alSetError(AL_INVALID_NAME);
	}
	else
	{
		_alSourcePause(src);
	}

	_alcUnlockContext(ctx);
}

ALvoid alSourceRewind(ALuint sid)
{
	AL_context *ctx;
	AL_source *src;

	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}
	
	_alcLockContext(ctx);

	if (!(src = _alFindSource(ctx, sid)))
	{
		_alSetError(AL_INVALID_NAME);
	}
	else
	{
		_alSourceRewind(src);
	}

	_alcUnlockContext(ctx);
}

ALvoid alSourcePlayv(ALsizei ns, ALuint *ids)
{
	AL_context *ctx;
	AL_source **src;
	ALsizei i;

	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}
	
	_alcLockContext(ctx);

	src = alloca(ns * sizeof(AL_source *));

	for (i = 0; i < ns; i++)
	{
		if (!(src[i] = _alFindSource(ctx, ids[i])))
		{
			_alSetError(AL_INVALID_NAME);
			goto unlock;
		}
	}

	for (i = 0; i < ns; i++)
	{
		_alSourcePlay(src[i]);
	}

unlock:
	_alcUnlockContext(ctx);
}

ALvoid alSourceStopv(ALsizei ns, ALuint *ids)
{
	AL_context *ctx;
	AL_source **src;
	ALsizei i;

	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}

	_alcLockContext(ctx);

	src = alloca(ns * sizeof(AL_source *));

	for (i = 0; i < ns; i++)
	{
		if (!(src[i] = _alFindSource(ctx, ids[i])))
		{
			_alSetError(AL_INVALID_NAME);
			goto unlock;
		}
	}
	
	for (i = 0; i < ns; i++)
	{
		_alSourceStop(src[i]);
	}

unlock:
	_alcUnlockContext(ctx);
}

ALvoid alSourcePausev(ALsizei ns, ALuint *ids)
{
	AL_context *ctx;
	AL_source **src;
	ALsizei i;

	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}

	_alcLockContext(ctx);

	src = alloca(ns * sizeof(AL_source *));

	for (i = 0; i < ns; i++)
	{
		if (!(src[i] = _alFindSource(ctx, ids[i])))
		{
			_alSetError(AL_INVALID_NAME);
			goto unlock;
		}
	}

	for (i = 0; i < ns; i++)
	{
		_alSourcePause(src[i]);
	}

unlock:
	_alcUnlockContext(ctx);
}

ALvoid alSourceRewindv(ALsizei ns, ALuint *ids)
{
	AL_context *ctx;
	AL_source **src;
	ALsizei i;

	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}

	_alcLockContext(ctx);

	src = alloca(ns * sizeof(AL_source *));

	for (i = 0; i < ns; i++)
	{
		if (!(src[i] = _alFindSource(ctx, ids[i])))
		{
			_alSetError(AL_INVALID_NAME);
			goto unlock;
		}
	}

	for (i = 0; i < ns; i++)
	{
		_alSourceRewind(src[i]);
	}

unlock:
	_alcUnlockContext(ctx);
}
