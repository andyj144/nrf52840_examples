# nrf52840_examples
## Introduction
This repo contains all the code for my set of tutorials targeting the NRF52840.  The examples will eventually included everthing from a simple blinky to usb and ble based shells.  The full documentation for these examples can be found at my blog: mynerdyprojects.com.

## Organization
The repo is split between projects and tasks.  Some of the projects are very basic and intended to illustrate a specific concept or peripheral, others are more involved, and intened to form a template for larger projects.  Some of the projects also use FreeRTOS. In these cases the project specific files such as the main loop, and Makefile, etc, are in the project folder, but the individual tasks are in a tasks folder as shown below.  This allows tasks to be used by multiple projects.

The Makefiles for all of these projects assume that you have 2 environment variables set.  ARM_GCC should point to the install directory for your compiler, and NRF_SDK should point to the install directory for the nRF5 SDK from Nordic Semiconductor.

```bash
.  
├── printf
├── projects  
└── tasks  
```

## Building
The NRF5_SDK is required for all projects and the adafruit-nrfutil is required for DFU loading of compiled programs via that make system.

Assuming you set the environment variables as described above, you can build any of the projects by navigating to that project's folder, and running `make`.

With your board in DFU mode, the project can be loaded with the command:  
```bash
$ make bootload SERIAL_PORT=<insert port here>
```
