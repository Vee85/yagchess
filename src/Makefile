#  Makefile
#  
#  Copyright 2017 Valentino Esposito <valentinoe85@gmail.com>
#  
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.
#  
#  


CC=g++

OPTIONS=-Wall -g

GTKC=`pkg-config gtkmm-3.0 --cflags --libs`

CO=-std=c++14

DEFC=chess_dconst
NAMEA=chessbase
NAMEB=ipcproc
NAMEC=chessutils
NAMED=chessboard

NAMEIB=gui_interface

MAING=chess_gui

FINAL=yagchess

YAGDIR=$(shell cd .. && pwd)

all: install
	@echo ""

install: mgui
	mv $(MAING).x ../$(FINAL)
	@echo "Exporting path to bash..."
	@grep -qF "$(YAGDIR)" ~/.bashrc || echo export PATH=\$${PATH}:$(YAGDIR) >> ~/.bashrc
	@echo "Done."
	
mgui: $(MAING).cpp $(NAMEA).o $(NAMEB).o $(NAMEC).o $(NAMED).o $(NAMEIB).o
	$(CC) $(MAING).cpp $(NAMEA).o $(NAMEB).o $(NAMEC).o $(NAMED).o $(NAMEIB).o -o $(MAING).x $(OPTIONS) $(CO) $(GTKC)
	
$(NAMEA).o: $(DEFC).hpp $(NAMEA).hpp $(NAMEA).cpp
	$(CC) -c $(NAMEA).cpp -o $(NAMEA).o $(OPTIONS) $(CO)
	
$(NAMEB).o: $(NAMEA).o $(NAMEB).hpp $(NAMEC).hpp $(NAMEB).cpp
	$(CC) -c $(NAMEB).cpp -o $(NAMEB).o $(OPTIONS) $(CO)

$(NAMEC).o: $(NAMEC).hpp $(NAMEC).cpp
	$(CC) -D yagdir="\"$(YAGDIR)\"" -c $(NAMEC).cpp -o $(NAMEC).o $(OPTIONS) $(CO)

$(NAMED).o: $(DEFC).hpp $(NAMEA).hpp $(NAMEB).hpp $(NAMEC).hpp $(NAMED).hpp $(NAMED).cpp
	$(CC) -c $(NAMED).cpp -o $(NAMED).o $(OPTIONS) $(CO)
	
$(NAMEIB).o: $(DEFC).hpp $(NAMEA).hpp $(NAMEB).hpp $(NAMEC).hpp $(NAMED).hpp $(NAMEIB).hpp $(NAMEIB).cpp
	$(CC) -D yagdir="\"$(YAGDIR)\"" -c $(NAMEIB).cpp -o $(NAMEIB).o $(OPTIONS) $(CO) $(GTKC)

clean:
	@echo "Cleaning..."
	@rm *.o ../$(FINAL)
	@echo "Removing from bashrc..."
	@grep -v "$(YAGDIR)" ~/.bashrc > tempbrc
	@cp tempbrc ~/.bashrc
	@rm tempbrc
	@echo "Done."

help:
	@echo "To compile $(FINAL) you need the gnu gcc compiler and the gtkmm3.0 library."
	@echo "The gcc compiler is already installed on most linux distributions."
	@echo "You can install gtkmm by using you package manager. Type:"
	@echo "  apt-get install libgtkmm-3.0-dev"
	@echo "for Debian based distributions or"
	@echo "  yum install gtkmm30-docs"
	@echo "for Fedora/Red Hat based distributions. See also https://www.gtkmm.org/en/download.html"
	@echo "You probably need root privilege to install the packages." 
	@echo ""
	@echo "Once you have installed gtkmm, in the src directory type:"
	@echo "  make install"
	@echo "to install $(FINAL). Type:"
	@echo "  make clean"
	@echo "to remove executable and object files."
