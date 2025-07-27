# Jack2Digital

Personal project to eventually be able to play my digital piano (that only has an analog Jack output) in silence. I had no idea about analog audio when I started this project so expect major improvements over time as I learn more. 

Take a look at the samples directory - There you can find interesting "milestones" of this project.

## Compiling it

### Prerequisites
- [Pico C SDK](https://github.com/raspberrypi/pico-sdk) with all submodules downloaded and `PICO_SDK_PATH` environmental variable set accordingly
- CMake
- Ninja / make or any other build system
- [ARM Toolchain (arm-none-eabi)](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)

### Commands

*Example with [Ninja](https://ninja-build.org/):*

```bash
mkdir build
cd build
cmake .. -G "Ninja" 
ninja -j4
cp Jack2Digital.uf [RPI_PICO_DRIVE]
```

*Or use the provided `run.bat` script if you're on Windows. (You may need to tweak the destination RPi Pico drive)*


## Building it

### Parts
- Raspberry Pi Pico
- Minijack connector with soldered on jumper wires
- 2x 47kΩ resistors, 1x 150Ω resistor, 1x 330Ω resistor, 1x 1kΩ resistor
- 2x ceramic 100nF capacitors
- LM358P operational amplifier

*I'll try to update the parts list and the schematic image whenever I make any meaningful changes to the circuit.

### Schematic

All GNDs on the schematic refer to the AGND pin on the RPi Pico. The 1khz AC wave symbol refers to the analog input straight from the Jack connector:

![schematic](/schematics/schematic_v1.png)

*The Falstad project is in /schematics/*

### Final product

*There will be a photo soon*