#include "timestamp.h"
#include "audio.h"
#include "debug.h"
int midi_timestamp=0;
int smpl_timestamp=0;
int tempo=120;//us/QN:tempo/15000:N/min
int tpqn=480;// 1/QN:
int rate=SAMPLE_RATE;//1/s

//s->S:
//[tempo]=ms/QN=tpqn/(1000000*rate)=(1/QN)/(1000000/s)
//tpqn/tempo/SR
//=([T]/QN)*([t]*1000*QN/s)*(s/[S])
//=([T])*([t]*1000)*([S])
//=1000*T*S,

//ticks->samples
//ticks->QN->Sample
//ticks*(tempo*rate/tpqn)->samples

//ticks/tpqn->QN*tempo->us/1000000->s*rate->samples
//(ticks*(tempo*rate/tpqn)/1000)/1000->samples

int
smpl_rate_set(int r)
{
    rate=r;
    return 0;
}

int
midi_tempo_set(int uspqn)
{
    if(!uspqn)
	return -1;
    tempo=60000000/uspqn;
    return 0;
}

int
midi_tpqn_set(int t)
{
    tpqn=t;
    return 0;
}

////TIMESTAMP
//ADD
int
smpl_timestamp_add(int offset)
{
    if(dbg_filter&DBG_TIMESTAMP)
	printf("smpl_timestamp_add: %d, %d, %d\n", offset, smpl_timestamp, timestamp_smpl2midi(smpl_timestamp));
    smpl_timestamp += offset;
    midi_timestamp = timestamp_smpl2midi(smpl_timestamp);
    //  printf("smpl_timestamp: %d, %d\n", smpl_timestamp, midi_timestamp);
    return 0;
}

int
midi_timestamp_add(int offset)
{
    if(dbg_filter&DBG_TIMESTAMP)
	printf("midi_timestamp_add: %d, %d, %d\n", offset, midi_timestamp, smpl_timestamp);
    midi_timestamp += offset;
    smpl_timestamp = timestamp_midi2smpl(midi_timestamp);
    return 0;
}

//GET
int
midi_timestamp_get()
{
    return midi_timestamp;
}

int
smpl_timestamp_get()
{
    return smpl_timestamp;
}

//2
int
timestamp_midi2smpl(int timestamp)
{
    //(ticks*(tempo*rate/tpqn)/1000)/1000->samples
    //return (timestamp*(tempo*rate/tpqn)/1000)/1000;
    return ((long long)timestamp*60*rate)/(tpqn*tempo);
}

int
timestamp_smpl2midi(int timestamp)
{
    //return (((timestamp*tpqn*1000000)/tempo)*1)/rate;
    return ((long long)timestamp*tpqn*tempo)/(rate*60);
}

int
timestamp_reset()
{
    midi_timestamp=smpl_timestamp=0;
    return 0;
}
