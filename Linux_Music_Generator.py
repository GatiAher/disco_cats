"""
Music Generator

Python script to read serial and generate music

Init:
1. Load magenta VAE model
2. Load embedding reconstruction for m midi files

Loop:
1. Read n numbers over serial
2. Select embeddings and generate new music
3. Save music to midi file format
4. Run bash command in python to open midi file reader and send output over serial

"""

import fire

import numpy as np
import os
import tensorflow.compat.v1 as tf

import magenta.music as mm
from magenta.music.sequences_lib import concatenate_sequences
from magenta.models.music_vae import configs
from magenta.models.music_vae.trained_model import TrainedModel

tf.disable_v2_behavior()

import pretty_midi
from mido import MidiFile

import serial
import glob
import subprocess as sp
import multiprocessing as mp

BATCH_SIZE = 4
Z_SIZE = 512
TOTAL_STEPS = 512
BAR_SECONDS = 2.0
CHORD_DEPTH = 49

SAMPLE_RATE = 44100
SF2_PATH = 'content/SGM-v2.01-Sal-Guit-Bass-V1.3.sf2'

# # FSM state of user_input.py
STATE_INIT = 0 # initializing
STATE_LISTEN = 1 # ready to listen for serial input
STATE_SEND = 2 # ready to send serial output (seed music)
STATE_INTERP = 3 # ready to send serial output (interpolated music)

state = STATE_INIT

# get model checkpoint: not chord conditioned
config = configs.CONFIG_MAP['hier-multiperf_vel_1bar_med']
model = TrainedModel(
    config, batch_size=BATCH_SIZE,
    checkpoint_dir_or_path='content/model_fb256.ckpt')
model._config.data_converter._max_tensors_per_input = None

# Seed files (2 seconds of generated music)
seed_file_names = {
    "a": "gen/arpeg.mid",
    "b": "gen/crash.mid",
    "c": "gen/flutter.mid",
    "d": "gen/geralt.mid",
    "e": "gen/heavy.mid",
    "f": "gen/jazzy.mid",
    "g": "gen/rock_candy.mid",
    "h": "gen/smooth.mid",
    "i": "gen/whine.mid"
}

seed_musics = {}

send_file = None

####################
# HELPER FUNCTIONS #
####################

# Play sequence using SoundFont.
def play(note_sequences):
  if not isinstance(note_sequences, list):
    note_sequences = [note_sequences]
  for ns in note_sequences:
    mm.play_sequence(ns, synth=mm.fluidsynth, sf2_path=SF2_PATH)
  
# Spherical linear interpolation.
def slerp(p0, p1, t):
  """Spherical linear interpolation."""
  omega = np.arccos(np.dot(np.squeeze(p0/np.linalg.norm(p0)), np.squeeze(p1/np.linalg.norm(p1))))
  so = np.sin(omega)
  return np.sin((1.0-t)*omega) / so * p0 + np.sin(t*omega)/so * p1

# Download sequence.
def download(note_sequence, filename):
  mm.sequence_proto_to_midi_file(note_sequence, filename)
  print("downloaded to:", filename)

# Chord encoding tensor.
def chord_encoding(chord):
  index = mm.TriadChordOneHotEncoding().encode_event(chord)
  c = np.zeros([TOTAL_STEPS, CHORD_DEPTH])
  c[0,0] = 1.0
  c[1:,index] = 1.0
  return c

# Trim sequences to exactly one bar.
def trim_sequences(seqs, num_seconds=BAR_SECONDS):
  for i in range(len(seqs)):
    seqs[i] = mm.extract_subsequence(seqs[i], 0.0, num_seconds)
    seqs[i].total_time = num_seconds

# Consolidate instrument numbers by MIDI program.
def fix_instruments_for_concatenation(note_sequences):
  instruments = {}
  for i in range(len(note_sequences)):
    for note in note_sequences[i].notes:
      if not note.is_drum:
        if note.program not in instruments:
          if len(instruments) >= 8:
            instruments[note.program] = len(instruments) + 2
          else:
            instruments[note.program] = len(instruments) + 1
        note.instrument = instruments[note.program]
      else:
        note.instrument = 9

########
# MAIN #
########

def load_seed_files():
    global state
    global send_file
    print("Start loading embedding reconstruction")

    midi_files = [m for m in seed_file_names.values()]
    print(type(midi_files), midi_files)

    seqs = []
    for f in midi_files:
        midi_data = pretty_midi.PrettyMIDI(f)
        seqs.append(mm.midi_to_note_sequence(midi_data))

    uploaded_seqs = []
    for seq in seqs:
        _, tensors, _, _ = model._config.data_converter.to_tensors(seq)
        uploaded_seqs.extend(model._config.data_converter.from_tensors(tensors))

    trim_sequences(uploaded_seqs)

    print(f"Parsed {len(uploaded_seqs)} measures")

    for i, k, in enumerate(seed_file_names):
        seed_musics[k] = uploaded_seqs[i]

    state = STATE_LISTEN
    # send_file = "gen/rock_candy_geralt.mid"
    # state = STATE_SEND

def main(seri="/dev/ttyACM1", sero="/dev/ttyACM0"):
    global state
    global send_file

    load_seed_files()

    # Set up serial input
    ser = serial.Serial(seri)
    ser.baudrate = 115200
   
    while True:
        print("state:", state)

        if state == STATE_LISTEN:
            # 1. Read n numbers over serial
            # if ser.in_waiting:
            ser_bytes = ser.readline()
            print("ser_bytes:", ser_bytes)
            all_bytes = ser_bytes[0:len(ser_bytes)].decode("utf-8").split()
            print("All Bytes:", all_bytes)

            if len(all_bytes) > 1:
                command_byte = all_bytes[0]
                data_bytes = all_bytes[1:]
                print("Command Byte:", command_byte)
                print("Data Bytes:", data_bytes)
                
                if command_byte == "0" and data_bytes[0] in seed_musics.keys():
                    # send seed music
                    send_file = seed_file_names[data_bytes[0]]
                    state = STATE_SEND

                elif command_byte == "1" and len(data_bytes) > 1 and all((db in seed_musics.keys() for db in data_bytes)):
                    # generate interpolated music
                    state = STATE_INTERP


        if state == STATE_INTERP:
            # 2. Select embeddings and generate new music
            num_bars = 20
            temperature = 0.2

            num_bars_each = int(num_bars/(len(data_bytes) - 1))

            chosen_seed_musics = []
            for db in data_bytes:
                chosen_seed_musics.append(seed_musics[db])

            z_list = []
            for i in range(len(chosen_seed_musics) - 1):
                index_s = i
                index_e = i + 1

                # get encoding coordinate (dim 512)
                print("Getting chosen music seed encodings...")
                zs, _, _ = model.encode([chosen_seed_musics[index_s]])
                ze, _, _ = model.encode([chosen_seed_musics[index_e]])

                # spherical interpolation
                z_list.append(np.array([slerp(np.squeeze(zs), np.squeeze(ze), t) for t in np.linspace(0, 1, num_bars_each)]))

            # generation
            z = np.concatenate(z_list, axis=0)
            print("Generating interpolated decoding...")
            seqs = model.decode(length=TOTAL_STEPS, z=z, temperature=temperature)
            trim_sequences(seqs)
            fix_instruments_for_concatenation(seqs)
            recon_interp_ns = concatenate_sequences(seqs)

            # 3. Save music to midi file format
            save_to = 'gen/music.mid'
            download(recon_interp_ns, save_to)
            print(f"Saved generated midi to {save_to}")
            
            send_file = save_to
            state = STATE_SEND


        if state == STATE_SEND:
            # 4. Run bash command in python to send midi file over serial
            PID_send_midi = sp.Popen(["aplaymidi", "-p", "128:1", send_file])
            PID_play_midi = sp.Popen(["timidity", "-Os", send_file])
            print(f"PID_send_midi {PID_send_midi} | PID_play_midi {PID_play_midi}")
            
            # # 5. Return to listening state
            state = STATE_LISTEN

if __name__ == "__main__":
    fire.Fire(main)