SUBDIRS=modules

bin_PROGRAMS=sisy
sisy_SOURCES=					\
parse.y lex.l					\
audio.c audio.h					\
audio_alsa.c audio_oss.c audio_file.c		\
sisy.c sisy.h					\
bank.c bank.h					\
instru.c instru.h				\
module.c module.h				\
timestamp.c timestamp.h				\
instru_parse.c instru_parse.h			\
list.c list.h					\
obj.c obj.h					\
math.c math.h					\
debug.h misc.h					\
oscillators.c oscillators.h

#trigger.c trigger.h

AM_YFLAGS =  -d

sisy_LDADD=@LIBSNDFILE_LIBS@ @SIMILI_LIBS@ modules/libmodules.a
sisy_CFLAGS=@LIBSNDFILE_CFLAGS@ @SIMILI_CFLAGS@
