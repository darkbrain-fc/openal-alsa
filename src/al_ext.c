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

#include <string.h>
#include <ansidecl.h>

#include <AL/al.h>
#include <AL/alc.h>

static struct {	const ALubyte *name; ALenum value; } _al_enums[]= {
	{ "AL_INVALID",			  AL_INVALID			},
	{ "AL_NONE",			  AL_NONE			},
	{ "AL_FALSE",			  AL_FALSE			},
	{ "AL_TRUE",			  AL_TRUE			},

	{ "ALC_INVALID",		  ALC_INVALID			},
	{ "ALC_TRUE",			  ALC_TRUE			},
	{ "ALC_FALSE",			  ALC_FALSE			},
	
	{ "AL_SOURCE_RELATIVE",		  AL_SOURCE_RELATIVE		},
	{ "AL_CONE_INNER_ANGLE",	  AL_CONE_INNER_ANGLE		},
	{ "AL_CONE_OUTER_ANGLE",	  AL_CONE_OUTER_ANGLE		},
	{ "AL_PITCH",			  AL_PITCH			},
	{ "AL_POSITION",		  AL_POSITION			},
	{ "AL_DIRECTION",		  AL_DIRECTION			},
	{ "AL_VELOCITY",		  AL_VELOCITY			},
	{ "AL_LOOPING",			  AL_LOOPING			},
	{ "AL_BUFFER",			  AL_BUFFER			},
	{ "AL_GAIN",			  AL_GAIN			},
	{ "AL_MIN_GAIN",		  AL_MIN_GAIN			},
	{ "AL_MAX_GAIN",		  AL_MAX_GAIN			},
	{ "AL_ORIENTATION",		  AL_ORIENTATION		},
	{ "AL_REFERENCE_DISTANCE",	  AL_REFERENCE_DISTANCE		},
	{ "AL_ROLLOFF_FACTOR",		  AL_ROLLOFF_FACTOR		},
	{ "AL_CONE_OUTER_GAIN",		  AL_CONE_OUTER_GAIN		},
	{ "AL_MAX_DISTANCE",		  AL_MAX_DISTANCE		},

	{ "AL_SOURCE_STATE",		  AL_SOURCE_STATE		},
	{ "AL_INITIAL",			  AL_INITIAL			},
	{ "AL_PLAYING",			  AL_PLAYING			},
	{ "AL_PAUSED",			  AL_PAUSED			},
	{ "AL_STOPPED",			  AL_STOPPED			},

	{ "AL_BUFFERS_QUEUED",		  AL_BUFFERS_QUEUED		},
	{ "AL_BUFFERS_PROCESSED",	  AL_BUFFERS_PROCESSED		},
	
	{ "AL_FORMAT_MONO8",		  AL_FORMAT_MONO8		},
	{ "AL_FORMAT_MONO16",		  AL_FORMAT_MONO16		},
	{ "AL_FORMAT_STEREO8",		  AL_FORMAT_STEREO8		},
	{ "AL_FORMAT_STEREO16",		  AL_FORMAT_STEREO16		},

	{ "AL_FREQUENCY",		  AL_FREQUENCY			},
	{ "AL_BITS",			  AL_BITS			},
	{ "AL_CHANNELS",		  AL_CHANNELS			},
	{ "AL_SIZE",			  AL_SIZE			},
	{ "AL_DATA",			  AL_DATA			},
	
	{ "AL_UNUSED",			  AL_UNUSED			},
	{ "AL_PENDING",			  AL_PENDING			},
	{ "AL_PROCESSED",		  AL_PROCESSED			},

	{ "ALC_MAJOR_VERSION",		  ALC_MAJOR_VERSION		},
	{ "ALC_MINOR_VERSION",		  ALC_MINOR_VERSION		},
	{ "ALC_ATTRIBUTES_SIZE",	  ALC_ATTRIBUTES_SIZE		},
	{ "ALC_ALL_ATTRIBUTES",		  ALC_ALL_ATTRIBUTES		},
	{ "ALC_DEFAULT_DEVICE_SPECIFIER", ALC_DEFAULT_DEVICE_SPECIFIER	},
	{ "ALC_DEVICE_SPECIFIER",	  ALC_DEVICE_SPECIFIER		},
	{ "ALC_EXTENSIONS",		  ALC_EXTENSIONS		},
	{ "ALC_FREQUENCY",		  ALC_FREQUENCY			},
	{ "ALC_REFRESH",		  ALC_REFRESH			},
	{ "ALC_SYNC",			  ALC_SYNC			},

	{ "AL_NO_ERROR",		  AL_NO_ERROR			},
	{ "AL_INVALID_NAME",		  AL_INVALID_NAME		},
	{ "AL_INVALID_ENUM",		  AL_INVALID_ENUM		},
	{ "AL_INVALID_VALUE",		  AL_INVALID_VALUE		},
	{ "AL_INVALID_OPERATION",	  AL_INVALID_OPERATION		},
	{ "AL_OUT_OF_MEMORY",		  AL_OUT_OF_MEMORY		},
	
	{ "ALC_NO_ERROR",		  ALC_NO_ERROR			},
	{ "ALC_INVALID_DEVICE",		  ALC_INVALID_DEVICE		},
	{ "ALC_INVALID_CONTEXT",	  ALC_INVALID_CONTEXT		},
	{ "ALC_INVALID_ENUM",		  ALC_INVALID_ENUM		},
	{ "ALC_INVALID_VALUE",		  ALC_INVALID_VALUE		},
	{ "ALC_OUT_OF_MEMORY",		  ALC_OUT_OF_MEMORY		},

	{ "AL_VENDOR",			  AL_VENDOR			},
	{ "AL_VERSION",			  AL_VERSION			},
	{ "AL_RENDERER",		  AL_RENDERER			},
	{ "AL_EXTENSIONS",		  AL_EXTENSIONS			},
	
	{ "AL_DOPPLER_FACTOR",		  AL_DOPPLER_FACTOR		},
	{ "AL_DOPPLER_VELOCITY",	  AL_DOPPLER_VELOCITY		},
	{ "AL_DISTANCE_MODEL",		  AL_DISTANCE_MODEL		},
	
	{ "AL_INVERSE_DISTANCE",	  AL_INVERSE_DISTANCE		},
	{ "AL_INVERSE_DISTANCE_CLAMPED",  AL_INVERSE_DISTANCE_CLAMPED	},

	{ 0, 0 }
};

ALboolean alIsExtensionPresent(const ALubyte *name ATTRIBUTE_UNUSED)
{
	return AL_FALSE;
}


ALvoid *alGetProcAddress(const ALubyte *name ATTRIBUTE_UNUSED)
{
	return 0;
}


ALenum alGetEnumValue(const ALubyte *name)
{
	ALuint i=0;

	while ((_al_enums[i].name) && strcmp(_al_enums[i].name, name))
	{
		i++;
	}

	return _al_enums[i].value;
}
