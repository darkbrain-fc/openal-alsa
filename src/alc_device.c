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

#include <sys/types.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <ansidecl.h>
#include <alsa/asoundlib.h>

#include "alc_device.h"
#include "alc_context.h"
#include "alc_error.h"
		    
#define _ALC_DEF_FREQ 44100
#define _ALC_NUM_PERIODS 2
#define _ALC_BUFFER_SIZE 4096

ALvoid _alcLoadConfig(struct _AL_device *dev)
{
	char *s, buf[1024];
	FILE *fp;
	ALuint i;
	char par[16],val[16];


	if (!(s = getenv("HOME")))
	{
		return;
	}

	sprintf(buf, "%s/.openal-alsa", s);

	if (!(fp = fopen(buf, "r")))
	{
		return;
	}

	while (fgets(buf, 1024, fp))
	{
		if ((s = strchr(buf, '#')))
		{
			*s = '\0';
		}

		if (sscanf(buf, "%s %s", par,val) == 2)
		{
			if (strcmp(par,"device") == 0)
			{
				sprintf(dev->device,val);
/*				fprintf(stderr,"device : %s\n",dev->device);*/
			}
			else 
			if (strcmp(par,"channels") == 0)
			{
				i = atoi(val);
				dev->channels = i;
/*				fprintf(stderr,"channels : %d\n",dev->channels);*/
			}
			else
			if (strcmp(par,"devices") == 0)
			{
				i = atoi(val);
				dev->subdevs = i;
/*				fprintf(stderr,"devices : %d\n",dev->subdevs);*/
			}
			
		}
	}

	fclose(fp);
}


ALCboolean _alcOpenSource(AL_source *src)
{
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_info_t *info;
	snd_pcm_uframes_t size;
	AL_context *ctx = src->context;
	ALCdevice *dev = ctx->device;
	int access_type = SND_PCM_ACCESS_MMAP_INTERLEAVED;

	if (dev->count < dev->subdevs)
	{
		if (snd_pcm_open(&src->handle, dev->device, SND_PCM_STREAM_PLAYBACK,
				 SND_PCM_NONBLOCK))
			return ALC_FALSE;
	
		snd_pcm_hw_params_alloca(&hw_params);
	
		if (snd_pcm_hw_params_any(src->handle, hw_params))
			return ALC_FALSE;
	
		src->channels = dev->channels;
		snd_pcm_hw_params_set_channels(src->handle, hw_params, src->channels);
		if (src->channels > 2) access_type = SND_PCM_ACCESS_MMAP_COMPLEX;
	
		if (snd_pcm_hw_params_set_access(src->handle, hw_params,
						 access_type))
			return ALC_FALSE;
		if (snd_pcm_hw_params_set_format(src->handle, hw_params,
						 SND_PCM_FORMAT_S16))
			return ALC_FALSE;
	
		src->freq = dev->freq;
		if (snd_pcm_hw_params_set_rate_near(src->handle, hw_params,
						    &src->freq, 0))
			return ALC_FALSE;
	
		src->periods = _ALC_NUM_PERIODS;
		if (snd_pcm_hw_params_set_periods_near(src->handle, hw_params, &src->periods,0))
			return ALC_FALSE;
	
	
		size = _ALC_BUFFER_SIZE;
		if (snd_pcm_hw_params_set_buffer_size_near(src->handle, hw_params,
							   &size))
			return ALC_FALSE;
	
		
/*		fprintf(stderr,"device: %s\n",dev->device);	
		fprintf(stderr,"channels : %d\n",src->channels);
		fprintf(stderr,"refresh : %d\n",dev->refresh);
		fprintf(stderr,"buffer_size : %d\n",size);
		fprintf(stderr,"freq: %d\n",src->freq);
		fprintf(stderr,"periods : %d\n",src->periods);*/
	
		int err;
		int dir;
	
		if ((err = snd_pcm_hw_params(src->handle, hw_params))){
			fprintf(stderr,"Unable to set hwparams: %s\n", snd_strerror(err));
			return ALC_FALSE;
		}
	
	
		err = snd_pcm_hw_params_get_period_size(hw_params,&src->period_size,&dir);
		if (err < 0) {
			fprintf(stderr,"Unable to determine current swparams for playback: %s\n", snd_strerror(err));
			return ALC_FALSE;
		}
/*		fprintf(stderr,"period_size : %d\n",src->period_size);*/
	
	
		snd_pcm_info_alloca(&info);
	
		if (snd_pcm_info(src->handle, info))
			return ALC_FALSE;
		
	
		src->subdev = dev->subdevs - ++dev->count;
		
		return ALC_TRUE;
	}
	else 
	{
		return ALC_FALSE;
	}
		
}

ALCvoid _alcCloseSource(AL_source *src)
{
	AL_context *ctx = src->context;
	ALCdevice *dev = ctx->device;

	dev->count--;
	if (src->handle) snd_pcm_close(src->handle);
}

static ALCboolean _alcOpenDevice(ALCdevice *dev)
{
	snd_pcm_info_t *pcm_info;
	snd_pcm_t	*handle;
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_uframes_t size;
	ALuint freq;
	ALuint periods;
	int access_type = SND_PCM_ACCESS_MMAP_INTERLEAVED;

	snd_pcm_info_alloca(&pcm_info);
	
	/* Opening a pcm only to calculate refesh */
	if (snd_pcm_open(&handle, dev->device, SND_PCM_STREAM_PLAYBACK,
			 SND_PCM_NONBLOCK))
		return ALC_FALSE;
	snd_pcm_hw_params_alloca(&hw_params);

	if (snd_pcm_hw_params_any(handle, hw_params))
		return ALC_FALSE;
	snd_pcm_hw_params_set_channels(handle, hw_params, dev->channels);
	if (dev->channels > 2) access_type = SND_PCM_ACCESS_MMAP_COMPLEX;
				
	if (snd_pcm_hw_params_set_access(handle, hw_params,
					 access_type))
		return ALC_FALSE;
	
	if (snd_pcm_hw_params_set_format(handle, hw_params,
					 SND_PCM_FORMAT_S16))
		return ALC_FALSE;

	freq = dev->freq;
	if (snd_pcm_hw_params_set_rate_near(handle, hw_params,
						&freq, 0))
		return ALC_FALSE;
	
	periods = _ALC_NUM_PERIODS;
	if (snd_pcm_hw_params_set_periods_near(handle, hw_params, &periods,0))
		return ALC_FALSE;

	size = _ALC_BUFFER_SIZE;
	if (snd_pcm_hw_params_set_buffer_size_near(handle, hw_params,
					   &size))
		return ALC_FALSE;

	dev->refresh = (ALint)((float)freq * (float)periods / (float)size * 2.0);

	if (snd_pcm_hw_params(handle, hw_params))
		return ALC_FALSE;

	
	if (snd_pcm_info(handle, pcm_info)<0){
		fprintf(stderr,"error on getting info\n");
		return ALC_FALSE;
	}
	
	if ( dev->subdevs == 0 )
		dev->subdevs = snd_pcm_info_get_subdevices_avail(pcm_info)+1;
	snd_pcm_close(handle); /* refresh calculated, closing pcm */

	return ALC_TRUE;
}

static ALCvoid _alcCloseDevice(ALCdevice *dev)
{
	free(dev);
}

ALCdevice *alcOpenDevice(const ALubyte *spec ATTRIBUTE_UNUSED)
{
	ALCdevice *dev;

	if (!(dev = malloc(sizeof(ALCdevice))))
	{
		_alcSetError(ALC_OUT_OF_MEMORY);
		return 0;
	}

	dev->count = 0;
	dev->sync = ALC_FALSE;
	dev->freq = _ALC_DEF_FREQ;
	dev->refresh = 0; /* to calculate */
	dev->subdevs = 0;
	dev->channels = 2;
	sprintf(dev->device,"hw:0");
	
	_alcLoadConfig(dev);

	if (_alcOpenDevice(dev))
		return dev;

	_alcCloseDevice(dev);

	_alcSetError(ALC_INVALID_DEVICE);

	return 0;
}

ALCvoid alcCloseDevice(ALCdevice *dev)
{
	if (dev)
	{
		_alcCloseDevice(dev);
	}
	else
	{
		_alcSetError(ALC_INVALID_DEVICE);
	}
}
