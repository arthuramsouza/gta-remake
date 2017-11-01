# Building with QtCreator #

## Supported platforms:
- Linux 64 bits (compiled on Linux Mint 16)
- Mac OSX (tested both on Snow Leopard and Mavericks, should work in anything in between)

### Special instructions for OSX:
- Snow Leopard: you must compile **gcc 4.7** (at least) from source - use e.g. Homebrew -
and copy the files from the "mkspecs" directory to /usr/local/Cellar/qt/4.7.2/mkspecs/
- For Lion, ML and Mavericks: the provided compiler already supports C++11 (you must either install XCode or Command Line Tools)

## Not tested, but should work:
- Windows 64 bits (probably will have to change the project file)

## Instructions:
- Open the project file (framework.pro)
- Select the appropriate target (Debug/Release)
- Build it!

### Contact ###

* Dr. Marcelo Cohen (marcelo.cohen@pucrs.br)
