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

#include "alc_device.h"
#include "alc_error.h"

ALCvoid alcGetIntegerv(ALCdevice *dev, ALCenum param,
		       ALCsizei size, ALCint *data)
{
	if (!data || size < sizeof(ALCint))
	{
		_alcSetError(ALC_INVALID_VALUE);
		return;
	}

	switch(param)
	{
	case ALC_MAJOR_VERSION:
		*data = 1;
		break;
	case ALC_MINOR_VERSION:
		*data = 0;
		break;
	case ALC_ATTRIBUTES_SIZE:
		if (!dev)
		{
			_alcSetError(ALC_INVALID_DEVICE);
			break;
		}
		*data = 8 * sizeof(ALCint);
		break;
	case ALC_ALL_ATTRIBUTES:
		if (!dev)
		{
			_alcSetError(ALC_INVALID_DEVICE);
			break;
		}
		if (size < (8 * sizeof(ALCint)))
		{
			_alcSetError(ALC_INVALID_VALUE);
			break;
		}
		data[0] = ALC_FREQUENCY;
		data[1] = dev->freq;
		data[2] = ALC_REFRESH;
		data[3] = dev->refresh;
		data[4] = ALC_SYNC;
		data[5] = dev->sync;
		data[6] = ALC_INVALID;
		data[7] = 0;
		break;
	default:
		_alcSetError(ALC_INVALID_ENUM);
		break;
	}
}

const ALubyte *alcGetString(ALCdevice *dev ATTRIBUTE_UNUSED, ALCenum param)
{
	switch (param)
	{
	case ALC_DEFAULT_DEVICE_SPECIFIER:
		return "";
	case ALC_DEVICE_SPECIFIER:
		return "";
	case ALC_EXTENSIONS:
		return "";
	case ALC_NO_ERROR:
		return "No error";
	case ALC_INVALID_DEVICE:
		return "There is no accessible sound device/driver/server";
	case ALC_INVALID_CONTEXT:
		return "The Context argument does not name a valid context";
	case ALC_INVALID_ENUM:
		return "Illegal paramater";
	case ALC_INVALID_VALUE:
		return "Invalid enum parameter value";
	case ALC_OUT_OF_MEMORY:
		return "Unable to allocate memory";
	default:
		_alcSetError(ALC_INVALID_ENUM);
		break;
	}

	return 0;
}
