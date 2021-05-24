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

#include "al_listener.h"
#include "al_error.h"
#include "al_vector.h"
#include "alc_context.h"

static ALvoid _alListenerSetSpeakers(AL_listener *listener,
				     AL_speaker *speakers)
{
	ALfloat matrix[9];
	ALuint i;

	_alVectorCrossProduct(matrix + 0, listener->orientation,
			      listener->orientation + 3);
	_alVectorNormalize(matrix + 0, matrix + 0);

	_alVectorCrossProduct(matrix + 3, listener->orientation + 0,
			      matrix + 0);
	_alVectorNormalize(matrix + 3, matrix + 3);

	_alVectorNormalize(matrix + 6, listener->orientation + 0);

	for (i = 0; i < _ALC_NUM_SPEAKERS; i++)
	{
		_alVectorMatrix(listener->speakers[i].position,
				speakers[i].position, matrix);
	}
}

ALvoid _alInitListener(AL_listener *listener, AL_speaker *speakers)
{
	ALuint i;

	listener->gain = 1.0f;

	listener->position[0] = 0.0f;
	listener->position[1] = 0.0f;
	listener->position[2] = 0.0f;

	listener->velocity[0] = 0.0f;
	listener->velocity[1] = 0.0f;
	listener->velocity[2] = 0.0f;

	listener->orientation[0] = 0.0f;
	listener->orientation[1] = 0.0f;
	listener->orientation[2] = 0.0f;
	listener->orientation[3] = 0.0f;
	listener->orientation[4] = 0.0f;
	listener->orientation[5] = 0.0f;

	for (i = 0; i < _ALC_NUM_SPEAKERS; i++)
	{
		listener->speakers[i].gain = speakers[i].gain;
	}

	_alListenerSetSpeakers(listener, speakers);
}

ALvoid alListeneri(ALenum pname, ALint value)
{
	alListenerf(pname, (ALfloat)value);
}

ALvoid alListenerf(ALenum pname, ALfloat value)
{
	AL_context *ctx;

	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}

	_alcLockContext(ctx);

	switch (pname)
	{
	case AL_GAIN:
		_alRangedAssign1(ctx->listener.gain, value, 0.0f);
		break;
	default:
		_alSetError(AL_INVALID_ENUM);
		break;
	}

	_alcUnlockContext(ctx);
}

ALvoid alListener3f(ALenum pname, ALfloat f1, ALfloat f2, ALfloat f3)
{
	AL_context *ctx;

	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}

	_alcLockContext(ctx);

	switch (pname)
	{
	case AL_POSITION:
		ctx->listener.position[0] = f1;
		ctx->listener.position[1] = f2;
		ctx->listener.position[2] = f3;
		break;
	case AL_VELOCITY:
		ctx->listener.velocity[0] = f1;
		ctx->listener.velocity[1] = f2;
		ctx->listener.velocity[2] = f3;
		break;
	default:
		_alSetError(AL_INVALID_ENUM);
		break;
	}

	_alcUnlockContext(ctx);
}

ALvoid alListenerfv(ALenum pname, ALfloat* values)
{
	AL_context *ctx;

/*	fprintf(stderr,"alListenerfv\n");*/
	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}

	_alcLockContext(ctx);

	switch (pname)
	{
	case AL_POSITION:
		ctx->listener.position[0] = values[0];
		ctx->listener.position[1] = values[1];
		ctx->listener.position[2] = values[2];
		break;
	case AL_VELOCITY:
		ctx->listener.velocity[0] = values[0];
		ctx->listener.velocity[1] = values[1];
		ctx->listener.velocity[2] = values[2];
		break;
	case AL_GAIN:
		_alRangedAssign1(ctx->listener.gain, values[0], 0.0f);
		break;
	case AL_ORIENTATION:
		ctx->listener.orientation[0] = values[0];
		ctx->listener.orientation[1] = values[1];
		ctx->listener.orientation[2] = values[2];
		ctx->listener.orientation[3] = values[3];
		ctx->listener.orientation[4] = values[4];
		ctx->listener.orientation[5] = values[5];
		_alListenerSetSpeakers(&ctx->listener, ctx->speakers);
		break;
	default:
		_alSetError(AL_INVALID_ENUM);
		break;
	}

	_alcUnlockContext(ctx);
}

ALvoid alGetListeneri(ALenum pname, ALint *value)
{
	ALint values[6];

	values[0] = *value;
	alGetListeneriv(pname, values);
	*value = values[0];
}

ALvoid alGetListeneriv(ALenum pname, ALint* values)
{
	AL_context *ctx;

	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}

	_alcLockContext(ctx);

	switch (pname)
	{
	case AL_POSITION:
		values[0] = (ALint)ctx->listener.position[0];
		values[1] = (ALint)ctx->listener.position[1];
		values[2] = (ALint)ctx->listener.position[2];
		break;
	case AL_VELOCITY:
		values[0] = (ALint)ctx->listener.velocity[0];
		values[1] = (ALint)ctx->listener.velocity[1];
		values[2] = (ALint)ctx->listener.velocity[2];
		break;
	case AL_GAIN:
		values[0] = (ALint)ctx->listener.gain;
		break;
	case AL_ORIENTATION:
		values[0] = (ALint)ctx->listener.orientation[0];
		values[1] = (ALint)ctx->listener.orientation[1];
		values[2] = (ALint)ctx->listener.orientation[2];
		values[3] = (ALint)ctx->listener.orientation[3];
		values[4] = (ALint)ctx->listener.orientation[4];
		values[5] = (ALint)ctx->listener.orientation[5];
		break;
	default:
		_alSetError(AL_INVALID_ENUM);
		break;
	}

	_alcUnlockContext(ctx);
}

ALvoid alGetListenerf(ALenum pname, ALfloat *value)
{
	ALfloat values[6];

	values[0] = *value;
	alGetListenerfv(pname, values);
	*value = values[0];
}

ALvoid alGetListener3f(ALenum pname, ALfloat *f1, ALfloat *f2, ALfloat *f3)
{
	ALfloat values[6];

	values[0] = *f1;
	values[1] = *f2;
	values[2] = *f3;

	alGetListenerfv(pname, values);

	*f1 = values[0];
	*f2 = values[1];
	*f3 = values[2];
}

ALvoid alGetListenerfv(ALenum pname, ALfloat* values)
{
	AL_context *ctx;

	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}

	_alcLockContext(ctx);

	switch (pname)
	{
	case AL_POSITION:
		values[0] = ctx->listener.position[0];
		values[1] = ctx->listener.position[1];
		values[2] = ctx->listener.position[2];
		break;
	case AL_VELOCITY:
		values[0] = ctx->listener.velocity[0];
		values[1] = ctx->listener.velocity[1];
		values[2] = ctx->listener.velocity[2];
		break;
	case AL_GAIN:
		values[0] = ctx->listener.gain;
		break;
	case AL_ORIENTATION:
		values[0] = ctx->listener.orientation[0];
		values[1] = ctx->listener.orientation[1];
		values[2] = ctx->listener.orientation[2];
		values[3] = ctx->listener.orientation[3];
		values[4] = ctx->listener.orientation[4];
		values[5] = ctx->listener.orientation[5];
		break;
	default:
		_alSetError(AL_INVALID_ENUM);
		break;
	}

	_alcUnlockContext(ctx);
}
