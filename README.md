ES40 Alpha System Emulator

Original project at http://www.es40.org (now dead)

Original code at https://sourceforge.net/projects/es40

=== Change log:

* 12 Jul 2020
* compiles at least under Linux Ubuntu 18.04

* 20 Nov 2022
* compiles also under Linux Ubuntu 20.04

=== Running the test
```
   cd ES40 (Project main dir)
   test/run_testsuite.sh
```
or cd into the test dir and run
```
   test.sh
```

== Important Note
The rom test must run first. It will generate the decompressed firmware image.
Currently this is only possible with the es40_idb executable because the Single-Step functionality
is need and this is turned off in the es40 prg for performance reasons.

So ALWAYS use es40_idl if you need to decompress the firmware.

Original README below

Welcome to es40.

es40 is free software. Please see the file COPYING for details.
For documentation, please see the files in the doc subdirectory.
For building and installation instructions please see the INSTALL file.
