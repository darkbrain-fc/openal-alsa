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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <AL/alut.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN

#define WAV_RIFF 0x46464952
#define WAV_WAVE 0x45564157
#define WAV_fmt  0x20746D66
#define WAV_data 0x61746164
#define WAV_smpl 0x6C706D73

#define swap16le(D) (D)
#define swap32le(D) (D)

#elif __BYTE_ORDER == __BIG_ENDIAN

#define WAV_RIFF 0x52494646
#define WAV_WAVE 0x57415645
#define WAV_fmt  0x666D7420
#define WAV_data 0x64617461
#define WAV_smpl 0x736D706C

#define swap16le(D) (((D)<<8) | ((D)>>8))
#define swap32le(D) ((((D)<<24) | (((D)<<8)&0x00FF0000) | (((D)>>8)&0x0000FF00) | ((D)>>24)))

#else
#error "Unknown endian"
#endif

typedef struct
{
	u_int32_t id;
	u_int32_t size;
	u_int32_t type;
	u_int32_t *data;
	u_int32_t len;
}
AL_wav_file;

typedef struct
{
	u_int32_t id;
	u_int32_t size;
	u_int32_t *data;
}
AL_wav_chunk;

static ALboolean _alutReadWavHeader(AL_wav_file *file, u_int32_t *memory)
{
	if ((file->id = *(memory++)) != WAV_RIFF)
	{
		return AL_FALSE;
	}

	file->size = swap32le(*(memory++));

	if (file->size < 4)
	{
		return AL_FALSE;
	}

	if ((file->type = *(memory++)) != WAV_WAVE)
	{
		return AL_FALSE;
	}

	file->data = memory;
	file->len = (file->size - 1) >> 2;

	return AL_TRUE;
}

static ALboolean _alutReadWavChunk(AL_wav_file *file, AL_wav_chunk *chunk)
{
	ALuint len;

	if (file->len < 2)
	{
		return AL_FALSE;
	}

	chunk->id = file->data[0];
	chunk->size = swap32le(file->data[1]);

	len = (chunk->size + 11) >> 2;

	if (file->len < len)
	{
		return AL_FALSE;
	}

	chunk->data = file->data + 2;

	file->data += len;
	file->len -= len;

	return AL_TRUE;
}

ALvoid alutLoadWAVMemory(ALbyte *memory, ALenum *format, ALvoid **data,
			 ALsizei *size, ALsizei *freq, ALboolean *loop)
{
	AL_wav_file file;
	AL_wav_chunk chunk;
	ALuint bits = 0;

	*data = 0;
	*size = 0;
	*freq = 0;
	*format = 0;
	*loop = 0;

	if (!_alutReadWavHeader(&file, (u_int32_t *)memory))
	{
		return;
	}

	while (_alutReadWavChunk(&file, &chunk))
	{
		u_int16_t *data16 = (u_int16_t *)chunk.data;

		switch (chunk.id)
		{
		case WAV_fmt:
			if (data16[0] != 1)
			{
				return;
			}
				
			bits = swap16le(data16[7]);
			*freq = swap32le(chunk.data[1]);

			switch (swap16le(data16[1]))
			{
			case 1:
				switch (bits)
				{
				case 8:
					*format = AL_FORMAT_MONO8;
					break;
				case 16:
					*format = AL_FORMAT_MONO16;
					break;
				default:
					return;
				}
				break;
			case 2:
				switch (bits)
				{
				case 8:
					*format = AL_FORMAT_STEREO8;
					break;
				case 16:
					*format = AL_FORMAT_STEREO16;
					break;
				default:
					return;
				}
				break;
			default:
				return;
			}
			break;
		case WAV_data:
			if (!bits)
			{
				return;
			}

			*data = malloc(chunk.size);
			*size = chunk.size;

#if __BYTE_ORDER == __LITTLE_ENDIAN
			memcpy(*data, chunk.data, chunk.size);
#else
			if (bits == 16)
			{
				u_int16_t *ptr = *data;
				ALuint i, s = chunk.size << 1;

				for (i = 0; i < s; i++)
				{
					ptr[i] = swap16le(data16[i]);
				}
			}
			else
			{
				memcpy(*data, chunk.data, chunk.size);
			}
#endif
			break;
		case WAV_smpl:
			*loop = swap32le(chunk.data[7]);
			break;
		}
	}
}

ALvoid alutLoadWAVFile(ALbyte *file, ALenum *format, ALvoid **data,
		       ALsizei *size, ALsizei *freq, ALboolean *loop)
{
	int fd;
	void *memory;
	struct stat st;


	if ((fd = open(file, O_RDONLY)) < 0)
	{
		return;
	}

	if (!fstat(fd, &st))
	{
		memory = mmap(0, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
		if (memory != MAP_FAILED)
		{
			alutLoadWAVMemory(memory, format, data,
					   size, freq, loop);
			munmap(memory, st.st_size);
		}
	}

	close(fd);
}

ALboolean alutLoadWAV(const char *fname, ALvoid **wave, ALsizei *format, ALsizei *size, ALsizei *bits, ALsizei *freq )
{
	ALboolean loop;

	alutLoadWAVFile((ALubyte *)fname, format, wave, size, freq, &loop);

	if (! *wave)
	{
		return AL_FALSE;
	}

	switch (*format)
	{
	case AL_FORMAT_MONO8:
	case AL_FORMAT_STEREO8:
		*bits = 8;
		break;
	case AL_FORMAT_MONO16:
	case AL_FORMAT_STEREO16:
		*bits = 16;
		break;
	}

	return AL_TRUE;
}

ALvoid alutUnloadWAV(ALenum format ATTRIBUTE_UNUSED, ALvoid *data,
		     ALsizei size ATTRIBUTE_UNUSED,
		     ALsizei freq ATTRIBUTE_UNUSED)
{
	if (data)
	{
		free(data);
	}
}
