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
#ifndef _SISY_H
#define _SISY_H

//#include "sisy_midi.h"
#include "instru.h"

#define SISY_NB_CHAN 16

typedef struct {
   int seed;
} sisy_IO_t;

//FIXME:change int to char where it should be good
typedef struct {
  int volume;//MOVE !
} sisy_track_midi_IO_t;

typedef struct {
  midi_t *midi;
  bank_t bank;
  sisy_track_midi_IO_t IO;
} sisy_track_midi_t;

typedef struct sisy_track_t sisy_track_t;

struct sisy_track_t {
  sisy_track_midi_t midi;
  instru_t *chan[SISY_NB_CHAN];
  sisy_IO_t IO;
  buffer_t *buffer;
  int EOT;
};

typedef struct {
  sisy_track_t *tracks;
  int nb_tracks;
  sisy_IO_t IO;
  buffer_t *buffer;
} sisy_t;


void init_tab ();


#endif
