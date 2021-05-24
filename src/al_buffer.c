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

#include <sys/types.h>
#include <stdlib.h>
#include <alloca.h>
#include <pthread.h>
#include <stdio.h>

#include "al_buffer.h"
#include "al_error.h"

#define HASH_SIZE 0x100
#define HASH_MASK 0x0FF

static pthread_mutex_t _al_buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

static ALuint _al_last_buffer_id = 0;
static AL_buffer *_al_buffers[HASH_SIZE];

static AL_buffer *_alFindBuffer(ALuint bid)
{
	AL_buffer *buf;

	for (buf = _al_buffers[bid & HASH_MASK]; buf; buf = buf->next)
	{
		if (buf->id == bid)
		{
			return buf;
		}
	}

	return 0;
}

AL_buffer *_alLockBuffer(ALuint bid)
{
	AL_buffer *buf;

	pthread_mutex_lock(&_al_buffer_mutex);

	if ((buf = _alFindBuffer(bid)))
	{
		buf->used++;
	}

	pthread_mutex_unlock(&_al_buffer_mutex);

	return buf;
}

ALvoid _alUnlockBuffer(AL_buffer *buf)
{
	pthread_mutex_lock(&_al_buffer_mutex);

	buf->used--;

	pthread_mutex_unlock(&_al_buffer_mutex);
}

static AL_buffer *_alGenBuffer(ALvoid)
{
	AL_buffer *buf;
	ALuint bid;

	if (!(buf = malloc(sizeof(AL_buffer))))
	{
		_alSetError(AL_OUT_OF_MEMORY);
		return 0;
	}

	buf->used = 0;
	buf->data = 0;
	buf->size = 0;
	buf->freq = 0;
	buf->mono = AL_TRUE;

	bid = _al_last_buffer_id;
	while (!++bid || _alFindBuffer(bid));
	_al_last_buffer_id = bid;

	buf->id = bid;

	buf->next = _al_buffers[bid & HASH_MASK];
	_al_buffers[bid & HASH_MASK] = buf;

	return buf;
}

static ALvoid _alDeleteBuffer(AL_buffer *buf)
{
	AL_buffer *b, **p;

	for (p = &_al_buffers[buf->id & HASH_MASK]; (b = *p); p = &b->next)
	{
		if (b->id == buf->id)
		{
			*p = b->next;
			break;
		}
	}

	if (buf->data) free(buf->data);
	free(buf);
}

ALvoid alGenBuffers(ALsizei n, ALuint *buffers)
{
	ALsizei i;
	AL_buffer **temp;

	if (n == 0)
		return;

	if (n < 0)
	{
		_alSetError(AL_INVALID_VALUE);
		return;
	}

	temp = alloca(n * sizeof(AL_buffer *));

	pthread_mutex_lock(&_al_buffer_mutex);

	for (i = 0; i < n; i++)
	{
		if (!(temp[i] = _alGenBuffer()))
		{
			ALsizei j;

			for (j = 0; j < i; j++)
			{
				_alDeleteBuffer(temp[j]);
			}

			goto unlock;
		}
	}

	for (i = 0; i < n; i++)
	{
		buffers[i] = temp[i]->id;
	}

unlock:
	pthread_mutex_unlock(&_al_buffer_mutex);
}

ALvoid alDeleteBuffers(ALsizei n, ALuint *buffers)
{
	ALsizei i;
	AL_buffer **temp;

	if (n == 0)
	{
		return;
	}

	if (n < 0)
	{
		_alSetError(AL_INVALID_VALUE);
		return;
	}

	temp = alloca(n * sizeof(AL_buffer *));

	pthread_mutex_lock(&_al_buffer_mutex);

	for (i = 0; i < n; i++)
	{
		if (!(temp[i] = _alFindBuffer(buffers[i])))
		{
			_alSetError(AL_INVALID_NAME);
			goto unlock;
		}

		if (temp[i]->used)
		{
			_alSetError(AL_INVALID_OPERATION);
			goto unlock;
		}
	}

	for (i = 0; i < n; i++)
	{
		_alDeleteBuffer(temp[i]);
	}

unlock:
	pthread_mutex_unlock(&_al_buffer_mutex);
}

ALboolean alIsBuffer(ALuint bid)
{
	ALboolean value;

	pthread_mutex_lock(&_al_buffer_mutex);

	value = _alFindBuffer(bid) ? AL_TRUE : AL_FALSE;

	pthread_mutex_unlock(&_al_buffer_mutex);

	return value;
}

static ALvoid _alBufferMono8(AL_buffer *buf, ALvoid *data,
			     ALsizei size, ALsizei freq)
{
	ALvoid *copy;
	ALsizei frames = size;

	if (!(copy = malloc(frames << 1)))
	{
		_alSetError(AL_OUT_OF_MEMORY);
		return;
	}

	{
		__uint8_t *from = data;
		__int16_t *to = copy;
		ALsizei i;

		for (i = 0; i < frames; i++)
		{
			__int16_t value = (*(from++) - 128) << 8;
			*(to++) = value;
		}
	}

	buf->data = copy;
	buf->size = frames;
	buf->freq = freq;
}

static ALvoid _alBufferMono16(AL_buffer *buf, ALvoid *data,
			     ALsizei size, ALsizei freq)
{
	ALvoid *copy;
	ALsizei frames = size >> 1;

	if (!(copy = malloc(frames << 1)))
	{
		_alSetError(AL_OUT_OF_MEMORY);
		return;
	}

	{
		__int16_t *from = data;
		__int16_t *to = copy;
		ALsizei i;

		for (i = 0; i < frames; i++)
		{
			__int16_t value = *(from++);
			 *(to++) = value;
		}
	}

	buf->data = copy;
	buf->size = frames;
	buf->freq = freq;
}

static ALvoid _alBufferStereo8(AL_buffer *buf, ALvoid *data,
			     ALsizei size, ALsizei freq)
{
	ALvoid *copy;
	ALsizei frames = size >> 1;

	if (!(copy = malloc(frames << 2)))
	{
		_alSetError(AL_OUT_OF_MEMORY);
		return;
	}

	{
		__uint8_t *from = data;
		__int16_t *to = copy;
		ALsizei i;

		for (i = 0; i < frames; i++)
		{
			*(to++) = (*(from++) - 128) << 8;
			*(to++) = (*(from++) - 128) << 8;
		}
	}

	buf->data = copy;
	buf->size = frames;
	buf->freq = freq;
	buf->mono = AL_FALSE;
}

static ALvoid _alBufferStereo16(AL_buffer *buf, ALvoid *data,
				ALsizei size, ALsizei freq)
{
	ALvoid *copy;
	ALsizei frames = size >> 2;

	if (!(copy = malloc(frames << 2)))
	{
		_alSetError(AL_OUT_OF_MEMORY);
		return;
	}

	{
		__int32_t *from = data;
		__int32_t *to = copy;
		ALsizei i;

		for (i = 0; i < frames; i++)
		{
			*(to++) = *(from++);
		}
	}

	buf->data = copy;
	buf->size = frames;
	buf->freq = freq;
	buf->mono = AL_FALSE;	
}

ALvoid alBufferData(ALuint bid, ALenum format, ALvoid *data,
		    ALsizei size, ALsizei freq)
{
	AL_buffer *buf;

	if (!(buf = _alLockBuffer(bid)))
	{
		_alSetError(AL_INVALID_NAME);
		return;
	}

	if (buf->used > 1)
	{
		_alSetError(AL_INVALID_OPERATION);
		goto unlock;
	}

	if (buf->data)
	{
		free(buf->data);
		buf->data = 0;
		buf->size = 0;
	}

	switch (format)
	{
	case AL_FORMAT_MONO8:
		_alBufferMono8(buf, data, size, freq);
		break;
	case AL_FORMAT_MONO16:
		_alBufferMono16(buf, data, size, freq);
		break;
	case AL_FORMAT_STEREO8:
		_alBufferStereo8(buf, data, size, freq);
		break;
	case AL_FORMAT_STEREO16:
		_alBufferStereo16(buf, data, size, freq);
		break;
	default:
		_alSetError(AL_INVALID_ENUM);
		break;
	}

unlock:
	    _alUnlockBuffer(buf);
}

ALvoid alGetBufferi(ALuint bid, ALenum param, ALint *value)
{
	AL_buffer *buf;

	pthread_mutex_lock(&_al_buffer_mutex);

	if (!(buf = _alFindBuffer(bid)))
	{
		_alSetError(AL_INVALID_NAME);
		goto unlock;
	}

	switch (param)
	{
	case AL_FREQUENCY:
		*value = (ALint)buf->freq;
		break;
	case AL_BITS:
		*value = 16;
		break;
	case AL_CHANNELS:
		*value = 2;
		break;
	case AL_SIZE:
		*value = (ALint)(buf->size << 2);
		break;
	default:
		_alSetError(AL_INVALID_ENUM);
		break;
	}

unlock:
	pthread_mutex_unlock(&_al_buffer_mutex);
}

ALvoid alGetBufferf(ALuint bid, ALenum param, ALfloat *value)
{
	AL_buffer *buf;

	pthread_mutex_lock(&_al_buffer_mutex);

	if (!(buf = _alLockBuffer(bid)))
	{
		_alSetError(AL_INVALID_NAME);
		goto unlock;
	}

	switch (param)
	{
	case AL_FREQUENCY:
		*value = (ALfloat)buf->freq;
		break;
	case AL_BITS:
		*value = 16.0f;
		break;
	case AL_CHANNELS:
		*value = 2.0f;
		break;
	case AL_SIZE:
		*value = (ALfloat)(buf->size << 2);
		break;
	default:
		_alSetError(AL_INVALID_ENUM);
		break;
	}

unlock:
	pthread_mutex_unlock(&_al_buffer_mutex);
}

ALvoid alGetBufferiv(ALuint bid, ALenum param, ALint *values)
{
	alGetBufferi(bid, param, values);
}

ALvoid alGetBufferfv(ALuint bid, ALenum param, ALfloat *values)
{
	alGetBufferf(bid, param, values);
}
