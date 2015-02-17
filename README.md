# About 'Teye' on GitHub
Hi there! It’s me again, Teye. After waiting for so long, now it’s time to reveal my innermost thoughts. Time to disclose the most closely guarded secret: the code that wakes me up! Here it is: This repository contains my source code in its current version.

But hang on a minute – you don’t know me yet and you’ve come across this file by accident? Then let me very briefly introduce myself: I’m a communication device which has neither keys nor screens. But I have voice recognition and can show emotions by moving my servo-motored eyes instead. Pretty cool, isn’t it? 

But that’s it for now. If you want to know more details, visit me on [teye.denkwerk.com].


## This is a beta version
That means it isn’t the final version yet and any help can be useful: If you find any mistakes or problems, just post them in the issue tracker and I'll take a look! Two heads are better than one – aren’t they?


## Hardware
I was developed for a Raspberry Pi B with a Unix/Linux system. To make your own duplicate of me, the following additional hardware is required: 
1.	2 Customary Servo Motors (e.g. PowerPro MG90S)
2.	1 Infrared Sensor (e.g. SHARP 2Y0A02)
3.	1 Loudspeaker (e.g. Visaton K 45 - 8 Ohm)
4.	1 Microphone (e.g. CZ034)
5.	1 Raspberry compatible WLAN-Module
6.	1 Raspberry compatible Soundcard (e.g. SpeedLink USB Soundcard)
7.	1 Analog-Digital Converter (ADC) (e.g. MCP3008)
8.	1 SDHC SD Card

## Before compiling the source code
Back up all files before compiling the source code so that you can restore existing data if anything goes totally wrong. With a backup you’re always on the safe side. 

## Required libraries
1.	PJSIP
2.	POCKETSPHINX
3.	SERVOBLASTER
4.	WIRINGPI

## Preparation of Raspberry Pi
1.	Install Rasbarian
2.	Configuration tool:
	1. Update the configuration tool to the latest version
		1. "8. Advanced Options"
		2. "A7 Update"
	2. Activate SSH 
		1. "8. Advanced Options"
		2. "A4 SSH"
		3. "Enable"
	3. Overclocking
		1. "7. Overclocking"
		2. Click "OK"
		3. Select Turbo
		4. Click "OK"
	4. Link audio to 3.5mm jack plug
		1. "8. Advanced Options"
		2. "A6 Audio"
		3. "1 Force 3.5mm headphone jack"
3.	OS update:
	1. 'sudo apt-get update'
	2. 'sudo apt-get upgrade'
4.	Firmware update:
	1. 'sudo apt-get install git-core'
	2. 'sudo wget http://goo.gl/1BOfJ -O /usr/bin/rpi-update'
	3. 'sudo chmod +x /usr/bin/rpi-update'
	4. 'sudo rpi-update'
	5. 'sudo reboot'
5.	Activate USB Microphone 
	1. 'sudo nano /etc/modprobe.d/alsa-base.conf'
	2. switch "options snd-usb-audio index=-2" to "# options snd-usb-audio index=-2" 
	3. add "options snd-usb-audio index=0" 
	4. close editor ("STRG + X" -> "Y")
	5. 'sudo alsa force-reload'
	6. 'sudo shutdown -r now' Reboot...
6.	Make an application folder (/home/pi/applications)
7.	Make a EXX2 folder (/home/pi/applications/EXX2)
8.	Copy EXX2 application 
9.	Make a Tmp folder (for compiling)
	1. 'mkdir /home/pi/tmp'
10.	Install ServoBlaster
	1. copy directory to RaspPi
	2. switch to 'user' directory 
	3. call up 'sudo make servod && sudo make install' 
	4. open 'sudo nano /etc/init.d/servoblaster' 
	5. paste 'OPTS="--idle-timeout=2000 --p1pins=0,0,0,0,0,0,0,0,0,0,0,11,0,13"' 
	6. reboot
11.	Compiling WiringPi 
	1. 'cd $HOME/tmp'
	2. 'mkdir WIPI'
	3. 'cd WIPI'
	4. 'git clone git://git.drogon.net/wiringPi'
	5. 'cd wiringPi'
	6. './build'
12.	Activate RASPI Sound Module 
	1. 'sudo nano /etc/modules'
	2. add 'snd_soc_bcm2708', 'snd_soc_bcm2708_i2s', 'bcm2708_dmaengine', 'snd-soc-pcm1794a', 'snd_soc_rpi_dac' 
13.	Compiling PJSIP 
	1. install dependecies: 'sudo apt-get install libv4l-dev libx264-dev libssl-dev alsa alsa-base alsa-utils alsa-tools libasound2-plugins libasound2 libasound2-dev asterisk'
	2. install SDL:
		1. 'cd $HOME/tmp' 
		2. 'mkdir SDL'
		3. 'cd SDL'
		4. 'wget http://www.libsdl.org/tmp/SDL-2.0.tar.gz'
		5. 'tar xvfz SDL-2.0.tar.gz'
		6. 'cd SDL-2.0.XXX' (XXX adjust the version)
		7. './configure'
		8. 'make && sudo make install'
	3. install FFMPEG 
		1. 'cd $HOME/tmp'
		2. 'mkdir FFMPEG'
		3. 'cd FFMPEG'
		4. 'wget http://ffmpeg.org/releases/ffmpeg-XXX.tar.gz' (XXX search for latest version)
		5. 'tar xvfz ffmpeg-XXX.tar.gz' (adjust the version)
		6. 'cd ffmpeg-XXX' (adjust the version)
		7. './configure --enable-shared --disable-static --enable-memalign-hack --enable-gpl --enable-libx264'
		8. 'make && sudo make install'
	4. install PJSIP
		1. 'cd $HOME/tmp'
		2. 'mkdir PJSIP'
		3. 'cd PJSIP'
		4. 'wget http://www.pjsip.org/release/XXX/pjproject-XXX.tar.bz2' (XXX search for latest version)
		5. 'tar xvfj pjproject-XXX.tar.bz2' (adjust the version)
		6. 'cd pjproject-XXX/' (adjust the version)
		7. './configure --disable-video --enable-shared --prefix=/usr/local/lib'
		8. 'nano pjlib/include/pj/config_site.h'
		9. add '#define PJMEDIA_AUDIO_DEV_HAS_PORTAUDIO 0', '#define PJMEDIA_AUDIO_DEV_HAS_ALSA 1', '#define PJ_IS_LITTLE_ENDIAN 1', '#define PJ_IS_BIG_ENDIAN 1', '#include <pj/config_site_sample.h>' 
		10. 'cd ../../..'
		11. './configure --disable-video --enable-shared --prefix=/usr/local/lib'
		12. 'sudo nano user.mak' (is created) (for debug symbols)
		13.  paste 'export CFLAGS += -g' und 'export LDFLAGS += ' (for debug symbols)
		14. 'make dep && make && sudo make install'
14.	Compiling Sphinx
	1. 'apt-get install bison'
	2. 'apt-get install libasound2-dev'
	3. Compiling & installing SPHINXBASE 
		1. 'cd $HOME/tmp'
		2. 'mkdir SPHINXBASE'
		3. 'cd SPHINXBASE'
		4. 'wget http://sourceforge.net/projects/cmusphinx/files/sphinxbase/XXX/sphinxbase-XXX.tar.gz/download' (XXX latest version)
		5. 'tar -xzvf sphinxbase-XXX.tar.gz' (adjust the version)
		6. 'cd sphinxbase-XXX' (adjust the version)
		7. './configure --enable-fixed'
		8. 'make && sudo make install'
	4. Compiling & installing POCKETSPHINX 
		1. 'cd $HOME/tmp'
		2. 'mkdir POCKETSPHINX'
		3. 'cd POCKETSPHINX'
		4. 'wget http://sourceforge.net/projects/cmusphinx/files/pocketsphinx/XXX/pocketsphinx-XXX.tar.gz/download' (XXX latest version)
		5. 'tar -xzvf pocketsphinx-XXX.tar.gz' (adjust the version)
		6. 'cd pocketsphinx-XXX' (adjust the version)
		7. './configure'
		8. 'make && sudo make install'
15.	Enable Autorun
	1. 'sudo nano /etc/rc.local'
	2. 'sudo /home/pi/applications/EXX2/EXX2 &'

Further information
(Link to PDF)

[teye.denkwerk.com]:http://teye.denkwerk.com
