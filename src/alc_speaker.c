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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "alc_speaker.h"
#include "al_vector.h"

static AL_speaker _alcDefaultSpeakers[_ALC_NUM_SPEAKERS] =
{
	{   1.0f, { -1.0f, 1.0f, 0.0f } }, /* Front Left */
	{   1.0f, {  1.0f, 1.0f, 0.0f } }, /* Front Right */
	{   1.0f, { -1.0f, -1.0f, 0.0f } }, /* Rear Left */
	{   1.0f, {  1.0f, -1.0f, 0.0f } }, /* Rear Right */
	{   0.0f, {  0.0f, 0.0f, 0.0f } }, /* Center */
	{   0.0f, {  0.0f, 0.0f, 0.0f } }, /* LFE */
	{   0.0f, {  0.0f, 0.0f, 0.0f } }, /* Side Left */
	{   0.0f, {  0.0f, 0.0f, 0.0f } }  /* Side Right */
};

ALvoid _alcLoadSpeakers(AL_speaker *speakers)
{
	char *s, buf[1024];
	FILE *fp;
	ALuint i;
	ALfloat gain;
	ALfloat pos[3];

	memcpy(speakers, _alcDefaultSpeakers, sizeof(_alcDefaultSpeakers));

	if (!(s = getenv("HOME")))
	{
		return;
	}

	sprintf(buf, "%s/.openal-speakers", s);

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

		if (sscanf(buf, "%u: %f %f %f %f", &i, &gain,
			   pos + 0, pos + 1, pos + 2) == 5)
		{
			if (i >= _ALC_NUM_SPEAKERS)
			{
				continue;
			}

			if (gain < 0.0)
			{
				gain = 0.0;
			}
			else if (gain > 1.0)
			{
				gain = 1.0;
			}

			speakers[i].gain = gain;
			_alVectorNormalize(speakers[i].position, pos);
		}
	}

	fclose(fp);
}
