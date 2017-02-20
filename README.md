# WS2813 CONTROLLER FIRMWARE

## Supported MCUs

* Atmel AVR ATtiny25/45/85

## Fuse bits

Ext | High | Low
--|--|--
FF | DF | F1

**NOTE:** these values will set MCU speed to 16MHz, but `F_CPU` constant must be set to `8000000`. This configuration supports working in "slow" mode (400 kbps). To use it just set MCU speed to 8MHz.

## Ports

Data channel | Control LED
--|--
B1 | B3

## Commands format

The firmware fetches commands from EEPROM. Each command is a word (16 bits, big-endian in docs, little-endian in EEPROM). Following formats are supported.

### RESTART command

This command must be the last one in the program. When it is reached the MCU restarts the program.

```
1|1|1|1|1|1|1|1|1|1|1|1|1|1|1|1 (0xFFFF)
```

### RGB command

Color value, 5 bits per channel, very low bit is `0`. Each color applies to next LED in the strip (see also REPEAT command below). Use DELAY command (see below) to restart applying colors from very first LED.

```
R|R|R|R|R|G|G|G|G|G|B|B|B|B|B|0
```
**NOTE:** the channels could be mixed in any ways (GRB, RBG, etc.), so check it for each device.

### DELAY command

Two low bits are `01`, others are delay value (`ms` in fast mode, `ms * 2` in slow mode). Use REPEAT command (see below) if you need longer delay. Each DELAY command restarts applying colors from first LED.

```
x|x|x|x|x|x|x|x|x|x|x|x|x|x|0|1
```

### REPEAT command

Repeats next command `N + 1` times. For example, if repeat value is `1` then next command will be performed twice. Next command will be treated as `RGB color` or `Delay command`. Other commands are not supported after REPEAT.

Low byte is `0x07`, high byte is repeat times.

```
x|x|x|x|x|x|x|x|0|0|0|0|0|1|1|1
```

## Program example

Here is simple program that imitating traffic light (red and green channels are reversed due to my broken led strip, sorry). HEX, little-endian:

```
0008 411F 0000 8008 411F 0000 0000 4000 411F 0000 8008 411F 0000 0000 0000 FFFF
```

Comments:

`0008` - faint green light for first LED.

`411F` - 2 seconds delay (fast mode). Restart applying colors from first LED.

`0000` - turn off first LED.

`8008` - faint yellow light for second LED.

`411F` - 2 seconds delay (fast mode). Restart applying colors from first LED.

`0000` - turn off first LED.

`0000` - turn off second LED.

`4000` - faint red light for third LED.

`411F` - 2 seconds delay (fast mode). Restart applying colors from first LED.

`0000` - turn off first LED.

`8008` - faint yellow light for second LED.

`0008` - faint red light for third LED.

`0000` - turn off first LED.

`0000` - turn off second LED.

`0000` - turn off third LED.

`FFFF` - program end, restart.