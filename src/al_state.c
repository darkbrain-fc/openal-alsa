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

#include "alc_context.h"
#include "al_error.h"

ALboolean alGetBoolean(ALenum param)
{
	ALboolean value = AL_FALSE;

	alGetBooleanv(param, &value);

	return value;
}

ALint alGetInteger(ALenum param)
{
	ALint value = AL_FALSE;

	alGetIntegerv(param, &value);

	return value;
}

ALfloat alGetFloat(ALenum param)
{
	ALfloat value = AL_FALSE;

	alGetFloatv(param, &value);

	return value;
}

ALdouble alGetDouble(ALenum param)
{
	ALdouble value = AL_FALSE;

	alGetDoublev(param, &value);

	return value;
}

ALvoid alGetBooleanv(ALenum param, ALboolean *values)
{
	AL_context *ctx;

	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}

	_alcLockContext(ctx);

	switch(param)
	{
	case AL_DOPPLER_FACTOR:
		*values = (ctx->doppler_factor != 0.0f) ?
			AL_TRUE : AL_FALSE;
		break;
	case AL_DOPPLER_VELOCITY:
		*values = (ctx->doppler_velocity != 0.0f) ?
			AL_TRUE : AL_FALSE;
		break;
	case AL_DISTANCE_MODEL:
		*values = (ctx->distance_model != AL_INVERSE_DISTANCE) ?
			AL_TRUE : AL_FALSE;
		break;
	default:
		_alSetError(AL_INVALID_ENUM);
		break;
	}

	_alcUnlockContext(ctx);
}

ALvoid alGetIntegerv(ALenum param, ALint *values)
{
	AL_context *ctx;

	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}

	_alcLockContext(ctx);

	switch(param)
	{
	case AL_DOPPLER_FACTOR:
		*values = (ALint)ctx->doppler_factor;
		break;
	case AL_DOPPLER_VELOCITY:
		*values = (ALint)ctx->doppler_velocity;
		break;
	case AL_DISTANCE_MODEL:
		*values = ctx->distance_model;
		break;
	default:
		_alSetError(AL_INVALID_ENUM);
		break;
	}

	_alcUnlockContext(ctx);
}

ALvoid alGetFloatv(ALenum param, ALfloat *values)
{
	AL_context *ctx;

	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}

	_alcLockContext(ctx);

	switch(param)
	{
	case AL_DOPPLER_FACTOR:
		*values = ctx->doppler_factor;
		break;
	case AL_DOPPLER_VELOCITY:
		*values = ctx->doppler_velocity;
		break;
	case AL_DISTANCE_MODEL:
		*values = (ALfloat)ctx->distance_model;
		break;
	default:
		_alSetError(AL_INVALID_ENUM);
		break;
	}

	_alcUnlockContext(ctx);
}

ALvoid alGetDoublev(ALenum param, ALdouble *values)
{
	AL_context *ctx;

	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}

	_alcLockContext(ctx);

	switch(param)
	{
	case AL_DOPPLER_FACTOR:
		*values = (ALdouble)ctx->doppler_factor;
		break;
	case AL_DOPPLER_VELOCITY:
		*values = (ALdouble)ctx->doppler_velocity;
		break;
	case AL_DISTANCE_MODEL:
		*values = (ALdouble)ctx->distance_model;
		break;
	default:
		_alSetError(AL_INVALID_ENUM);
		break;
	}

	_alcUnlockContext(ctx);
}

const ALubyte *alGetString(ALenum param)
{
	switch (param)
	{
	case AL_NO_ERROR:
		return (const ALubyte *)"No error";
	case AL_INVALID_NAME:
		return (const ALubyte *)"Invalid Name parameter";
	case AL_INVALID_ENUM:
		return (const ALubyte *)"Illegal paramater";
	case AL_INVALID_VALUE:
		return (const ALubyte *)"Invalid enum parameter value";
	case AL_INVALID_OPERATION:
		return (const ALubyte *)"Illegal call";
	case AL_OUT_OF_MEMORY:
		return (const ALubyte *)"Unable to allocate memory";
	case AL_VENDOR:
		return (const ALubyte *)"Christopher John Purnell/Dino Puller";
	case AL_VERSION:
		return (const ALubyte *)"0.2.0";
	case AL_RENDERER:
		return (const ALubyte *)"ALSA";
	case AL_EXTENSIONS:
		return (const ALubyte *)"";
	default:
		_alSetError(AL_INVALID_ENUM);
		break;
	}

	return 0;
}
