## General information

Yagchess is a free GUI to play Chess. It can be used to play chess against other human players on the same machine.

It can be used to play chess against a chess engine, however no chess engine is provided with the yagchess distribution. You have to obtain and install a compatible chess engine, and set Yagchess to use it.

Yagchess uses the UCI protocol to interact with the chesse engine, hence any UCI-compatible chess engine should work. You can find free chess engines on the web, such as Stockfish or Gnuchess.

Yagchess stands for Yet Another Gui for CHESS.

NOTE: the installation process should be improved.

## Files

This distribution of Yagchess version 1.0 consists of the following files:

* README.md, the file you are currently reading.
* LICENSE.txt, a text file containing the GNU General Public License.
* src/, a subdirectory containing the source code in C++, including a Makefile that can be used to compile Yagchess.
* resources/, a subdirectory containing xml and css files needed to build the GUI.
* resources/chess_pieces_icons/, a subdirectory of resources containing the 12 images of the pieces (png format).
* documentation/, a directory containing the manual in pdf format and the tex source of the manual.


## Installation

Yagchess can be installed on Linux systems. To compile yagchess, you need the gnu gcc compiler and the gtkmm3.0 (or later) library. The gcc compiler is already installed on most linux distributions. You can install gtkmm by using you package manager. Type:

> apt-get install libgtkmm-3.0-dev

for Debian based distributions or

> yum install gtkmm30-docs

for Fedora/Red Hat based distributions. See also https://www.gtkmm.org/en/download.html
You probably need root privilege to install the packages.


To compile the program, once you have installed gtkmm, go in the src directory and type:

> make install

To remove executable and object files, go in the src directory and type:

> make clean


## Terms of use

Yagchess is free and distributed under the GNU General Public License (GPL). Essentially, this means that you are free to do almost exactly what you want with the program, including distributing it among your friends, making it available for download from your web site, selling it (either by itself or as part of some bigger software package), or using it as the starting point for a software project of your own.

The only real limitation is that whenever you distribute Yagchess in some way, you must always include the full source code, or a pointer to where the source code can be found. If you make any changes to the source code, these changes must also be made available under the GPL.

For full details, read the copy of the GPL found in the file named gpl-3.0.txt
