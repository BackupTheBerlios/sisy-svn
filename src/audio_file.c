#include <sndfile.h>
#include "audio.h"
#include "debug.h"
#include "timestamp.h"


int
audio_file_write(audio_t *audio, short int *buffer, int size)
{
  ck_err (sf_writef_short(audio->data.p, buffer, size/2) != size);

  return 0;
 error:
  return -1;
}


int
audio_file_quit(audio_t *audio)
{
  return sf_close(audio->data.p);
}


int
audio_file_init (audio_t *audio, char *device)
{
  SF_INFO sf_info;

  sf_info.frames = 0;
  sf_info.samplerate = SAMPLE_RATE;
  sf_info.channels = 1;
  sf_info.format = SF_FORMAT_AU|SF_FORMAT_PCM_16|SF_ENDIAN_FILE;//le chien
  sf_info.sections = 0;
  sf_info.seekable = 0;

  ck_err(!(audio->data.p=sf_open(device, SFM_WRITE, &sf_info)));
  audio->write = audio_file_write;
  audio->quit = audio_file_quit;

  return 0;
 error:
  return -1;
}
