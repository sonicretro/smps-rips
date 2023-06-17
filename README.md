# SMPS rips

This repository contains sound data ripped from games that use SEGA's sound driver, which is commonly known as SMPS.

It includes songs, PCM samples and instrument data. It is raw binary data, extracted directly from ROMs.

## Folder structure

The top level folders name the basic type of SMPS driver used:

- `68k` - Sega MegaDrive SMPS 68k
- `Pico` - Sega Pico SMPS 68k
- `preSMPS` - Sega MegaDrive preSMPS (68k and Z80)
- `Z80` - Sega MegaDrive SMPS Z80

Inside those is one folder per game, which contains the ripped data.

Game folders may contain additional sub-folders with rips from prototypes.
(see e.g. Dynamite Headdy or Sonic The Hedgehog 3)

Some folders contain bonus content, such as versions of songs with fixed bugs.
A few folders also contain rips of sound effects.

## File naming

### Songs

Song files generally follow the pattern: `01 Song Title.ABCD.extension`

The "ABCD" part is optional and specifies a hexadecimal Z80 address the song starts at. It is used when SMPSPlay's offset detection goes wrong.

The file extension for the songs generally follows this scheme:

- `psz` - pre-SMPS Z80 *(has an SMPSPlay-specific header)*
- `psk` - pre-SMPS 68k Type 1 *(has an SMPSPlay-specific header)*
- `psm` - pre-SMPS 68k Type 2 *(has an SMPSPlay-specific header)*
- `sze` - early SMPS Z80
- `smy` - SMPS Z80 Type 1
- `smz` - SMPS Z80 Type 2
- `sm1`/`sm2` - used by some SMPS Z80 rips with songs of different Z80 banks in the same folder
- `mmw` - SMPS 68k Type 1 ("mmw" due to MJ's Moonwalker being one of the first SMPS 68k Type 1 games that I had a look at)
- `tra` - SMPS 68k Type 2, "overflow" tempo algorithm
- `trs` - SMPS 68k Type 2, "timeout" tempo algorithm
- `spi` - Sega Pico SMPS 68k
- `org` - "original" dumps, indicates that the "normal" file was patched to be self-contained and playable by SMPSPlay

game-specific extensions:

- `ors` - OutRunners (heavily modified SMPS Z80)
- `ps4` - Phantasy Star IV (SMPS 68k Type 2 with special PCM channel)
- `rst` - Ristar (SMPS 68k Type 2 with two PCM drum channels)
- `s2b` - Sonic The Hedgehog 2 Simon Wai prototype (heavily modified SMPS Z80)
- `s3k` - Sonic The Hedgehog 3 & Knuckles / Sonic 3D Blast (SMPS Z80 Type 2)
- `skc` - Chaotix (SMPS Z80 Type 2 with four PWM drum channels)
- `sm2` - Sonic The Hedgehog 2 (heavily modified SMPS Z80)
- `smp` - Sonic The Hedgehog 1 (modified SMPS 68k Type 1)
- `sqs` - Quack Shot (modified SMPS Z80)
- `svr` - Virtua Racing (heavily modified SMPS Z80)

*Note:* Originally the extensions above were used by SMPSPlay v1 to identify the SMPS format type.
However, since SMPSPlay v2 the file extensions can be freely defined using `config.ini`.
I kept them to make the driver type more easy to identify.

### Other files

- `config.ini` - configuration file to be loaded by SMPSPlay
- `DAC.ini`/`DAC_Voice.ini` - list of PCM samples (file name, compression type, playback rate)
- `DefCFlag.txt` - describes SMPS sequence commands (also known as "coordination flags")
- `DefDrum.txt` - describes the drum channel note map (maps notes to FM/PSG tones or PCM sample)
- `DefDrum_PSG.txt` - describes the note-specific instruments on PSG drum channels
- `DefDrv.txt` - describes general SMPS driver parameters (instrument format, tempo algorithm, FM/PSG frequencies, available channels, ...)
- `Drums_FM.bin`/`Drums_PSG.bin` - contains SMPS data for FM and PSG drums, mainly used by Z80-based drivers *(has an SMPSPlay-specific header)*
- `InsSet.bin` - FM instrument set
- `Modulat.lst` - frequency modulation envelope data *(mid2smps envelope list format)*
- `PanAni.bin` - pan animation data *(has an SMPSPlay-specific header)*
- `Pointers.txt` - technical information about the game, includes pointer lists and often describes oddities about the game or sound engine
- `PSG.lst` - PSG volume envelope data *(mid2smps envelope list format)*
- `Z80Drv.bin` - dump of the Z80 driver code

## Song playback

The rips can be played back using [SMPSPlay](https://github.com/ValleyBell/SMPSPlay) by setting the following configuration entries

```ini
MusicDir = <folder with rips>
DataDir = <folder with rips>
LoadConfig = config.ini
```

## Credits

- Most of the rips were done by [Valley Bell](https://github.com/ValleyBell)  
  Songs, PCM samples and envelopes were extracted using the tools *SMPSExtract* and *SMPSpreExtract*.
  Information about the sound drivers was obtained using *Smps68kExtract* and *SmpsZ80Extract*, as well as disassemblies.  
  These tools are part of the [SMPS Research Pack](https://forums.sonicretro.org/index.php?threads/valley-bells-smps-research.32473/).
