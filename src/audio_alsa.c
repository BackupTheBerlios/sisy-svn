#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include "audio.h"
#include "debug.h"


int
audio_alsa_quit(audio_t *audio)
{
  return snd_pcm_close ((snd_pcm_t*)audio->data.p);
}

int
audio_alsa_write(audio_t *audio, short int *buffer, int size)
{
  ck_err(snd_pcm_writei ((snd_pcm_t*)audio->data.p, buffer, size) != size);

  return 0;
 error:
  return -1;
}

int
audio_alsa_init(audio_t *audio, char *audio_device)
{
  snd_pcm_hw_params_t *hw_params;
  snd_pcm_t *playback_handle;
  int rate=SAMPLE_RATE;

  ck_err(snd_pcm_open (&playback_handle, audio_device, SND_PCM_STREAM_PLAYBACK, 0) < 0);
  ck_err(snd_pcm_hw_params_malloc (&hw_params) < 0);
  ck_err(snd_pcm_hw_params_any (playback_handle, hw_params) < 0);
  ck_err(snd_pcm_hw_params_set_access (playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0);
  ck_err(snd_pcm_hw_params_set_format (playback_handle, hw_params, SND_PCM_FORMAT_S16_LE) < 0);
  ck_err(snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, &rate, 0) < 0);
  ck_err(snd_pcm_hw_params_set_channels (playback_handle, hw_params, 1) < 0);
  ck_err(snd_pcm_hw_params (playback_handle, hw_params) < 0);
  snd_pcm_hw_params_free (hw_params);
  ck_err(snd_pcm_prepare (playback_handle) < 0);

  audio->write = audio_alsa_write;
  audio->quit = audio_alsa_quit;
  audio->data.p = playback_handle;

  return 0;
 error:
  return -1;
}
