/*
 * ipcproc.hpp
 * 
 * Copyright 2017 Valentino Esposito <valentinoe85@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#ifndef IPCPROC_H_DEF
#define IPCPROC_H_DEF 1 

#include <iostream>
#include <string>

#define IPCPROC_BUFSIZE 51

/* class to allow bidirectional IPC through fork and pipe
 */
class IPCproc {
  private:
    std::string path, command;
    int retex;
    int pipe_ptoc[2], pipe_ctop[2];
    char readbuffer[IPCPROC_BUFSIZE];
    bool isset = false;
    pid_t pid; //the process id discriminating between parent and child
    
  public:
    IPCproc();
    IPCproc(std::string);
    virtual ~IPCproc();
    
    void init(void);
    void setpath(std::string);
    bool isactive(void) {return isset;}

    void send_to_proc(std::string);
    std::string get_from_proc(void);
};

/* Non member function definition, to overload operators
 */
std::ostream& operator<<(std::ostream& os, IPCproc& ipc);

std::istream& operator>>(std::istream& is, IPCproc& ipc);

#endif
