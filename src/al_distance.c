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

#include "al_source.h"
#include "al_error.h"
#include "alc_context.h"

static ALfloat _alDistanceNone(AL_source *src, ALfloat dist ATTRIBUTE_UNUSED)
{
	return src->gain;
}

ALfloat _alDistanceInverse(AL_source *src, ALfloat dist)
{
	ALfloat ref = src->reference_distance;

	if (dist < ref)
	{
		dist = ref;
	}

	return src->gain * ref / (ref + src->rolloff_factor * (dist - ref));
}

static ALfloat _alDistanceInverseClamped(AL_source *src, ALfloat dist)
{
	ALfloat ref = src->reference_distance;

	if (dist < ref)
	{
		dist = ref;
	}

	if (dist > src->max_distance)
	{
		dist = src->max_distance;
	}

	return src->gain * ref / (ref + src->rolloff_factor * (dist - ref));
}


ALvoid alDistanceModel(ALenum model)
{
	AL_context *ctx;
	ALfloat (*df)(AL_source *, ALfloat);

	if (!(ctx = _alcCurrentContext))
	{
		_alSetError(AL_INVALID_OPERATION);
		return;
	}

	_alcLockContext(ctx);

	switch (model)
	{
	case AL_NONE:
		df = _alDistanceNone;
		break;
	case AL_INVERSE_DISTANCE:
		df = _alDistanceInverse;
		break;
	case AL_INVERSE_DISTANCE_CLAMPED:
		df = _alDistanceInverseClamped;
		break;
	default:
		_alSetError(AL_ILLEGAL_ENUM);
		goto unlock;
	}

	ctx->distance_model = model;
	ctx->distance_func = df;

unlock:
	_alcUnlockContext(ctx);
}
