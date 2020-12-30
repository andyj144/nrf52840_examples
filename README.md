# nrf52840_projects

## Introduction
This repo contains all the code for my personal projects targeting the NRF52840.  In addition to more complete projects is also contains some very simple example code that others may find useful when first beginning with the chip.  These examples may be expanded into tutorials in the near future.
Current all projects target the sparkfun nrf52840 mini board.

## Organization
The repo is split between basic projects that only demonstrate a specific library or peripheral, and free RTOS based projects that are more complete and do something useful (maybe).

Each of the basic projects lives in its own folder that contains all the non-SDK code and a make file that can build the project.

The free RTOS based projects are divided between tasks and projects. The tasks folder contains the code and header files for any tasks that are created, while the projects folders contain the main.c, config files and make files.

All the makefiles assume that other required libraries are located at the same directory level as the repo.
```bash
.  
├── Adafruit_nRF52_Bootloader  
├── nRF5_SDK_17.0.0_9d13099  
├── printf  
└── nrf52840_projects  
```

If you chose to locate these files somewhere else you will need to modify the make files.

## Building
The NRF5_SDK is required for all projects and the adafruit-nrfutil is required for DFU loading of compiled programs via that make system.

You need to have gcc for arm installed and set the correct path to the toolchain and version information in the project make file:  
```Makefile
GNU_INSTALL_ROOT ?= /usr/local/gcc-arm-none-eabi-9-2020-q2-update/bin/
GNU_VERSION ?= 9.3.1
GNU_PREFIX ?= arm-none-eabi
```

simply typing `make` in the project directory will build the project.
With your board in DFU mode project can be loaded with the command:  
```bash
$ make bootload SERIAL_PORT=<insert port here>
```
