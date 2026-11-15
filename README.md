# Rodent IV

A UCI chess engine with configurable playing personalities, based on Sungorus.
This repository continues the community effort around Rodent IV.

## Build

Requires a C++14 compiler, CMake 3.24 or newer, and platform thread support.
The presets use Ninja. With MSVC, run them in a Visual Studio developer shell.

```sh
cmake --preset release
cmake --build --preset release --parallel
ctest --preset release
```

The executable is `build/release/bin/rodentIV` (`rodentIV.exe` on Windows).
Books and personalities are copied alongside it, so it can be launched from
any working directory or registered directly in a UCI chess GUI.

The `debug` and `relwithdebinfo` presets use separate build directories:

```sh
cmake --preset debug
cmake --build --preset debug --parallel
ctest --preset debug
```

Builds without Ninja and IDE-generated projects are also supported:

```sh
cmake -S . -B build/custom -DCMAKE_BUILD_TYPE=Release
cmake --build build/custom --config Release --parallel
ctest --test-dir build/custom -C Release --output-on-failure
```

For Visual Studio, select its generator with `-G`, for example
`-G "Visual Studio 17 2022" -A x64`. Multi-configuration generators place the
executable and its data in `build/custom/bin/Release/` (or `Debug/`).

CPU-specific GCC/Clang optimizations are optional:

```sh
cmake --preset release -DRODENT_NATIVE=ON
```

Such binaries are intended for the build machine. The default is
`RODENT_NATIVE=OFF`. Use `-DBUILD_TESTING=OFF` to omit test registration.

## Install and runtime data

```sh
cmake --install build/release --prefix "$HOME/.local"
```

This installs the engine in `bin/`, books and personalities in
`share/rodentIV/`, and the manual, personality editor, and license in
`share/doc/RodentIV/`. Standard `CMAKE_INSTALL_BINDIR` and
`CMAKE_INSTALL_DATADIR` overrides and `DESTDIR` staging are supported. For a
multi-configuration build, add `--config Release` when installing.

The engine uses the following data locations, in order:

1. `RODENT4HOME`, when set, identifies a data root containing `books/` and
   `personalities/`.
2. Data next to the executable, for build trees and portable distributions.
3. The installed data directory relative to the executable, then the path
   configured at build time as a fallback.

A data root is recognized by `personalities/basic.ini`. Explicit
`RODENT4BOOKS` and `RODENT4PERSONALITIES` values override the individual
subdirectories. Use absolute paths for these two overrides. Existing Android
storage locations remain available as fallbacks.

Installation prefixes can be moved together with their `bin/` and `share/`
directories. For a portable copy, keep the executable and its adjacent
`books/` and `personalities/` directories together.

To edit installed personalities, copy the data to a writable directory and set
`RODENT4HOME` to that directory. The optional GUI personality-set selector also
needs write access to `personalities/basic.ini`.

## Personalities and documentation

- `personalities/basic.ini` lists the personality sets exposed in UCI.
- `personalities/default.txt` contains optional startup settings.
- [Personality editor](docs/personality-editor.html): open locally in a browser
  to create a personality file.
- [Manual](docs/FinalRodent.pdf).
- `books/rodent.bin` is the default main book. The former `exe/rodent.bin` is
  preserved as the alternative `books/main/rodent-legacy.bin`; see its
  [description](books/main/README.md).
- [Opening books and origin notes](books/README.md).

## Verification

`ctest` checks UCI initialization, the `bench 8` reference of **557,513 nodes**,
loading selected personalities and their books from a separate working
directory, and all shipped personality/book file references. Engine input and
output are saved under the build directory's `tests/` folder.

For a manual benchmark, run the engine and enter `bench 8`, followed by `quit`.
Compare node counts before and after changes to engine behavior.

## Contributing

If You add something to Rodent IV, keep in mind the following:

1. It's easier to work with small patches. They are more readable and cause
   less problems in case of an error.
   
2. Starting with bugfixes reduces headache. Fixing things before adding some
   great idea might accidentally do something good to implementation of Your
   idea as well. Going the other way round causes bugs to breed.

3. If fixing a bug required writing some diagnostic code, then diagnostic 
   code stays (perhaps under an #ifdef) because it might help with 
   yet another bug.

4. With a non-functional patch, please test that it is non-functional: run 
   bench command and compare node counts before and after the patch.

5. With an Elo-gaining patch, please run a test of at least 1000 games,
   preferably more.

6. With a feature-adding patch it gets more difficult. Strength is not 
   the main goal of Rodent project, but creating personalities requires
   some Elo to be thrown to the burner, sometimes in hundreds. And I would
   like most personalities to be able to beat Fruit 2.1 to ensure they are
   of decent GM strength. Default Rodent should at least keep 2900-3000 CCRL
   rating. Improvement is nice, but optional.
   
7. Treat Rodent like legacy software. A lot of things in there require
   refactoring, code quality is worse than chess quality, so tread carefully.
   I had no training in IT when I started this project. It taught me enough
   to get a job - and to notice that some of my code sucks.
   
8. Don't overdo automatic tuning. Rodent III lost a lot of style because of that,
   and I had to revert many changes.
   
9. Please don't remove quirks and features. They make Rodent what it is.

10. Enjoy. Rodent is meant to be fun.

