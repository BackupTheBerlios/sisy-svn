#ifndef _SISY_TIMESTAMP_H_
#define _SISY_TIMESTAMP_H_

int midi_timestamp_get();
int timestamp_smpl2midi(int timestamp);
int timestamp_midi2smpl(int timestamp);
int smpl_timestamp_get();
int midi_timestamp_get();
int midi_timestamp_add(int offset);
int smpl_timestamp_add(int offset);
int midi_tpqn_set(int t);
int midi_tempo_set(int t);
int smpl_rate_set(int r);

#endif
