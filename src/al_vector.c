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

#include <math.h>

#include "al_vector.h"

ALfloat _alVectorMagnitude(ALfloat *v)
{
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

ALvoid _alVectorNormalize(ALfloat *d, ALfloat *s)
{
	ALfloat mag;

	if (!(mag = _alVectorMagnitude(s)))
	{
		d[0] = 0.0f;
		d[1] = 0.0f;
		d[2] = 0.0f;
	}
	else
	{
		d[0] = s[0] / mag;
		d[1] = s[1] / mag;
		d[2] = s[2] / mag;
	}
}

ALvoid _alVectorCrossProduct(ALfloat *d, ALfloat *v1, ALfloat *v2)
{
	d[0] = v1[1] * v2[2] - v1[2] * v2[1];
	d[1] = v1[2] * v2[0] - v1[0] * v2[2];
	d[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

ALfloat _alVectorDotProduct(ALfloat *v1, ALfloat *v2)
{
	return (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]);
}

ALvoid _alVectorMatrix(ALfloat *d, ALfloat *s, ALfloat *m)
{
	d[0] = s[0] * m[0] + s[1] * m[3] + s[2] * m[6];
	d[1] = s[0] * m[1] + s[1] * m[4] + s[2] * m[7];
	d[2] = s[0] * m[2] + s[1] * m[5] + s[2] * m[8];
}
