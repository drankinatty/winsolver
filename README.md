# winsolver
**WinSolver** - Linear System of Equations Solver written in C and C++ native Windows API.

WinSolver is a simple but fast and capable linear system solver that is simply a minimal interface and muliline text control wrapped around a command-line solver to allow simple copy/paste of the coefficent matrix (including the constant vector as the last column) into the editor for solving. Basically it is an editor with a `[Solve...]` button.  A convenience that saves creating an input file with the coefficent matrix and constant vector before calling the solver from the command line. The underlying parser and solver is written entirely in C. (the `mtrx_t.[ch]` source files contain code for handling direct file input as well)

The linear system solver uses Gauss-Jordan Elimination with full pivoting to solve a system of equations containing any number of unknowns up to the physical memory limits of your computer. See [Gaussian Elimination](https://en.wikipedia.org/wiki/Gaussian_elimination) Though the buffer for each line of the coefficent matrix parsed from the edit-control is limited to 8192 characters. If your needs are greater, just change the `MAXC` define at the top of `mtrx_t.h`.

This project is a rewrite of [GtkSolver](https://github.com/drankinatty/gtksolver) to use a native Windows API interface for those running windows to eliminate the need to install Gtk+2.0 libraries and allow the program to run on any recent version of windows as a standalone program.

### Solver Use

The interface is straight forward. The program lauches with a short help message and example of the input format expected in the textview itself. Simply `[Clear]` (or delete) the help message shown and paste your coefficent matrix with the constant vector as the final column and click `[Solve...]`.

The contents of the edit control is converted from `wchar_t` to `char*` and initial text up to the first `.+-[0-9]` is ignored. The first character containing `.+-[0-9]` is taken as the start of the coefficent matrix for the system of equations and is passed to the solver. The input format is flexible, but must be an `[N x N+1]` matrix (representing `N` equations and `N` unknowns **PLUS** the constant vector as the last column). Delimiters between the values are ignored. All values are processed as type `double`. (as defined in `mtrx_t.h`)

The contents of the textview before pressing `[Solve...]` is flexible and could equally be:

     3.0  2.0  -4.0   3.0
     2.0  3.0   3.0  15.0
     5.0 -3.0   1.0  14.0

or


    linear system of equations:

        3    2   -4  |   3
        2    3    3  |  15
        5   -3    1  |  14

or

    3,2,-4,3
    2,3,3,15
    5,-3,1,14

Clicking `[Help]` clears the buffer and redisplays the initial help message.

### Output

Taking the contents of the second example above as the contents of the edit window and clicking `[Solve...]` results in:

    linear system of equations:

        3    2   -4  |   3
        2    3    3  |  15
        5   -3    1  |  14


    Solution Vector:

     x[  0] :   3.0000000
     x[  1] :   1.0000000
     x[  2] :   2.0000000

(where the formatted solution vector is simply concatenated with the existing text written back to the edit control for display in the window)

### Compiling with VS (cl.exe)

Compiling this solver on windows is a simple matter of compiling the windows resource file and then compiling the application. The application can be built with VS10 or later (probably earlier versions as well) You have several options. If space isn't a concern and you want to install the full VS2017 compiler, you can install the VS2017 Community Edition. If you don't want a full VS install or don't have 8G of space to devote to the full install, then all you really need is the [Windows v7.1 SDK](https://www.microsoft.com/en-us/download/details.aspx?id=8279). While it may be antiquated by some standards, for simple windows applications, it provides the VS10 compiler and all needed libraries.

With either the full VS2017 install or the Windows v7.1 SDK, open `Developers Command Prompt` (or `Windows SDK 7.1 Command Prompt`) and simply build the application for the command line. (takes about 15 seconds -- including typing)

To build the resouce file using the windows resource compiler, just change to the directory containing the sources and issue:

    rc /Fowinsolver.res winsolver.rc

To buld the application issue:

    cl /nologo /W3 /wd4996 /Ox /Fewinsolver /Tp winsolver.cpp /Tc memrealloc.c
    /Tc mtrx_t.c user32.lib comctl32.lib gdi32.lib winsolver.res

(the above is all one line - see top of winsolver.cpp for additional details)

That's it. You will now find `winsolver.exe` in the present directory which can be run by simply typing `winsolver`. You can also create a shortcut and add it to your start menu as you see fit.

### Compiling with MinGW

You can also use MinGW or TDM-MinGW to compile the solver. However, to compile on Win32,

    **note:** you must first edit MinGW/Include/commctrl.h to change the default
              target version, otherwise a number of constants needed by the
              controls will not be defined, e.g. make the following changes:

**locate (near the top):**

    /* define _WIN32_IE if you really want it */
    #if 0
    #define _WIN32_IE   0x0300
    #endif

**change to:**

    #if 1
    #define _WIN32_IE   0x0500
    #endif

Now proceed normally compiling the resource file to object:

    windres -o acceltstmg.o acceltst.rc

Then linking the resource as a normal object file when building the solver:

    g++ -Wall -Ofast -o acceltstmg acceltst.cpp -lcomctl32 -luser32 -lgdi32 \
    acceltstmg.o -Wl,-subsystem,windows

(**note:** the only drawback to using MinGW to build the solver is that the `CLEARTYPE_QUALITY` flag is not available to fonts within the edit-control, otherwise there is no difference)

For a bit of background on this problem and solution, see: [MinGW 32-bit Error Buttons Not Declared](https://stackoverflow.com/questions/27663558/opencv-win8-1-mingw32-source-code-error-tbbuttoninfo-was-not-declared-in-this). The change to `commctrl.h` simply changes a define to enable the required constants, e.g.

### License/Copyright

This code is licensed under the GPLv2 open-source license contained in `LICENSE.txt` and copyright David C. Rankin, 2019.
