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

#ifndef _AL_LISTENER_H_
#define _AL_LISTENER_H_

#include <AL/al.h>

#include "alc_speaker.h"

typedef struct _AL_listener {
        ALfloat gain;
        ALfloat position[3];
        ALfloat velocity[3];
	ALfloat orientation[6];
	AL_speaker speakers[_ALC_NUM_SPEAKERS];
} AL_listener;

extern ALvoid _alInitListener(AL_listener *, AL_speaker *);

#endif
