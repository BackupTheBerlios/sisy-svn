#include <unistd.h>
#include <stdio.h>
#include "instru.h"
#include "debug.h"
#include "misc.h"
#include "bank.h"
#include "instru_parse.h"
#include "timestamp.h"
#include "module.h"
#include "math.h"

char *instru_path[6];
int instru_path_size=0;

#define TIMEOUT

//IDEA:dynamic allocation of voices
//=>create_module when needed
//=>multiplexage

extern instru_t *instru_create();

extern int instru_voice_init(instru_voice_t * voice, instru_midi_t * mc,
			     int vi);
static int instru_midi_init(instru_midi_t * mc);
//static int instru_instru_voice_midi_init(instru_t *instru);

static int instru_prog(instru_t * instru, char prog);
//static int instru_voice_process(instru_voice_t * voice);
//static int instru_midi_process(instru_t * instru);
//static int instru_instru_voice_midi_process(instru_t *instru);

static int instru_voice_midi_process(instru_voice_t * iv);
int instru_voice_midi_init(instru_voice_midi_t * ivm, instru_midi_t * im, int voice);


symbole_t instru_IO_symtab[] = {
    {"polyphony_type",		OFFSET(instru_IO_t, poly_type), SIAD_SCOPE_GLOBAL, SIAD_TYPE_VALUE},
    {"polyphony_nb_voices",	OFFSET(instru_IO_t, nb_voices), SIAD_SCOPE_GLOBAL, SIAD_TYPE_VALUE},
    {0, 0, 0, 0}
};

symbole_t instru_voice_IO_symtab[] = {
    {"timestamp",	OFFSET(instru_voice_IO_t, timestamp), SIAD_SCOPE_OUT, SIAD_TYPE_VALUE},//Who cares ?
    {"audio_out",	OFFSET(instru_voice_IO_t, buffer),    SIAD_SCOPE_OUT, SIAD_TYPE_BUFFER},
    {0, 0, 0, 0}
};

symbole_t instru_midi_IO_symtab[] = {
    //  {"midi_ctrl_cont", OFFSET(instru_midi_IO_t, ctrl_cont), SIAD_TYPE_ARRAY},
    //  {"midi_ctrl_switch", OFFSET(instru_midi_IO_t, ctrl_switch), SIAD_TYPE_ARRAY},
    //  {"midi_ctrl_byte", OFFSET(instru_midi_IO_t, ctrl_byte), SIAD_TYPE_ARRAY},
    {"midi_pitch",		OFFSET(instru_midi_IO_t, pitch),      SIAD_SCOPE_IN, SIAD_TYPE_VALUE},
    {"midi_char program",	OFFSET(instru_midi_IO_t, program),    SIAD_SCOPE_IN, SIAD_TYPE_VALUE},
    {"midi_aftertouch",		OFFSET(instru_midi_IO_t, aftertouch), SIAD_SCOPE_IN, SIAD_TYPE_VALUE},
    {0, 0, 0, 0}
};

symbole_t instru_voice_midi_IO_symtab[] = {
    {"midi_note",         OFFSET(instru_voice_midi_IO_t, note),         SIAD_SCOPE_IN, SIAD_TYPE_VALUE},
    {"midi_trigger", OFFSET(instru_voice_midi_IO_t, note_trigger),	SIAD_SCOPE_IN, SIAD_TYPE_VALUE},
    {"midi_freq",    OFFSET(instru_voice_midi_IO_t, note_freq),    SIAD_SCOPE_IN, SIAD_TYPE_VALUE},
    {"midi_period",  OFFSET(instru_voice_midi_IO_t, note_period),  SIAD_SCOPE_IN, SIAD_TYPE_VALUE},
    {"midi_velo",         OFFSET(instru_voice_midi_IO_t, velo),         SIAD_SCOPE_IN, SIAD_TYPE_VALUE},
    {"midi_aftertouch",   OFFSET(instru_voice_midi_IO_t, aftertouch),   SIAD_SCOPE_IN, SIAD_TYPE_VALUE},
    {0, 0, 0, 0}
};


instru_t*
instru_create()
{
    instru_t *instru = 0;
    ck_err(!(instru = Talloc(instru_t)));

    instru->bank_USER.name = "instru USER";
    instru->bank_IO.name = "instru IO";
    instru->bank_IO.size = sizeof(instru_IO_t);
    instru->bank_IO.data = &instru->IO;
    instru->bank_IO.symtab = instru_IO_symtab;

    ck_err(!(instru->buffer = buffer_create()));
    ck_err(instru_midi_init(&instru->midi) < 0);
    ck_err(instru_prog(instru, 0) < 0);
    return instru;
  error:
    return NULL;
}


int
instru_destroy(instru_t * instru)
{
    //FIXME: destroy it
    return 0;
}


int
instru_midi_msg(instru_t *instru, midi_msg_t *mmsg)
{
    unsigned char ctrl;

    if (dbg_filter & DBG_MIDI)
	printf("instru_midi: mmsg_count: %d\n",
	       instru->midi.mmsg_count++);
    ck_err(MIDI_MSG_DATA(mmsg));
    switch (MIDI_MSG_TYPE(mmsg)) {
    case MIDI_CONTROL:
	ctrl = MIDI_MSG_CTRL(mmsg);
	if (ctrl < 32)
	    instru->midi.IO.ctrl_cont[ctrl] =
		(short) (MIDI_MSG_VALUE(mmsg) << 7);
	else if (ctrl < 64)
	    instru->midi.IO.ctrl_cont[ctrl] |=
		(short) (MIDI_MSG_VALUE(mmsg));
	break;
    case MIDI_PROG_CHNG:
	if (dbg_filter & DBG_MIDI)
	    printf("MIDI_PROG_CHNG: prog:%d\n", MIDI_MSG_PRGM(mmsg));
	instru_prog(instru, MIDI_MSG_PRGM(mmsg));
	break;
    case MIDI_CHAN_TOUCH:
    case MIDI_PITCHBEND:
	break;
    case MIDI_NOTE_ON:
    case MIDI_NOTE_OFF:
	//FIXME: process midi
	dbg(DBG_INSTRU | DBG_MIDI, "instru_midi_process: mmsg");
	//printf("instru->midi.mmux: %p, ", instru->midi.mmux);
	if (instru->midi.mmux)
	    ck_err(midi_put_msg(instru->midi.mmux, mmsg) < 0);
	//printf("instru->global.midi.midi: %p\n", instru->global.midi.midi);
	if (instru->global.midi.midi)
	    ck_err(midi_put_msg(instru->global.midi.midi, mmsg) < 0);
    default:
	break;
    }

    return 0;
  error:
    return -1;
}


//PROCESS
static int
instru_voice_process(instru_voice_t * voice, int size)
{
    module_t *module;
    node_t *node;

    dbg(DBG_PROC, "instru_voice_process");
    ck_err(instru_voice_midi_process(voice) < 0);

#ifdef TIMEOUT
    if ((voice->timeout -= size) <= 0)
	return 0;
#endif

    ck_err(buffer_zero(voice->IO.buffer) < 0);

    bank_push(&voice->midi.bank);
    bank_push(&voice->bank_IO);
    bank_push(&voice->bank_USER);

    for (node = voice->modules.first; node; node = node->next)
	{
	    module = MODULE(node->data);
	    bank_push(&module->IO);
	    module_process(module, size);
	    ck_err(bank_pop_check(&module->IO));
	}

    ck_err(bank_pop_check(&voice->bank_USER));
    ck_err(bank_pop_check(&voice->bank_IO));
    ck_err(bank_pop_check(&voice->midi.bank));

/*     if(!is_buffer_flat(voice->IO.buffer)) */
/* 	printf("instru_voice_process: buffer qui bouge\n"); */

    return 0;
  error:
    return -1;
}


int
instru_process(instru_t * instru, int size)
{
    int i;

    dbg(DBG_PROC, "instru_process");
//	ck_err(instru_midi_process(instru) < 0);
    ck_err(buffer_zero(instru->buffer) < 0);	//One more tiiiime !

    bank_push(&instru->midi.bank);
    bank_push(&instru->bank_IO);
    bank_push(&instru->bank_USER);

    for (i = 0; i < instru->IO.nb_voices; i++)
	{
	    ck_err(instru_voice_process(&instru->voices[i], size) < 0);
#ifdef TIMEOUT
	    if (instru->voices[i].timeout > 0)
#endif
		ck_err(buffer_mix(instru->voices[i].IO.buffer, instru->buffer) < 0);
	}

    if (instru->global.IO.buffer) {
	ck_err(buffer_zero(instru->global.IO.buffer) < 0);
	ck_err(instru_voice_process(&instru->global, size) < 0);
	ck_err(buffer_mix(instru->global.IO.buffer, instru->buffer) < 0);
    }

    ck_err(bank_pop_check(&instru->bank_USER));
    ck_err(bank_pop_check(&instru->bank_IO));
    ck_err(bank_pop_check(&instru->midi.bank));

    return 0;
  error:
    return -1;
}


//Per channel process, all received events are addressed to this chan
static int
instru_voice_midi_process(instru_voice_t * iv)
{
    midi_msg_t mmsg;
    instru_voice_midi_t *voice;

    dbg(DBG_INSTRU | DBG_MIDI, "instru_voice_midi_process");
    ck_err(!iv || !iv->midi.midi);
    voice = &iv->midi;
    midi_timestamp(voice->midi, midi_timestamp_get());
    while (midi_get_msg(voice->midi, &mmsg) >= 0 && !MIDI_MSG_NULL(&mmsg)) {
	if(dbg_filter&DBG_MIDI)
	    printf("instru_voice_midi_process: %x: ", MIDI_MSG_STATUS(&mmsg));
	iv->midi.mmsg_count++;
	//       printf("MIDI VOICE MSG\n");
	ck_err(MIDI_MSG_DATA(&mmsg));
	switch (MIDI_MSG_TYPE(&mmsg)) {
	case MIDI_NOTE_ON:
	    iv->timeout = SAMPLE_RATE*60*6;//6 minutes
	    //      printf("MIDI NOTE ON: %d\n", MIDI_MSG_CHAN(&mmsg));
	    if (dbg_filter & DBG_MIDI)
		printf("#%03d:NOTE ON:  chan:%2x, note:%2x, velo:%2x: ",
		       iv->midi.mmsg_count, MIDI_MSG_CHAN(&mmsg),
		       MIDI_MSG_NOTE(&mmsg), MIDI_MSG_VELO(&mmsg));
	    voice->IO.note_trigger++;//if 2 notes are played without setting trigger to 0...
	    voice->IO.note = MIDI_MSG_NOTE(&mmsg);
	    voice->IO.velo = MIDI_MSG_VELO(&mmsg)<<8;//fxd:15
	    voice->IO.note_period = note2smpl(MIDI_MSG_NOTE(&mmsg));
	    voice->IO.note_freq = note2freq(MIDI_MSG_NOTE(&mmsg));
	    break;
	case MIDI_NOTE_OFF:
	    if (dbg_filter & DBG_MIDI)
		printf("#%03d:NOTE OFF: chan:%2x, note:%2x, velo:%2x: ",
		       iv->midi.mmsg_count, MIDI_MSG_CHAN(&mmsg),
		       MIDI_MSG_NOTE(&mmsg), MIDI_MSG_VELO(&mmsg));
	    iv->timeout = SAMPLE_RATE*2;//just 2 sec.
	    voice->IO.note_trigger=0;
	    voice->IO.velo = MIDI_MSG_VELO(&mmsg)<<8;//fxd:15
	    voice->IO.note = MIDI_MSG_NOTE(&mmsg);//meaning full for global *voice*
	    break;
	default:
	    if (dbg_filter & DBG_MIDI)
		printf("POUEEEEET ! type:%d\n", MIDI_MSG_TYPE(&mmsg));
	}
	//   voice->IO.velo=100;
    }
    return 0;
  error:
    return -1;
}

//INIT
int
instru_voice_init(instru_voice_t * voice, instru_midi_t * im, int vi)
{
    memset(voice, 0, sizeof(instru_voice_t));
    ck_err(!(voice->IO.buffer = buffer_create()));
    ck_err(instru_voice_midi_init(&voice->midi, im, vi));
    voice->bank_IO.name = "instru voice IO";
    voice->bank_IO.size = sizeof(instru_voice_IO_t);
    voice->bank_IO.data = &voice->IO;
    voice->bank_IO.symtab = instru_voice_IO_symtab;
    voice->bank_USER.name = "voice->bank_USER";
	return 0;
  error:
    return -1;
}


int
instru_voice_copy(instru_voice_t * src, instru_voice_t * dst)
{
    node_t *nodule;
    //FIXME:copy IOs
    //  dst->IO=src->IO;!!!surtout pas ca!!!
    dst->midi.IO = src->midi.IO;
    ck_err(bank_copy(&src->midi.bank,  &dst->midi.bank) < 0);
    ck_err(bank_copy(&src->bank_IO,    &dst->bank_IO)   < 0);
    ck_err(bank_clone(&src->bank_USER, &dst->bank_USER) < 0);

    for (nodule = src->modules.first; nodule; nodule = nodule->next) {
	module_t *module;
	node_t *node;

	ck_err(!(module = module_clone((module_t*)nodule->data)));
	ck_err(!(node = node_create(module)));
	ck_err(list_add_end(&dst->modules, node) < 0);
    }

    dst->affects = src->affects;
    return 0;
  error:
    return -1;
}


int
instru_midi_init(instru_midi_t * im)
{
    dbg(DBG_MIDI, "instru_midi_init");
    ck_err(!(im->midi = midi_pipe_create(MIDI_FLAG_RT)));

    im->bank.name = "instru midi IO";
    im->bank.size = sizeof(instru_midi_IO_t);
    im->bank.data = &im->IO;
    im->bank.symtab = instru_midi_IO_symtab;

    return 0;
  error:
    return -1;
}


int
instru_midi_mux_init(instru_midi_t * im, int nb_voices)
{
    dbg(DBG_MIDI, "instru_midi_mux_init");
    ck_err(!(im->mmux = midi_mux_create(nb_voices, MIDI_FLAG_RT)));

    return 0;
  error:
    return -1;
}


int
instru_voice_midi_init(instru_voice_midi_t * ivm, instru_midi_t * im,
		       int voice)
{
    dbg(DBG_MIDI, "instru_voice_midi_init");

    if (im)
	ck_err(!(ivm->midi = midi_mux_get_voice(im->mmux, voice, MIDI_FLAG_RT)));
    else
	ck_err(!(ivm->midi = midi_pipe_create(MIDI_FLAG_RT)));

    ivm->bank.name = "midi voice IO";
    ivm->bank.size = sizeof(instru_voice_midi_IO_t);
    ivm->bank.data = &ivm->IO;
    ivm->bank.symtab = instru_voice_midi_IO_symtab;

    return 0;
  error:
    return -1;
}


//af377560ae68925c4033c8d22d6cf066  nestat.anp
//9a1981debd642968c2c0b6007ef39167  ps.axuf
//185aa89f9e355686ec17282af85ba5cb  iptables.vL


//INIT


char *prog_name(char prog)
{
    switch (prog & 0x78) 
	{
	case 0x00: return "PIANO";
	case 0x08: return "CHROM_PERCUSSION";
	case 0x10: return "ORGAN";
	case 0x18: return "GUITAR";

	case 0x20: return "BASS";
	case 0x28: return "STRINGS";
	case 0x30: return "ENSEMBLE";
	case 0x38: return "BRASS";

	case 0x40: return "REED";
	case 0x48: return "PIPE";
	case 0x50: return "SYNTH_LEAD";
	case 0x58: return "SYNTH_PAD";

	case 0x60: return "SYNTH_EFFECTS";
	case 0x68: return "ETHNIC";
	case 0x70: return "PERCUSSIVE";
	case 0x78: return "SOUND_EFFECTS";
	}
    return 0;
}

int
strncatsucks(char *dst, char *src, int dst_size)
{
    if(strlen(dst) + strlen(src) >= dst_size)
	return -1;
    strncat(dst, src, dst_size - strlen(dst));
    return 0;
}


//free the result
char*
instru_file_name(char *name, char *file_name, int file_name_size)
{
    int i;
    for(i=0; i<instru_path_size; i++)
	{
	    file_name[0] = 0;
	    ck_err(strncatsucks(file_name, instru_path[i], file_name_size) < 0);
	    ck_err(strncatsucks(file_name, "/",   file_name_size) < 0);
	    ck_err(strncatsucks(file_name, name,  file_name_size) < 0);
	    if(!access(file_name, R_OK))
		return file_name;
	}
  error:
    return 0;
}


#define FILE_NAME_SIZE 256


int
instru_prog(instru_t * instru, char prog)
{
    char *name;
    char file_name[FILE_NAME_SIZE];

    instru->midi.IO.program = prog;
    ck_err(!(name = prog_name(prog)));
    ck_err(!(name = instru_file_name(name, file_name, FILE_NAME_SIZE)));
//  if (dbg_filter & DBG_MIDI)
	printf("instru_prog: prog #%x, file: %s\n", prog, name);
    //   abort();
    ck_err(instru_parse(instru, name) < 0);
    return 0;
  error:
    return -1;
}
