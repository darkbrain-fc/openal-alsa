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

#ifndef _AL_VECTOR_H_
#define _AL_VECTOR_H_

#include <AL/al.h>

ALfloat _alVectorMagnitude(ALfloat *);
ALvoid _alVectorNormalize(ALfloat *, ALfloat *);
ALvoid _alVectorCrossProduct(ALfloat *, ALfloat *, ALfloat *);
ALfloat _alVectorDotProduct(ALfloat *, ALfloat *);
ALvoid _alVectorMatrix(ALfloat *, ALfloat *, ALfloat *);

#endif
