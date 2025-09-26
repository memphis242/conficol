# `conficol`: Generic Collections Library in C
TODO

## Benchmarks
TODO

## Usage
TODO

## Building
### Dependencies
#### Library File(s)
No dependencies. :slightly_smiling_face:
#### Testing
[Unity](https://github.com/ThrowTheSwitch/Unity/) is used. The [Makefile](./Makefile) assumes that the Unity headers and `libunity` static library are on the include/library paths searched for by `gcc`.

### Notes on Link-Time Optimization
One may ask,

> "If I'm only using a subset of the functions provided in conficol, do I still get the entire library statically linked into my final executable?"

Although this may vary depending on your specific linker, typically, a linker is smart enough to optimize out which individual _object files_ within a library are not used. However, individual symbols within an object file are not _by default_ optimized out, even if unused and not linked to any specific reference within an application. Overall, this technically results in an inefficient application size.   

With that said, a lot of linkers support **Link-Time Optimization** (LTO), which means the linker can get more granular. This will require that the library code is compiled with special compiler options enabled and the final application to be linked with extra linker options. In GCC, for example, compiling this library code with `-ffunction-sections -fdata-sections` causes functions and data to be placed in their own sections within an object file, and then linking the library with your application using `--gc-sections` invokes the linker garbage collector (gc) against unused sections. This will hopefully (although is not guaranteed to) result in a smaller application size. In the `Makefile` within this repository, define `LTO` when invoking make, or simply append `-small` to the typical make targets (e.g., `make release` → `make release-small` !TODO).  

However, there is a potential cost to enabling LTO: runtime speed (also compilation speed). The compiler has to make fewer assumptions about an individual source code file / translation unit, which reduces its ability to make certain optimizations (specifically since the compiler won't know which functions/symbols will be optimized out or not). The GCC docs specifically mention:

> Only use these options when there are significant benefits from doing so. When you specify these options, the assembler and linker create larger object and executable files and are also slower. These options affect code generation. They prevent optimizations by the compiler and assembler using relative locations inside a translation unit since the locations are unknown until link time. An example of such an optimization is relaxing calls to short call instructions.

#### What if I want the best of both worlds?
Instead of linking this library as a static library file, you could compile all the source code yourself and specifically copy over all the functions you want into an individual source code file and let the compiler optimize things out before it's done with an object file. This is more manual labor and will probably result in slower compile times, but you **might** end up getting a smaller, more performant application.
