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
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
//#include <errno.h>
#include <getopt.h>
#include "debug.h"
#include "sisy.h"
#include "math.h"
#include "instru.h"
#include "audio.h"
#include "timestamp.h"

/* void* */
/* Malloc(int size) */
/* { */
/*     if(!size) */
/* 	abort(); */
/*     return calloc(size, 1); */
/* } */

//TODO : add instrument object
unsigned int dbg_filter;

static void usage (char *);
static int sisy_getopt(int ac, char **av);
static int sisy_midi_init(sisy_t *sisy, char *midi_device);
static int sisy_midi_file_init(sisy_t*, char*);
static int sisy_midi_alsa_init(sisy_t*, char*);
static int sisy_track_process(sisy_track_t *track);

char *midi_device = "alsa://sisy";
char *audio_device = "alsa://default";
//char *audio_device = "file://pouet.au";
//char *audio_device = "oss:///dev/dsp";
int midi_file_track=-1;


static symbole_t IO_symtab[]={
    {"seed", OFFSET(sisy_IO_t, seed), SIAD_SCOPE_LOCAL, SIAD_TYPE_VALUE},
    {0, 0, 0}
};


int
main (int ac, char **av)
{
    sisy_t sisy;
    bank_t bank;
    audio_t audio;
    int done=0;

    init_tab();

    memset(&sisy, 0, sizeof(sisy));

    bank.pos = 0;
    bank.watches_size = 0;
    bank.name = "sisy IO";
    bank.size = sizeof(sisy_IO_t);
    bank.data = &sisy.IO;
    bank.symtab = IO_symtab;
    bank_push(&bank);

    sisy_getopt(ac, av);

    instru_path[instru_path_size] = Malloc(strlen(getenv("HOME")) + strlen("/.sisy/") + 200);
    strcat(instru_path[instru_path_size], getenv("HOME"));
    strcat(instru_path[instru_path_size++], "/.sisy/");
    instru_path[instru_path_size++] = "/usr/share/sisy/";

    sisy.buffer = buffer_create();

    printf("Init midi: ");

    if(sisy_midi_init(&sisy, midi_device) < 0)
	{
	    fprintf(stderr, "ERROR: got some troubles while opening midi device %s\n", midi_device);
	    goto error;
	}

    printf("Ok\nInit audio: ");

    if(audio_init(&audio, audio_device) < 0)
	{
	    fprintf(stderr, "ERROR: got some troubles while opening audio device %s\n", audio_device);
	    goto error;
	}

    printf("Ok\nEnter in main loop...\n");

    while(!done)
	{
	    int t;
	    done=1;
	    ck_err(buffer_zero(sisy.buffer)<0);
//	    printf("%d\r", midi_timestamp_get());
	    for(t=0; t<sisy.nb_tracks; t++)
		{
		    sisy_track_t *track=&sisy.tracks[t];
		    if(!track->EOT)
			{
			    done=0;//We are not done !
			    ck_err(bank_push(&track->midi.bank));
			    ck_err(buffer_zero(track->buffer) < 0);

			    ck_err(sisy_track_process(track) < 0);

			    ck_err(bank_pop_check(&track->midi.bank));
			    ck_err(buffer_mix(track->buffer, sisy.buffer));
			}
		}
	    
/* 	    if(!is_buffer_flat(sisy.buffer)) */
/* 		printf("sisy: buffer qui bouge\n"); */

	    ck_err(audio_write(&audio, sisy.buffer)<0);
	}

/* 	ck_err(bank_pop_check(&sisy.midi.bank)); */
/* 	ck_err(bank_pop_check(&bank)); */

    audio_quit(&audio);

    return 0;
  error:
    return -1;
}


#define BUFF_SIZE 65535
static int
/* TOC:
 * process midi messages
 * process audio messages
 */
sisy_track_process(sisy_track_t *track)
{
    midi_msg_t mmsg;
    unsigned char chan;
    unsigned char buff[BUFF_SIZE];
    int c;

    midi_timestamp(track->midi.midi, midi_timestamp_get());

//	printf("midi_timestamp_get(): %d\n", midi_timestamp_get());
    dbg(DBG_MIDI, "sisy_track_process");

    while(midi_get_msg (track->midi.midi, &mmsg) >= 0 && !MIDI_MSG_NULL (&mmsg))
	{
	    if(dbg_filter&DBG_MIDI)
		printf("sisy_midi_track_process: %x: ", MIDI_MSG_STATUS(&mmsg));
	    if (MIDI_MSG_DATA (&mmsg))
		{
		    ck_err (midi_get_data (track->midi.midi, buff, BUFF_SIZE)<0);
		    continue;//What?
		}
	    switch(MIDI_MSG_TYPE(&mmsg))//System level
		{
		case MIDI_SYSTEM:
		    if(dbg_filter&DBG_MIDI)
			printf("system: ");
		    switch(MIDI_MSG_STATUS(&mmsg))
			{
			case MIDI_SYS_META:
			    if(dbg_filter&DBG_MIDI)
				printf("meta: %x: ", MIDI_MSG_DATA1(&mmsg));
			    switch(MIDI_MSG_DATA1(&mmsg))
				{
				case MIDI_SYS_META_TEMPO:
				    if(dbg_filter&DBG_MIDI)
					printf("tempo: %x\n", MIDI_MSG_TEMPO(&mmsg));
				    midi_tempo_set(MIDI_MSG_TEMPO(&mmsg));
				    break;
				case MIDI_SYS_META_ENDOFTRACK:
				    track->EOT=1;
				    if(dbg_filter&DBG_MIDI)
					puts("EOT");
				    break;
				default:
				    if(dbg_filter&DBG_MIDI)
					puts("default");
				}
			    break;
			default:
			    if(dbg_filter&DBG_MIDI)
				puts("default");
			}
		    break;
		case MIDI_PROG_CHNG:
		case MIDI_NOTE_ON:
		case MIDI_NOTE_OFF:
		    if(dbg_filter&DBG_MIDI)
			printf("note on|off msg: %x\n", MIDI_MSG_TYPE(&mmsg));
		    chan=MIDI_MSG_CHAN(&mmsg);
		    if(!track->chan[chan])
			ck_err(!(track->chan[chan] = instru_create("PIANO")));
		    ck_err(instru_midi_msg(track->chan[chan], &mmsg)<0);
		    break;
		default:
		    if(dbg_filter&DBG_MIDI)
			printf("default msg: %x\n", MIDI_MSG_TYPE(&mmsg));
		}
	}

    for(c=0; c<SISY_NB_CHAN; c++)
	{
	    instru_t *chan=track->chan[c];
	    if(chan)
		{
		    ck_err(buffer_zero(chan->buffer));
		    instru_process(chan, BUFFER_SIZE);
		    ck_err(buffer_mix(chan->buffer, track->buffer));
		}
	}
/*     if(!is_buffer_flat(track->buffer)) */
/* 	printf("sisy_track_process: buffer qui bouge\n"); */

    return 0;
  error:
    return -1;
}


symbole_t sisy_track_midi_IO_symtab[]={
    {"midi_volume", OFFSET(sisy_track_midi_IO_t, volume), SIAD_TYPE_VALUE},
    {0, 0, 0}
};


int
sisy_track_init(sisy_track_t *track)
{
    ck_err(!(track->buffer = buffer_create()));
    dbg(DBG_MIDI, "sisy_track_init\n");
    track->midi.bank.name = "midi track IO";
    track->midi.bank.size = sizeof(sisy_track_midi_IO_t);
    track->midi.bank.data = &track->midi.IO;
    track->midi.bank.symtab = sisy_track_midi_IO_symtab;
    track->EOT=0;

    return 0;
  error:
    return -1;
}


static int
sisy_midi_init(sisy_t *sisy, char *midi_device)
{
    if(!strncmp("alsa:", midi_device, 5))
	return sisy_midi_alsa_init(sisy, midi_device+5);
    if(!strncmp("file:", midi_device, 5))
	return sisy_midi_file_init(sisy, midi_device+5);
    return sisy_midi_file_init(sisy, midi_device);
}


int
sisy_midi_alsa_init(sisy_t *sisy, char *midi_device)
{
    sisy->nb_tracks=1;
    ck_err(!(sisy->tracks=Talloc(sisy_track_t)));
    dbg(DBG_MIDI, "sisy_midi_alsa_init\n");
    ck_err (!(sisy->tracks->midi.midi=midi_alsa_create (midi_device, MIDI_FLAG_GET)));
    ck_err (sisy_track_init(sisy->tracks) < 0);

    return 0;
  error:
    return -1;
}


int
sisy_midi_file_init(sisy_t *sisy, char *midi_device)
{
    unsigned char buff[BUFF_SIZE];
    int size, t, tpqn, done;
    midi_msg_t mmsg;
    midi_t *file;

    dbg(DBG_MIDI, "sisy_midi_file_init: start: ");
    ck_err (!(file = midi_file_create (midi_device, MIDI_FLAG_GET)));
    ck_err ((sisy->nb_tracks = midi_file_get_nb_tracks(file)) < 0);
    ck_err (!(sisy->tracks=Xalloc(sisy_track_t, sisy->nb_tracks)));
    ck_err (midi_tpqn_set(tpqn=midi_file_get_tpqn(file)));

    for(t=0; t<sisy->nb_tracks; t++)
	{
	    sisy_track_t *track=&sisy->tracks[t];
	    if(dbg_filter&DBG_MIDI)
		printf("midi new track\n");
	    ck_err(!(track->midi.midi = midi_pipe_create (0)));
	    ck_err (sisy_track_init(track) < 0);

	    done=0;
	    while(!done)
		{
		    ck_err(midi_get_msg (file, &mmsg)<0);
		    if(dbg_filter&DBG_MIDI)
			printf("sisy_midi_track_init: %x\n", MIDI_MSG_STATUS(&mmsg));
		    if (MIDI_MSG_DATA (&mmsg)) 
			{
			    ck_err ((size=midi_get_data (file, buff, BUFF_SIZE-1)) < 0);
			    buff[size]=0;
			    if(dbg_filter&DBG_MIDI)
				printf("midi data(%d): %s\n", size, buff);
			    continue;
			}
		    if(MIDI_MSG_EOT (&mmsg))
			{
			    mmsg.timestamp += tpqn*4;
			    done=1;
			}
		    ck_err(midi_put_msg(track->midi.midi, &mmsg) < 0);
		}
	    dbg(DBG_MIDI, "sisy_midi_file_init: EOT\n");
	}

    midi_destroy(file);

    return 0;
  error:
    return -1;
}


int
sisy_getopt(int ac, char **av)
{
    int i;
   
    while (1)
	{
	    //	int this_option_optind = optind ? optind : 1;
	    int option_index = 0;
	    struct option long_options[] = {
		{"help", 0, 0, 'h'},
		{"audio_dev", 1, 0, 'a'},
		{"debug", 1, 0, 'd'},
		{0, 0, 0, 0}
	    };
	
	    i = getopt_long (ac, av, "hp:d:t:a:v:", long_options, &option_index);
	    if (i == -1)
		break;
	
	    switch (i)
		{
		case 'h':
		    usage (av[0]);
		    break;

		case 'p':
		    instru_path[instru_path_size++] = strdup(optarg);
		    break;

		case 'a':
		    audio_device = strdup(optarg);
		    printf ("audio_device:%s\n", audio_device);
		    break;

		case 't':
		    midi_file_track = atoi (optarg);
		    printf("Play only track %d\n", midi_file_track);
		    if (midi_file_track < 0) goto err_usage;
		    break;

		case 'd':
		    while(*optarg)
			switch(*(optarg++))
			    {
			    case 'P': dbg_filter|=DBG_PROC; break;
			    case 'M': dbg_filter|=DBG_MIDI; break;
			    case 'B': dbg_filter|=DBG_BANK; break;
			    case 'F': dbg_filter|=DBG_PARSE; break;
			    case 'I': dbg_filter|=DBG_INSTRU; break;
			    case 'T': dbg_filter|=DBG_TIMESTAMP; break;
			    }

		default:
		    printf ("?? getopt returned character code 0%o ??\n", i);
		}
	}

    switch(ac-optind)
	{
	case 1:
	    midi_device = strdup(av[optind]);
	case 0:
	    break;
	default:
	    goto err_usage;
	}
    errno = 0;

    return 0;
  err_usage:
    usage(av[0]);
    return -1;
}


void usage (char *name)
{
    printf ("usage:%s [options] <input midi file>\n", name);
    puts ("");
    puts (" Options:");
    puts ("   -h, --help");
    puts ("   -d, --debug PMBFIT     : Proc, Midi, Bank, Parse, Instru, Timestamp.");
    puts ("");
    puts ("   -a, --audio_dev <path> : Set output audio device");
    puts ("");
    puts (" Output audio device examples:");
    puts ("  To output on the default alsa audio device:\t\"alsa://default\"");
    puts ("  To output on the OSS audio device /dev/dsp:\t\"oss:///dev/dsp\"");
    puts ("  To output in a \"snd\" file name pouet.au:\t\"file://pouet.au\"");
    puts ("  To output in a \"snd\" file name pouet.au:\t\"pouet.au\"");
    puts ("");
    puts (" Input midi file examples:");
    puts ("  To listen on a alsa port name sisy:\t\"alsa://sisy\"");
    puts ("  To read a file name tadadada.midi:\t\"file://tadadadaaa.midi");
    puts ("  To read a file name tadadada.midi:\t\"tadadadaaa.midi");
    exit (0);
}
