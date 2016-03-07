mips32r2 ArchC functional model
=====

This is the mips32r2 ArchC functional model.

Current status
--------------
This model was a MIPS-I implementation and was updated to mips32r2 with
hardware floating-point support.

Currently it supports quite a few MIPS32r2 instructions, enough to compile
Mibench programs and run them correctly. However, this implementation does not
feature all instructions from the ISA specification, but only those appearing in
benchmarks. If you run into an implemented instruction, you can easily expand
this model.

The easiest way you can compile a program to mips32r2 is by using the ecc
compiler (http://ellcc.org/blog/?page_id=313) based on Clang/LLVM. Example:

    ecc -target mips-linux-eng hello.c -o hello
    mips.x --load=hello

It is possible that your program uses an unimplemented syscall, since this is
not a system simulator, but a process simulator. If this syscall is crucial to
your program, you may need to expand ArchC to implement it.

License
-------
 - ArchC models are provided under the ArchC license.
   See [Copying](COPYING) file for details on this license.

acsim
-----

    acsim mips.ac -abi -nw             (create the simulator)
    make                               (compile it)
    ecc -target mips-linux-eng hello.c -o hello
    mips.x --load=hello                (compile and run hello world program)

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
