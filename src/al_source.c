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

#include <float.h>
#include <alloca.h>

#include "al_source.h"
#include "al_vector.h"
#include "al_error.h"
#include "alc_device.h"
#include "alc_context.h"

#define AL_FIRST_SOURCE_ID 0x4000

AL_source *_alFindSource(AL_context *ctx, ALuint cid)
{
	cid -= AL_FIRST_SOURCE_ID;

	if (cid >= ctx->device->subdevs)
	{
		return 0;
	}

	return ctx->sources[cid];
}

static AL_source *_alGenSource(AL_context *ctx)
{
	AL_source *src;

	if (!(src = malloc(sizeof(AL_source))))
	{
		_alSetError(AL_OUT_OF_MEMORY);
		return 0;
	}

	src->context = ctx;
	src->handle = 0;
	src->subdev = -1;
	src->freq = 0;
	src->periods = 0;

	src->state = AL_INITIAL;
	src->playing = AL_FALSE;
	src->buffer = 0;
	src->index = 0;

	src->first_q = 0;
	src->current_q = 0;
	src->last_q = &src->first_q;

	if (!_alcOpenSource(src))
	{
		_alcCloseSource(src);
		free(src);
		_alSetError(AL_OUT_OF_MEMORY);
		return 0;
	}

	src->relative = AL_FALSE;
	src->looping = AL_FALSE;
	src->conic = AL_FALSE;

	src->position[0] = 0.0f;
	src->position[1] = 0.0f;
	src->position[2] = 0.0f;

	src->direction[0] = 0.0f;
	src->direction[1] = 0.0f;
	src->direction[2] = 0.0f;

	src->velocity[0] = 0.0f;
	src->velocity[1] = 0.0f;
	src->velocity[2] = 0.0f;

	src->pitch = 1.0f;
	src->gain = 1.0f;
	src->min_gain = 0.0f;
	src->max_gain = 1.0f;
	src->reference_distance = 1.0f;
	src->rolloff_factor = 1.0f;
	src->max_distance = FLT_MAX;
	src->cone_inner_angle = 360.0f;
	src->cone_outer_angle = 360.0f;
	src->cone_outer_gain = 0.0f;

	ctx->sources[src->subdev] = src;

	return src;
}

ALvoid _alDeleteSource(AL_source *src)
{
	AL_queue *que;
	AL_context *ctx = src->context;

	_alSourceStop(src);
	ctx->sources[src->subdev] = 0;
	_alcCloseSource(src);

	que = src->first_q;
	while (que)
	{
		AL_queue *next = que->next;
		_alUnlockBuffer(que->buffer);		
		free(que);
		que = next;
	}

	if (src->buffer)
	{
		_alUnlockBuffer(src->buffer);
	}

	free(src);
}

ALvoid alGenSources(ALsizei n, ALuint *sources)
{
	AL_context *ctx;
	AL_source **temp;
	ALsizei i;

	if (n == 0)
	{
		return;
	}

	if (n < 0)
	{
		_alSetError(AL_INVALID_VALUE);
		return;
	}

	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}

	_alcLockContext(ctx);

	temp = alloca(n * sizeof(AL_source *));
	
	for (i = 0; i < n; i++)
	{
		if (!(temp[i] = _alGenSource(ctx)))
		{
			ALsizei j;

			for (j = 0; j < i; j++)
				_alDeleteSource(temp[j]);

			goto unlock;
		}
	}

	for (i = 0; i < n; i++)
	{
		sources[i] = temp[i]->subdev + AL_FIRST_SOURCE_ID;
	}

unlock:
	_alcUnlockContext(ctx);
}

ALvoid alDeleteSources(ALsizei n, ALuint* sources)
{
	AL_context *ctx;
	AL_source **temp;
	ALsizei i;

	if (n == 0)
	{
		return;
	}

	if (n < 0)
	{
		_alSetError(AL_INVALID_VALUE);
		return;
	}

	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}

	_alcLockContext(ctx);

	temp = alloca(n * sizeof(AL_source *));

	for (i = 0; i < n; i++)
	{
		if (!(temp[i] = _alFindSource(ctx, sources[i])))
		{
			_alSetError(AL_INVALID_NAME);
			goto unlock;
		}
	}

	for (i = 0; i < n; i++)
	{
		_alDeleteSource(temp[i]);
	}

unlock:
	_alcUnlockContext(ctx);
}

ALboolean alIsSource(ALuint sid)
{
	AL_context *ctx;	
	ALboolean value;

	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return AL_FALSE;
	}

	_alcLockContext(ctx);

	value = _alFindSource(ctx, sid)  ? AL_TRUE : AL_FALSE;

	_alcUnlockContext(ctx);

	return value;
}

static ALvoid _alNormalizeDirection(AL_source *src)
{
	ALfloat mag = _alVectorMagnitude(src->direction);

	if (mag)
	{
		src->conic = AL_TRUE;
		src->direction[0] /= mag;
		src->direction[1] /= mag;
		src->direction[2] /= mag;
	}
	else
	{
		src->conic = AL_FALSE;
	}
}

ALvoid alSourcei(ALuint sid, ALenum param, ALint value)
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
		goto unlock;
	}

	switch(param)
	{
	case AL_SOURCE_RELATIVE:
		_alRangedAssignB(src->relative, value);
		break;
	case AL_LOOPING:
		_alRangedAssignB(src->looping, value);
		break;
	case AL_PITCH:
		_alRangedAssign1(src->pitch, value, 0);
		break;
	case AL_GAIN:
		_alRangedAssign1(src->gain, value, 0);
		break;
	case AL_MIN_GAIN:
		_alRangedAssign2(src->min_gain, value, 0, 1);
		break;
	case AL_MAX_GAIN:
		_alRangedAssign2(src->max_gain, value, 0, 1);
		break;
	case AL_REFERENCE_DISTANCE:	
		_alRangedAssign1(src->reference_distance, value, 0);
		break;
	case AL_ROLLOFF_FACTOR:
		_alRangedAssign1(src->rolloff_factor, value, 0);
		break;
	case AL_MAX_DISTANCE:
		_alRangedAssign1(src->max_distance, value, 0);
		break;
	case AL_CONE_INNER_ANGLE:
		_alRangedAssign2(src->cone_inner_angle, value, 0, 360);
		break;
	case AL_CONE_OUTER_ANGLE:
		_alRangedAssign2(src->cone_outer_angle, value, 0, 360);
		break;
	case AL_CONE_OUTER_GAIN:
		_alRangedAssign2(src->cone_outer_gain, value, 0, 1);
		break;
	case AL_BUFFER:
		{
			AL_buffer *buf;

			if (value)
			{
				if (!(buf = _alLockBuffer(value)))
				{
					_alSetError(AL_INVALID_VALUE);
					break;
				}
			}
			else
			{
				buf = 0;
			}

			if (src->buffer)
			{
				_alUnlockBuffer(src->buffer);
			}

			src->buffer = buf;
		}
		break;
	default:
		_alSetError(AL_INVALID_ENUM);
		break;
	}

unlock:
	_alcUnlockContext(ctx);
}

ALvoid alSourcef(ALuint sid, ALenum param, ALfloat value)
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
		goto unlock;
	}

	switch(param)
	{
	case AL_SOURCE_RELATIVE:
		_alRangedAssignB(src->relative, value);
		break;
	case AL_LOOPING:
		_alRangedAssignB(src->looping, value);
		break;
	case AL_PITCH:
		_alRangedAssign1(src->pitch, value, 0.0f);
		break;
	case AL_GAIN:
		_alRangedAssign1(src->gain, value, 0.0f);
		break;
	case AL_MIN_GAIN:
		_alRangedAssign2(src->min_gain, value, 0.0f, 1.0f);
		break;
	case AL_MAX_GAIN:
		_alRangedAssign2(src->max_gain, value, 0.0f, 1.0f);
		break;
	case AL_REFERENCE_DISTANCE:	
		_alRangedAssign1(src->reference_distance, value, 0.0f);
		break;
	case AL_ROLLOFF_FACTOR:
		_alRangedAssign1(src->rolloff_factor, value, 0.0f);
		break;
	case AL_MAX_DISTANCE:
		_alRangedAssign1(src->max_distance, value, 0.0f);
		break;
	case AL_CONE_INNER_ANGLE:
		_alRangedAssign2(src->cone_inner_angle, value, 0.0f, 360.0f);
		break;
	case AL_CONE_OUTER_ANGLE:
		_alRangedAssign2(src->cone_outer_angle, value, 0.0f, 360.0f);
		break;
	case AL_CONE_OUTER_GAIN:
		_alRangedAssign2(src->cone_outer_gain, value, 0.0f, 1.0f);
		break;
	default:
		_alSetError(AL_INVALID_ENUM);
		break;
	}

unlock:
	_alcUnlockContext(ctx);
}

ALvoid alSource3f(ALuint sid, ALenum param, ALfloat f1, ALfloat f2, ALfloat f3)
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
		goto unlock;
	}

	switch(param)
	{
	case AL_POSITION:
		src->position[0] = f1;
		src->position[1] = f2;
		src->position[2] = f3;
		break;
	case AL_DIRECTION:
		src->direction[0] = f1;
		src->direction[1] = f2;
		src->direction[2] = f3;
		_alNormalizeDirection(src);
		break;
	case AL_VELOCITY:
		src->velocity[0] = f1;
		src->velocity[1] = f2;
		src->velocity[2] = f3;
		break;
	default:
		_alSetError(AL_INVALID_ENUM);
		break;
	}

unlock:
	_alcUnlockContext(ctx);
}

ALvoid alSourcefv(ALuint sid, ALenum param, ALfloat *values)
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
		goto unlock;
	}

	switch(param)
	{
	case AL_SOURCE_RELATIVE:
		_alRangedAssignB(src->relative, values[0]);
		break;
	case AL_LOOPING:
		_alRangedAssignB(src->looping, values[0]);
		break;
	case AL_PITCH:
		_alRangedAssign1(src->pitch, values[0], 0.0f);
		break;
	case AL_GAIN:
		_alRangedAssign1(src->gain, values[0], 0.0f);
		break;
	case AL_MIN_GAIN:
		_alRangedAssign2(src->min_gain, values[0], 0.0f, 1.0f);
		break;
	case AL_MAX_GAIN:
		_alRangedAssign2(src->max_gain, values[0], 0.0f, 1.0f);
		break;
	case AL_REFERENCE_DISTANCE:	
		_alRangedAssign1(src->reference_distance, values[0], 0.0f);
		break;
	case AL_ROLLOFF_FACTOR:
		_alRangedAssign1(src->rolloff_factor, values[0], 0.0f);
		break;
	case AL_MAX_DISTANCE:
		_alRangedAssign1(src->max_distance, values[0], 0.0f);
		break;
	case AL_CONE_INNER_ANGLE:
		_alRangedAssign2(src->cone_inner_angle, values[0],
				 0.0f, 360.0f);
		break;
	case AL_CONE_OUTER_ANGLE:
		_alRangedAssign2(src->cone_outer_angle, values[0],
				 0.0f, 360.0f);
		break;
	case AL_CONE_OUTER_GAIN:
		_alRangedAssign2(src->cone_outer_gain, values[0], 0.0f, 1.0f);
		break;
	case AL_POSITION:
		src->position[0] = values[0];
		src->position[1] = values[1];
		src->position[2] = values[2];
		break;
	case AL_DIRECTION:
		src->direction[0] = values[0];
		src->direction[1] = values[1];
		src->direction[2] = values[2];
		_alNormalizeDirection(src);
		break;
	case AL_VELOCITY:
		src->velocity[0] = values[0];
		src->velocity[1] = values[1];
		src->velocity[2] = values[2];
		break;
	default:
		_alSetError(AL_INVALID_ENUM);
		break;
	}

unlock:
	_alcUnlockContext(ctx);
}

static ALint _alBuffersQueued(AL_source *src)
{
	AL_queue *q;
	ALint count = 0;

	for (q = src->first_q; q; q = q->next)
	{
		count++;
	}

	return count;
}

static ALint _alBuffersProcessed(AL_source *src)
{
	AL_queue *q;
	ALint count = 0;

	for (q = src->first_q; q; q = q->next)
	{
		if (q->state != AL_PROCESSED)
		{
			break;
		}
		count++;
	}

	return count;
}

ALvoid alGetSourcei(ALuint sid, ALenum param, ALint *value)
{
	ALint values[3];

	values[0] = *value;
	alGetSourceiv(sid, param, values);
	*value = values[0];
}

ALvoid alGetSourceiv(ALuint sid, ALenum param, ALint *values)
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
		goto unlock;
	}

	switch(param)
	{
	case AL_SOURCE_RELATIVE:
		values[0] = src->relative;
		break;
	case AL_SOURCE_STATE:
		values[0] = src->state;
		break;
	case AL_LOOPING:
		values[0] = src->looping;
		break;
	case AL_PITCH:
		values[0] = (ALint)src->pitch;
		break;
	case AL_GAIN:
		values[0] = (ALint)src->gain;
		break;
	case AL_MIN_GAIN:
		values[0] = (ALint)src->min_gain;
		break;
	case AL_MAX_GAIN:
		values[0] = (ALint)src->max_gain;
		break;
	case AL_REFERENCE_DISTANCE:
		values[0] = (ALint)src->reference_distance;
		break;
	case AL_ROLLOFF_FACTOR:
		values[0] = (ALint)src->rolloff_factor;
		break;
	case AL_MAX_DISTANCE:
		values[0] = (ALint)src->max_distance;
		break;
	case AL_CONE_INNER_ANGLE:
		values[0] = (ALint)src->cone_inner_angle;
		break;
	case AL_CONE_OUTER_ANGLE:
		values[0] = (ALint)src->cone_outer_angle;
		break;
	case AL_CONE_OUTER_GAIN:
		values[0] = (ALint)src->cone_outer_gain;
		break;
	case AL_POSITION:
		values[0] = (ALint)src->position[0];
		values[1] = (ALint)src->position[1];
		values[2] = (ALint)src->position[2];
		break;
	case AL_DIRECTION:
		values[0] = (ALint)src->direction[0];
		values[1] = (ALint)src->direction[1];
		values[2] = (ALint)src->direction[2];
		break;
	case AL_VELOCITY:
		values[0] = (ALint)src->velocity[0];
		values[1] = (ALint)src->velocity[1];
		values[2] = (ALint)src->velocity[2];
		break;
	case AL_BUFFER:
		values[0] = src->buffer ? src->buffer->id : 0;
		break;
	case AL_BUFFERS_QUEUED:
		values[0] = _alBuffersQueued(src);
		break;
	case AL_BUFFERS_PROCESSED:
		values[0] = _alBuffersProcessed(src);
		break;
	default:
		_alSetError(AL_INVALID_ENUM);
		break;
	}

unlock:
	_alcUnlockContext(ctx);
}

ALvoid alGetSourcef(ALuint sid, ALenum param, ALfloat *value)
{
	ALfloat values[3];

	values[0] = *value;
	alGetSourcefv(sid, param, values);
	*value = values[0];
}

ALvoid alGetSource3f(ALuint sid, ALenum param,
		     ALfloat *f1, ALfloat *f2, ALfloat *f3)
{
	ALfloat values[6];

	values[0] = *f1;
	values[1] = *f2;
	values[2] = *f3;

	alGetSourcefv(sid, param, values);

	*f1 = values[0];
	*f2 = values[1];
	*f3 = values[2];
}

ALvoid alGetSourcefv(ALuint sid, ALenum param, ALfloat* values)
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
		goto unlock;
	}

	switch(param)
	{
	case AL_SOURCE_RELATIVE:
		values[0] = src->relative;
		break;
	case AL_SOURCE_STATE:
		values[0] = (ALfloat)src->state;
		break;
	case AL_LOOPING:
		values[0] = (ALfloat)src->looping;
		break;
	case AL_PITCH:
		values[0] = src->pitch;
		break;
	case AL_GAIN:
		values[0] = src->gain;
		break;
	case AL_MIN_GAIN:
		values[0] = src->min_gain;
		break;
	case AL_MAX_GAIN:
		values[0] = src->max_gain;
		break;
	case AL_REFERENCE_DISTANCE:
		values[0] = src->reference_distance;
		break;
	case AL_ROLLOFF_FACTOR:
		values[0] = src->rolloff_factor;
		break;
	case AL_MAX_DISTANCE:
		values[0] = src->max_distance;
		break;
	case AL_CONE_INNER_ANGLE:
		values[0] = src->cone_inner_angle;
		break;
	case AL_CONE_OUTER_ANGLE:
		values[0] = src->cone_outer_angle;
		break;
	case AL_CONE_OUTER_GAIN:
		values[0] = src->cone_outer_gain;
		break;
	case AL_POSITION:
		values[0] = src->position[0];
		values[1] = src->position[1];
		values[2] = src->position[2];
		break;
	case AL_DIRECTION:
		values[0] = src->direction[0];
		values[1] = src->direction[1];
		values[2] = src->direction[2];
		break;
	case AL_VELOCITY:
		values[0] = src->velocity[0];
		values[1] = src->velocity[1];
		values[2] = src->velocity[2];
		break;
	case AL_BUFFERS_QUEUED:
		values[0] = (ALfloat)_alBuffersQueued(src);
		break;
	case AL_BUFFERS_PROCESSED:
		values[0] = (ALfloat)_alBuffersProcessed(src);
		break;
	default:
		_alSetError(AL_INVALID_ENUM);
		break;
	}

unlock:
	_alcUnlockContext(ctx);
}

ALvoid alSourceQueueBuffers(ALuint sid, ALsizei n, ALuint *buffers)
{
	AL_context *ctx;
	AL_source *src;
	ALsizei i;
	AL_queue *first_q;
	AL_queue **last_q;

	if (n == 0)
	{
		return;
	}

	if (n < 0)
	{
		_alSetError(AL_INVALID_VALUE);
		return;
	}

	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}
	
	_alcLockContext(ctx);

	if (!(src = _alFindSource(ctx, sid)))
	{
		_alSetError(AL_INVALID_NAME);
		goto unlock;
	}

	first_q = 0;
	last_q = &first_q;

	for (i = 0; i < n; i++)
	{
		AL_buffer *buf;
		AL_queue *que;

		if (!buffers[i])
			continue;

		if (!(buf = _alLockBuffer(buffers[i])))
		{
			while ((que = first_q))
			{
				first_q = que->next;
				_alUnlockBuffer(que->buffer);
				free(que);
			}

			_alSetError(AL_INVALID_NAME);
			goto unlock;
		}

		if (!(que = malloc(sizeof(AL_queue))))
		{
			while ((que = first_q))
			{
				first_q = que->next;
				_alUnlockBuffer(que->buffer);
				free(que);
			}

			_alSetError(AL_OUT_OF_MEMORY);
			goto unlock;
		}

		que->next = 0;
		que->buffer = buf;
		que->state = AL_PENDING;

		*last_q = que;
		last_q = &que->next;
	}

	if (first_q)
	{
		*src->last_q = first_q;
		src->last_q = last_q;
	}

unlock:
	_alcUnlockContext(ctx);
}


ALvoid alSourceUnqueueBuffers(ALuint sid, ALsizei n, ALuint *buffers)
{
	AL_context *ctx;
	AL_source *src;
	ALsizei i;

	if (n == 0)
	{
		return;
	}

	if (n < 0)
	{
		_alSetError(AL_INVALID_VALUE);
		return;
	}

	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}
	
	_alcLockContext(ctx);

	if (!(src = _alFindSource(ctx, sid)))
	{
		_alSetError(AL_INVALID_NAME);
		goto unlock;
	}

	if (_alBuffersProcessed(src) < n)
	{
		_alSetError(AL_INVALID_OPERATION);
		goto unlock;
	}

	for (i = 0; i < n; i++)
	{
		AL_queue *que = src->first_q;

		src->first_q = que->next;

		buffers[i] = que->buffer->id;

		_alUnlockBuffer(que->buffer);
		free(que);
	}

unlock:
	_alcUnlockContext(ctx);
}
