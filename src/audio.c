/*
 *  Copyright (C) 2001 Frederic Motte
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "audio.h"
#include "debug.h"
//#include "trigger.h"
#include "timestamp.h"


#ifdef ALSA
int audio_alsa_init(audio_t *audio, char *audio_device);
#endif
#ifdef OSS
int audio_oss_init(audio_t *audio, char *audio_device);
#endif
#ifdef FILE
int audio_file_init(audio_t *audio, char *audio_device);
#endif


int
audio_init(audio_t *audio, char *device)
{
  if(!strncmp("file://", device, 6))
    return audio_file_init(audio, device+7);
  if(!strncmp("alsa://", device, 7))
    return audio_alsa_init(audio, device+7);
  if(!strncmp("oss://", device, 6))
    return audio_oss_init(audio, device+6);
  return audio_file_init(audio, device);
}

int
audio_write (audio_t *audio, buffer_t *buffer)
{
  int i;
  int overflow = 0;
  smpl_t si;
  short int *sc_buffer;

  //buffer_zero(buffer);
  //buffer_buzz(buffer);

  sc_buffer = alloca(buffer->size*sizeof(*sc_buffer));

  //let's say that its just a strong compression
  for (i=0; i < buffer->size; i++)
    {
      si = buffer->smpl[i];
      if (si < SMPL_MIN)
	{
	  overflow = 1;
	  si = SMPL_MIN;
	}
      if (si > SMPL_MAX)
	{
	  overflow = 1;
	  si = SMPL_MAX;
	}
      sc_buffer[i] = si;
    }

  ck_err(audio->write(audio, sc_buffer, buffer->size) < 0);

  smpl_timestamp_add(buffer->size);

  return overflow;
 error:
  return -1;
}


int
audio_quit (audio_t *audio)
{
  ck_err(!audio || !audio->quit);
  return audio->quit(audio);
 error:
  return -1;
}


buffer_t*
buffer_create()
{
  buffer_t *buffer;

  ck_err(!(buffer = Talloc(buffer_t)));
  //  printf("buffer_create: %p\n", buffer);
  ck_err(!(buffer->smpl = Xalloc(smpl_t, BUFFER_SIZE)));
  buffer->size = BUFFER_SIZE;
  return buffer;
 error:
  return 0;
}


int
buffer_zero(buffer_t *buffer)
{
  ck_err(!buffer);
  memset(buffer->smpl, 0, sizeof(smpl_t) * buffer->size);
  return 0;
 error:
  return -1;
}

int
buffer_mix(buffer_t *src , buffer_t *dst)
{
  int i;
  smpl_t *ss, *sd;

  ck_err(!src || !dst);

  ss=src->smpl;
  sd=dst->smpl;

  for(i=0; i<dst->size; i++)
    sd[i]+=ss[i];
  //    *(sd++)+=*(ss++);

  return 0;
 error:
  return -1;
}


#define BUZZ 100

int
buffer_buzz(buffer_t *buff)
{
  int i;
  static int o;
   
  ck_err(!buff);
  for(i=0; i<buff->size; i++, o++)
    {
      if(o>BUZZ/2)
	buff->smpl[i]=30000;
      else
	buff->smpl[i]=-30000;
      if(o>BUZZ)
	o=0;
    }
   
  return 0;
 error:
  return -1;
}


int
buffer_valeff(buffer_t *buff)
{
  int valeff=0, i;

  for(i=0; i<buff->size; i++)
    {
      valeff+=buff->smpl[i]*buff->smpl[i];
    }
  return valeff/buff->size;
}
