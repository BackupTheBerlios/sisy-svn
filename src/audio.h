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
#ifndef	_AUDIO_H
#define	_AUDIO_H

#include <limits.h>

#define SMPL_MIN SHRT_MIN
#define SMPL_MAX SHRT_MAX

#define	FRAG_SIZE 64
#define BUFFER_SIZE 32
#define SAMPLE_RATE 22050
//#define SAMPLE_RATE 44100

typedef int smpl_t;

typedef struct {
   smpl_t *smpl;
   int size;
} buffer_t;

struct audio_t;
typedef int audio_write_t(struct audio_t*, short int*, int);
typedef int audio_quit_t(struct audio_t*);

typedef struct audio_t {
  union {
    int i;
    void *p;
  } data;
  audio_write_t *write;
  audio_quit_t *quit;
} audio_t;

int audio_init (audio_t*, char *);
int audio_write (audio_t*, buffer_t*);
int audio_quit (audio_t*);

buffer_t *buffer_create();
int buffer_zero(buffer_t*);
int buffer_mix(buffer_t* , buffer_t*);
int buffer_valeff(buffer_t*);
int is_buffer_flat(buffer_t*);
//#define SCAN_AUDIO_BUFFER

#endif
