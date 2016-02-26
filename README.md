MIPS ArchC functional model
=====

This is the MIPS ArchC functional model.

Current status
--------------
This model was a MIPS-I implementation and is now being updated to mips32r2 with
hardware floating-point support.

Currently it supports only a few non-MIPS-I instructions, but enough to compile
a simple hello-world program and run it.

The easiest way you can compile a program to mips32r2 is using the ecc compiler
(http://ellcc.org/blog/?page_id=313) based on Clang/LLVM. Example:

    ecc -target mips-linux-eng hello.c -o hello
    mips.x --load=hello

License
-------
 - ArchC models are provided under the ArchC license.
   See [Copying](COPYING) file for details on this license.

acsim
-----
This model has the system call emulation functions implemented,
so it is a good idea to turn on the ABI option.

To use acsim, the interpreted simulator:

    acsim mips.ac -abi                 (create the simulator)
    make                               (compile)
    mips.x --load=<file-path> [args]   (run an application)

The [args] are optional arguments for the application.

There are two formats recognized for application <file-path>:
- ELF binary matching ArchC specifications
- hexadecimal text file for ArchC



Binary utilities
----------------
To generate binary utilities use:

    acbingen.sh -i<abs-install-path> -a<arch-name> mips.ac

This will generate the tools source files using the architecture
name <arch-name> (if omitted, mips1 is used), copy them to the
binutils source tree, build and install them into the directory
<abs-install-path> (which -must- be an absolute path).
Use "acbingen.sh -h" to get information about the command-line
options available.


Change history
------------

See [History](HISTORY.md)


Contributing
------------

See [Contributing](CONTRIBUTING.md)


More
----

Remember that ArchC models and SystemC library must be compiled with
the same GCC version, otherwise you will get compilation problems.

Several documents which further information can be found in the 'doc'
subdirectory.

You can find language overview, models, and documentation at
http://www.archc.org



Thanks for the interest. We hope you enjoy using ArchC!

The ArchC Team
Computer Systems Laboratory (LSC)
IC-UNICAMP
http://www.lsc.ic.unicamp.br
