#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef __FreeBSD__
#include <machine/soundcard.h>
#else
#include <sys/soundcard.h>
#endif
#include <assert.h>
#include <sys/time.h>/* Needed only for gettimeofday() */
#include "audio.h"
#include "debug.h"
//#include "trigger.h"
#include "timestamp.h"

//#define AUDIO_USE_ABI
#define AUDIO_USE_SELECT

int
audio_oss_write(audio_t *audio, short int *buffer, int size)
{
#ifdef  AUDIO_USE_ABI
  struct audio_buf_info abi;
#endif
#ifdef AUDIO_USE_SELECT
   fd_set fds;
   ck_err(audio->data.i < 0);
   FD_SET (audio->data.i, &fds);
#endif
#ifdef  AUDIO_USE_ABI
   do {
     ck_err(ioctl (audio->data.i, SNDCTL_DSP_GETOSPACE, &abi)<0);
   } while (abi.fragments < abi.fragstotal - 3);
#endif
#ifdef AUDIO_USE_SELECT
   select (audio->data.i + 1, 0, &fds, 0, 0);
#endif

   ck_err (write (audio->data.i, buffer, size * sizeof(*buffer)) !=
	   size * sizeof(*buffer));

   return 0;
 error:
   return -1;
}

int
audio_oss_quit (audio_t *audio)
{
  return close(audio->data.i);
}


int
audio_oss_init (audio_t *audio, char *device)
{
  int tmp;
  int audio_fd;

  debug(device);
  audio_fd = open (device, O_WRONLY);
  if (audio_fd == -1)
    return -1;

  //  tmp = 0x0004000C; // (32<<16)+13; //SIZE_OF_AUDIO_BUFFER * 4; //(2<<15)+13;
  //  tmp = (8 << 16) + 8;
  //  tmp = (0x200<<16)+6; //(2<<15)+13;
  //  if(ioctl (audio_fd, SNDCTL_DSP_SETFRAGMENT, &tmp) < 0)
  //    goto err_ioctl;

  tmp = AFMT_S16_LE;
  if(ioctl (audio_fd, SNDCTL_DSP_SETFMT, &tmp) < 0)
    goto err_ioctl;

  tmp = 0;
  if(ioctl (audio_fd, SNDCTL_DSP_STEREO, &tmp) < 0)
    goto err_ioctl;

  tmp = SAMPLE_RATE;
  if(ioctl (audio_fd, SNDCTL_DSP_SPEED, &tmp) < 0)
    goto err_ioctl;
  //  printf("audio rate: %d\n", tmp);
  smpl_rate_set(tmp);//Use the rate returned by the ioctl

  audio->write = audio_oss_write;
  audio->quit = audio_oss_quit;
  audio->data.i = audio_fd;

  return 0;
 err_ioctl:
  close(audio_fd);
  return -1;
}

