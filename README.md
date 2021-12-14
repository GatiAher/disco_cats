# Disco Cats
>Gati Aher, Max Stopyra, Cory Knox, Efe Cemal Clucu, Mari Kang  
 Oct 28, 2021 - Dec 14, 2021  
 Final Project for Principles of Integrated Design

- [Disco Cats](#disco-cats)
  - [Project Website](#project-website)
  - [Hardware](#hardware)
  - [Integration Map](#integration-map)
  - [Subsystem for accepting user input](#subsystem-for-accepting-user-input)
  - [Subsystem for playing and transferring saved MIDI file from Laptop to Arduino](#subsystem-for-playing-and-transferring-saved-midi-file-from-laptop-to-arduino)
    - [Install](#install)
    - [Run](#run)
  - [Subsystem for Generating AI-Powered Music](#subsystem-for-generating-ai-powered-music)
    - [Installation Guide](#installation-guide)
    - [Run](#run-1)
  - [Subsystem for Arduino Control of Lights and Motors](#subsystem-for-arduino-control-of-lights-and-motors)
    - [MAX7219 Chip](#max7219-chip)
    - [Motor Control](#motor-control)

## Project Website

Check out our final website and documentation at: https://olincollege.github.io/pie-2021-03/Disco-Cats/ 

## Hardware
* Ubuntu 20.04
* Arduino Uno
* Arduino Mega
* [28-BYJ48 stepper motors](https://www.mouser.com/datasheet/2/758/stepd-01-data-sheet-1143075.pdf)
* [ULN2003 driver boards](https://www.electronicoscaldas.com/datasheet/ULN2003A-PCB.pdf)
* [MAX7219 LED driver chip](https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf)

## Integration Map

![Integration Map](/img/integration_map.png)

## Subsystem for accepting user input

Overview:
* Arduino_button sends results of button input over serial `/dev/ttyACM1`
* Laptop listens to `/dev/ttyACM1` and sends selected midi file over serial `/dev/ttyACM0` (can be configured to work with different ports)
* Arduino_lights listens to `/dev/ttyACM0` and controls LEDs

Installation:
* everything required for music generation and transmittance to arduino
* google/python-fire: `conda install fire -c conda-forge`
* pySerial: `pip install pyserial`

Run:
1. Load code onto Arduino
2. `ttymidi -s /dev/ttyACM0`
3. `timidity -iA`
4. `aconnect 128:0 129:0`
5. `conda activate magenta`
6. `python3 Linux_Music_Generator.py`

## Subsystem for playing and transferring saved MIDI file from Laptop to Arduino

[Ttymidi](http://www.varal.org/ttymidi/): allows external serial devices to interface with ALSA MIDI applications.

[TiMidity++](http://timidity.sourceforge.net/#info): software synthesizer that can play MIDI files without a hardware synthesizer. Plays MIDI files on Ubuntu and allows Arduino boards to talk to MIDI applications.

### Install

Install `ttymidi`
* follow these instructions: http://mkctbbq.blogspot.com/2014/12/installing-ttymidi-on-ubuntu-studio.html

Install `TiMidity++`
* `sudo apt install timidity timidity-interfaces-extra`

### Run

Process to connect Ttymidi to TiMidity++, play music, and send file to Arduino

```bash
ttymidi -s /dev/ttyACM0

timidity -iA

aconnect 128:0 129:0
aplaymidi -p 128:1 gen/rock_candy_geralt.mid & timidity -Os gen/rock_candy_geralt.mid
```

To see port options:
```bash
aconnect -i            # this just shows input port options
aconnect -o            # this just shows output port options
```

## Subsystem for Generating AI-Powered Music 

Subsystem for using machine learning to generate seed songs (1-phrase, 8 instruments) + Subsystem for using Variational AutoEncoder (VAE) interpolation to "generate" a longer song that transitions from seed A to seed B

### Installation Guide

Install Ubuntu Packages
* `sudo apt update`
* might have to run as root `sudo -s` 
  * `apt-get install libfluidsynth build-essential libasound2-dev libjack-dev portaudio19-dev`

Install and Activate `magenta` Environment
* `curl https://raw.githubusercontent.com/tensorflow/magenta/main/magenta/tools/magenta-install.sh > /tmp/magenta-install.sh`
* `bash /tmp/magenta-install.sh`
* `conda activate magenta`

Install Python Packages
* `conda install anaconda`
* `pip install --upgrade tensorflow`
* `pip install magenta pyfluidsynth pretty_midi`

Get `gsutil` and use it to put sf2 file and magenta model files into content folder:
* `mkdir content`
* Install `gsutil`: https://cloud.google.com/storage/docs/gsutil_install#deb
* Download [soundfonts](https://sites.google.com/site/soundfonts4u) (virtual musical instrument sounds in sf2 format)
  * `gsutil -q -m cp gs://download.magenta.tensorflow.org/soundfonts/SGM-v2.01-Sal-Guit-Bass-V1.3.sf2 content/`
* Download magenta model (3 files for each model, 6 files total): 
  * `gsutil -q -m cp gs://download.magenta.tensorflow.org/models/music_vae/multitrack/* content/`

### Run
* `conda activate magenta`
* `jupyer notebook Experiment.ipynb`

**Resources on Machine Learning for Multi-Track Music Generation**

VAE Theory
* [Picture-based explanation of VAE models](https://www.jeremyjordan.me/variational-autoencoders/)

Google Magenta Multitrack MusicVAE
* [Multitrack MusicVAE: Interactively Exploring Musical Styles](https://magenta.tensorflow.org/multitrack)
* [Jupyter Notebook: Multitrack MusicVAE.ipynb - Colaboratory](https://colab.research.google.com/github/magenta/magenta-demos/blob/master/colab-notebooks/Multitrack_MusicVAE.ipynb)
* [GitHub Repo](https://github.com/magenta/magenta/tree/main/magenta/models/music_vae)

## Subsystem for Arduino Control of Lights and Motors

**Components**
* MIDI Decoder: finite state machine, decodes each message as command-data-data bytes
* Matrix State: tracks state of LED
* Hardware Interface: Using 3-wire SPI interface to MAX7219 chip

### MAX7219 Chip
* Rapid multiplexing: to drive one row of LEDs at a time
* Shift register: to place data values for cells of a given row
* Latch register: to load data values to LED Matrix only when all values have been shifted into their final place

Challenges:
* Very funky wiring diagram and MAX7219 initialization on start-up
* Had to write own code to manage LED matrix with state of MIDI file
* Using no libraries because libraries are slow (bloated) and unnecessary

### Motor Control
* Using 28-BYJ48 stepper motors and ULN20-03 driver boards because they are the most common stepper drivers and thus have ready-made compatible arduino libraries
* Using [Accelstepper](https://www.arduino.cc/reference/en/libraries/accelstepper/) library because it allows us to control motors with array pointers, which makes the motor status iteration code very clean.

[Rosegarden] - track-oriented audio / MIDI sequencer to visualize MIDI files. Very finicky...
* Install: `sudo apt install rosegarden`
* Run:
  1. `rosegarden`
  2. `timidity -iA`
  3. In rosegarden UI, click on "manage midi devices" then connect first port in midi outputs box ([instructions](https://ubuntuforums.org/showthread.php?t=1700943))
