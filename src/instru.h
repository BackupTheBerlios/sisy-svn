#ifndef _SISY_INSTRU_H_
#define _SISY_INSTRU_H_


//#include "sisy_midi.h"
#include "module.h"
#include "list.h"
#include "bank.h"
#include "audio.h"


//VOICE
typedef struct {
  buffer_t *buffer;
  int timestamp;
} instru_voice_IO_t;

typedef struct {
  //  trigger_t *note_on_trig;
  //  trigger_t *note_off_trig;
  int note;
  int note_trigger;
  int note_freq;
  int note_period;
  int velo;
  int aftertouch;
} instru_voice_midi_IO_t;

typedef struct {
  midi_t *midi;
  bank_t bank;
  instru_voice_midi_IO_t IO;
  int mmsg_count;
} instru_voice_midi_t;

typedef struct {
  instru_voice_IO_t IO;
  instru_voice_midi_t midi;
  bank_t bank_IO; 
  bank_t bank_USER;
  list_t modules;
  list_t affects;
  int timeout;
} instru_voice_t;

//INSTRU
typedef struct {
  int poly_type;
  int nb_voices;
} instru_IO_t;

typedef struct {
  short pitch;
  short ctrl_cont[32];
  char ctrl_switch[6];
  char ctrl_byte[25];
  //CTRL UNDEFINED
  int program;
  int aftertouch;
} instru_midi_IO_t;

typedef struct {
  midi_t *midi;
  midi_t *mmux;
  bank_t bank;
  instru_midi_IO_t IO;
  int mmsg_count;
} instru_midi_t;

typedef struct {
  //channel defaults
  instru_IO_t IO;
  instru_voice_t *voices;
  instru_voice_t global;
  buffer_t *buffer;
  bank_t bank_IO;
  bank_t bank_USER;
  instru_midi_t midi;
  char *name;
  list_t affects;
  char chanum;//For file, channel number of the instru
} instru_t;


extern instru_t *instru_create();
extern int instru_midi_msg(instru_t *instru, midi_msg_t *mmsg);
extern int instru_process(instru_t *instru, int size);
extern int instru_init(instru_voice_t*);
extern int instru_mux_init(instru_t*, int);
extern int instru_destroy(instru_t *instru);
extern int instru_voice_copy(instru_voice_t *src, instru_voice_t *dst);
int instru_voice_init(instru_voice_t * voice, instru_midi_t * im, int vi);
int instru_voice_midi_init(instru_voice_midi_t * ivm, instru_midi_t * im, int voice);
int instru_midi_mux_init(instru_midi_t * im, int nb_voices);

extern instru_t *global_instru;
extern int siad_size[SIAD_NB_TYPE];
extern symbole_t instru_IO_symtab[];
extern symbole_t instru_voice_IO_symtab[];

extern char *instru_path[6];
extern int instru_path_size;


#endif
