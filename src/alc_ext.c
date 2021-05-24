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

#include <ansidecl.h>

#include <AL/al.h>
#include <AL/alc.h>

ALCboolean alcIsExtensionPresent(ALCdevice *dev ATTRIBUTE_UNUSED,
				 ALCubyte *name ATTRIBUTE_UNUSED)
{
	return ALC_FALSE;
}


ALCvoid *alcGetProcAddress(ALCdevice *dev ATTRIBUTE_UNUSED,
			   ALubyte *name ATTRIBUTE_UNUSED)
{
	return 0;
}


ALCenum alcGetEnumValue(ALCdevice *dev ATTRIBUTE_UNUSED, ALCubyte *name)
{
	return alGetEnumValue(name);
}
