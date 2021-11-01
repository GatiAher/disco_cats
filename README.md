# Disco Cats
>Gati Aher, Max Stopyra, Cory Knox, Efe Cemal Clucu, Mari Kang  
 Oct 28, 2021
 Final Project for Principles of Integrated Design

- [Disco Cats](#disco-cats)
  - [BOM](#bom)
  - [Subsystem for accepting user input](#subsystem-for-accepting-user-input)
  - [Subsystem for Generating AI-Powered Music](#subsystem-for-generating-ai-powered-music)
    - [Install](#install)
    - [Run](#run)
  - [Subsystem for playing and transferring saved MIDI file to arduino](#subsystem-for-playing-and-transferring-saved-midi-file-to-arduino)
    - [Install](#install-1)
    - [Run](#run-1)
  - [Subsystem for Arduino Control of Lights and Music](#subsystem-for-arduino-control-of-lights-and-music)
- [Resources](#resources)
  - [Machine Learning for Multi-Track Music Generation](#machine-learning-for-multi-track-music-generation)
  - [Send MIDI Files Between Laptop and Arduino](#send-midi-files-between-laptop-and-arduino)
  - [Observe MIDI Files](#observe-midi-files)

## BOM
* Ubuntu 20.04
* Arduino UNO

## Subsystem for accepting user input
* TODO

## Subsystem for Generating AI-Powered Music 

Subsystem for using machine learning to generate seed songs (1-phrase, 8 instruments) + Subsystem for using Variational AutoEncoder (VAE) interpolation to "generate" a longer song that transitions from seed A to seed B

### Install

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

## Subsystem for playing and transferring saved MIDI file to arduino

### Install

Install `ttymidi`
* follow these instructions: http://mkctbbq.blogspot.com/2014/12/installing-ttymidi-on-ubuntu-studio.html

Install `TiMidity++`
* `sudo apt install timidity timidity-interfaces-extra`

### Run

Process to connect Ttymidi to TiMidity++, play music, and send file to Arduino
```bash
ttymidi -s /dev/ttyACM0`
timidity -iA
aconnect 128:0 129:0
aplaymidi -p 128:1 rock_candy_geralt.mid & timidity -Os gen/rock_candy_geralt.mid
```

To see port options:
```bash
aconnect -i            # this just shows input port options
aconnect -o            # this just shows output port options
```

## Subsystem for Arduino Control of Lights and Music

* TODO

# Resources

## Machine Learning for Multi-Track Music Generation

VAE Theory
* [Picture-based explanation of VAE models](https://www.jeremyjordan.me/variational-autoencoders/)

Google Magenta Multitrack MusicVAE
* [Multitrack MusicVAE: Interactively Exploring Musical Styles](https://magenta.tensorflow.org/multitrack)
* [Jupyter Notebook: Multitrack MusicVAE.ipynb - Colaboratory](https://colab.research.google.com/github/magenta/magenta-demos/blob/master/colab-notebooks/Multitrack_MusicVAE.ipynb)
* [GitHub Repo](https://github.com/magenta/magenta/tree/main/magenta/models/music_vae)

## Send MIDI Files Between Laptop and Arduino

[Ttymidi](http://www.varal.org/ttymidi/): allows external serial devices to interface with ALSA MIDI applications.

[TiMidity++](http://timidity.sourceforge.net/#info): software synthesizer that can play MIDI files without a hardware synthesizer. Plays MIDI files on Ubuntu and allows Arduino boards to talk to MIDI applications.

## Observe MIDI Files

[Rosegarden] - track-oriented audio / MIDI sequencer to visualize MIDI files. Very finicky...
* Install: `sudo apt install rosegarden`
* Run:
  1. `rosegarden`
  2. `timidity -iA`
  3. In rosegarden UI, click on "manage midi devices" then connect first port in midi outputs box ([instructions](https://ubuntuforums.org/showthread.php?t=1700943))
