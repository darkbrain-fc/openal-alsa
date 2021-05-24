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

#ifndef _AL_ERROR_H_
#define _AL_ERROR_H_

#include <AL/al.h>

extern ALvoid _alSetError(ALenum);

#define _alRangedAssignB(x, v) \
	x = v ? AL_TRUE : AL_FALSE

#define _alRangedAssign1(x, v, lo) \
	if (v < lo) \
		_alSetError(AL_INVALID_VALUE); \
	else \
		x = (ALfloat)v

#define _alRangedAssign2(x, v, lo, hi) \
	if (v < lo || v > hi) \
		_alSetError(AL_INVALID_VALUE); \
	else \
		x = (ALfloat)v

#endif
