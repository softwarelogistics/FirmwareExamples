# Firmware Examples

Examples of how to use different drivers for Software Logistics Open Hardware Platform

Getting started with our hardware

Plug in the NuvIot sensor board and determine which com port is is connecting on.

# VS Code - Windows

## Clone the repository
https://github.com/softwarelogistics/FirmwareExamples

This currently has all of our examples.

## Install PlatformIO Extension

We use the VS Code Extension, PlatformIO IDE which is an open source and widely used for embedded development. More information can be found at: https://docs.platformio.org/en/latest/integration/ide/vscode.html

## Install Serial Debug Assitant

Another tool we have found useful is the Serial Debug Assistant which can be downloaded from the Microsoft Store. Search for "serial debug assitant".

## Setup

Open the folder of the sample you want to use and edit "platformio.ini"

### NuvIot LIbraries

Make sure that you have downloaded the NuvIot libraries and that the entry in platformio.ini points to the correct place on your machine.

### COM port and speed

You will also need to set the com port and com port speed.

The related platformio.ini entries for my machine are shown below:

monitor_speed = 115200
debug_speed = 115200

upload_port = COM4
monitor_port = COM4

lib_extra_dirs = c:\Arduino (where i have downloaded the NuvIot libraries..)

## Using the firmware

Open the example you want to use

Build the project

Note: A file with extension .elf (https://en.wikipedia.org/wiki/Executable_and_Linkable_Format) is generated.

Upload the project


