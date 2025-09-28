# `conficol`: A generic collections library in C that prioritizes **data-security** over performance and convenience
There are a lot of collections libraries out there - probably everybody and their grandmother who have done C for long enough will have implemented their own part of a collections library. What distinguishes this collections library is its priority - "data-security" over performance (speed/memory) and convenience (easy-to-use intuitive API). The name `conficol` comes from `confidential` and `collection` slammed together, hinting at this design prioritization.   

### Core Principles & Ideas:

1. Encapsulate data structures behind an API that never allows a user to explicitly grab a reference to mutate the underlying data structure.
   - No iterators, since iterators provide direct access to data within the data structure for the user
      - _However, the API supports a user providing a function that is performed on each element of a specified range within the data structure, much like a for-each loop would but with less convenient syntax (if only C had lambdas...)_
   - No get functions that return a direct reference to an element of the data structure

2. Provide user with opaque data types
   - The user only ever has a pointer handle to a data structure that they pass to the API, but never the ability to directly modify the data structure's members.

3. Maintain privacy of the data as much as possible.
   - Optionally encrypt/decrypt (using the built-in functions or providing one at constructor time) data with each access as applicable
   - Freeing data structures comes with a scrambling of their underlying memory

4. Prevent memory bugs whenever possible from within the bounds of this library (e.g., out-of-bounds accesses, use-after-frees).
   - Ideally, all memory bugs are prevented altogether, but not all such bugs are preventable from within the bounds of this library (e.g., memory leaks)

5. Provide a rich but orthogonal API.

## Benchmarks
This library is not designed to be the fastest collections library out there, since we trade speed/memory performance for data-security. With that said, after addressing the main goal, efficiency in speed is prioritized, then memory. You may also still want to know how the various data structure implementations compare to some of the popular options out there (e.g., glib, C++'s STL, etc.). See the [`benchmarks/`](./benchmarks/) directory for some of the detailed benchmarks of each data structure's API. There are also benchmarks of different approaches that could be taken in the implementation of functionality of each data structure whenever there was uncertainty.

## Usage
This is intended to be a library that you build and link into your own program. Check out the [inc/README.md](./inc/README.md) for a quick-reference of the API and basic usage of each data structure and the header file for each data structure provides more detailed documentation of the API. Doxygen will be added in the near future.

## Building
### Dependencies
You'll want to run `git submodule update --init` to set up the submodule dependencies in the `submodules/` directory. At the moment, the only dependency is [`danielcota/biski64`](https://github.com/danielcota/biski64) for pseudo-random number generation.
#### Testing
Make sure the following are installed on your system:
- [ThrowTheSwitch/Unity](https://github.com/ThrowTheSwitch/Unity/): used for unit testing
- [google/benchmark](https://github.com/google/benchmark): used for benchmarking

### Notes on Link-Time Optimization
One may ask,

> "If I'm only using a subset of the functions provided in conficol, do I still get the entire library statically linked into my final executable?"

Although this may vary depending on your specific linker, typically, a linker is smart enough to optimize out which individual _object files_ within a library are not used. However, individual symbols within an object file are not _by default_ optimized out, even if unused and not linked to any specific reference within an application. Overall, this technically results in an inefficient application size.   

With that said, a lot of linkers support **Link-Time Optimization** (LTO), which means the linker can get more granular. This will require that the library code is compiled with special compiler options enabled and the final application to be linked with extra linker options. In GCC, for example, compiling this library code with `-ffunction-sections -fdata-sections` causes functions and data to be placed in their own sections within an object file, and then linking the library with your application using `--gc-sections` invokes the linker garbage collector (gc) against unused sections. This will hopefully (although is not guaranteed to) result in a smaller application size. In the `Makefile` within this repository, define `LTO` when invoking make, or simply append `-small` to the typical make targets (e.g., `make release` → `make release-small` !TODO).  

However, there is a potential cost to enabling LTO: runtime speed (also compilation speed). The compiler has to make fewer assumptions about an individual source code file / translation unit, which reduces its ability to make certain optimizations (specifically since the compiler won't know which functions/symbols will be optimized out or not). The GCC docs specifically mention:

> Only use these options when there are significant benefits from doing so. When you specify these options, the assembler and linker create larger object and executable files and are also slower. These options affect code generation. They prevent optimizations by the compiler and assembler using relative locations inside a translation unit since the locations are unknown until link time. An example of such an optimization is relaxing calls to short call instructions.

#### What if I want the best of both worlds?
Instead of linking this library as a static library file, you could compile all the source code yourself and specifically copy over all the functions you want into an individual source code file and let the compiler optimize things out before it's done with an object file. This is more manual labor and will probably result in slower compile times, but you **might** end up getting a smaller, more performant application.
