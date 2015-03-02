README
======

Sayuri is ...
-------------

* A chess engine for UCI.
* MIT License software.
* Written in C++11 and only use the standard libraries.
* Easy to build with CMake.
* Equipped Lisp interpreter that is named *Sayulisp*. (unfinished)
    + Sayulisp can generate and operate Chess Engine. (unfinished)
    + Sayulisp can customize algorithm of Search Function. (unfinished)
    + Sayulisp can customize values of Evaluation Function. (unfinished)



Files and Directories
---------------------

### Directories ###

* src : Source files.
* SayuriCompiled : Binary files.
    + Linux : Binary files for Linux.
        - For64Bit : For 64 bit machines.
    + Windows : Binary files for Windows. (Created by Linux's MinGW.)
        - For64Bit : For 64 bit machines.
    + Android : Binary file for Android.
* SayuriLogo : Logo image files.
* SampleGames : Sample game files of Sayuri vs other chess engines.
* DocumentHTML : Documents.

### Files ###

* README.md : README file.
* CMakeLists.txt : A configuration file for CMake.
* LICENSE : The software license.



How To Build with CMake
-----------------------

Using CMake, you can build Sayuri easily.

It's default compiler is *"clang++"*.

(If you want to change a compiler, please change 26th line and 27th line
in "CMakeLists.txt".)

### Build for Release ###

1. Change directory to where "CMakeLists.txt" is placed in.
2. Run the following commands.
    1. `$ mkdir build`
    2. `$ cd build`
    3. `$ cmake ..`
    4. `$ make`
3. *"sayuri"* is built.

### Build for Debug ###

"for Debug" means *"No Optimize"* and *"Embedded Debug Information"*.

1. Change directory to where "CMakeLists.txt" is placed in.
2. Run the following commands.
    1. `$ mkdir build`
    2. `$ cd build`
    3. `$ cmake -DCMAKE\_BUILD\_TYPE=Debug ..`
    4. `$ make`
3. *"sayuri"* is built.



How To Build without CMake
--------------------------

If you don't have CMake, you can build "sayuri" by the following commands
in "src" directory.

If you have "clang" :

    clang++ -std=c++11 -O3 -march=native -pthread -o sayuri *.cpp

If you have "gcc" :

    g++ -std=c++11 -O3 -march=native -pthread -o sayuri *.cpp



How To Make Distributable Packages
------------------------------

After building Sayuri with CMake, run "`$ make dist`" command.

The following packages will be created.

* sayuri-xxxx.xx.xx.tar.Z
* sayuri-xxxx.xx.xx.tar.bz2
* sayuri-xxxx.xx.xx.tar.gz

(Note: "xxxx.xx.xx" is the version number.)



UCI Options
-----------

* To change size of the hash table. (Default: 32 MB, Max: 8192 MB, Min: 8 MB)
    + `setoption name Hash value <Size(MB)>`

* To initialize the hash table.
    + `setoption name Clear Hash`

* To enable Ponder. (Default: true)
    + `setoption name Ponder value <true or false>`

* To change the number of threads. (Default: 1, Max: 64, Min: 1)
    + `setoption name Threads value <Number of threads>`

* To enable analyse mode. (Default: false)
    + `setoption name UCI\_AnalyseMode value <true or false>`
