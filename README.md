# AFATSF
> This is a library that allows for reading and writing to disks formated as FAT32 in an asynchronous (non blocking) manner.

## Table of contents
* [General info](#general-info)
* [Screenshots](#screenshots)
* [Technologies](#technologies)
* [Setup](#setup)
* [Features](#features)
* [Status](#status)
* [Inspiration](#inspiration)
* [Contact](#contact)

## General info
This is a library that allows for reading and writing to disks formated as FAT32 in an asynchronous (non blocking) manner. Most of the file systems available online demands a write or read operation is finished by the time a function call returns. This is not a problem for systems for applications that do not have time constraints or for those operating above a Real-Time Operating System (RTOS). However, for those trying to avoid the processing overhead and additional memory usage of an RTOS, there is not an option available. With that need in mind and the intent to learn a little about file systems in general, this project was created.

## Screenshots
No screenshots available.

## Technologies
* Tech 1 - version 1.0
* Tech 2 - version 2.0
* Tech 3 - version 3.0

## Setup
Describe how to install / setup your local environement / add link to demo version.

## Code Examples
Show examples of usage:
`put-your-code-here`

## Features
List of features ready and TODOs for future development
* Open and read files from the root directory, subfolders not implemented
* Can read only the first cluster of the file, multicluster read not implemented
* Number of files opened simultaneously, number of disks and other parameters are configurable through macros
* Map files (header and source) used to add disks so the library can use then

To-do list:
* Add functions to write to existing files
* Add functions to create new files
* Implement the extended name size for files and folders. Currently limited to 8 characters for the name and 3 for the extension (8.3)
* Implement access to files inside subfolders

## Status
Project is: _in progress_. Functions are still being implemented.

## Inspiration

For some implementations:
* The first inspiration is a widely used library is [FatFs - Generic FAT Filesystem Module by Chan](http://elm-chan.org/fsw/ff/00index_e.html), which does a great job at allowing embedded systems of all sizes to read from and write to FAT formatted disks, and stil being updated.
* An rare option, probably the only implementation publicly available on the internet, of a non-blocking, asynchronous FAT library [AsyncFatFS](https://github.com/thenickdude/asyncfatfs). This implementatio has been used, for instance, on the [Cleanflight / Betaflight's "blackbox" logging system](https://github.com/betaflight/betaflight), and seems to be maintained uo to this commit.

For some tutorialson the workings of FAT file systems:
* [Wyoos operating system](http://wyoos.org/) and [the series of Youtube videos](https://www.youtube.com/watch?v=IGpUdIX-Z5A&list=PLHh55M_Kq4OApWScZyPl5HhgsTJS9MZ6M&index=31) about its implementation
* [Code and Life](https://codeandlife.com/2012/04/02/simple-fat-and-sd-tutorial-part-1/) web tutorial on interfacing FAT16 SDCards and the tutorials it references too.

The template for this readme file was created by [@flynerdpl](https://www.flynerd.pl/).

## Contact
Created by [@passoswell](https://github.com/passoswell).
