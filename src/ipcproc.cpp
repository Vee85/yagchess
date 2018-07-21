/*
 * ipcproc.cpp
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


#include <sstream>
#include <unistd.h> //for pipe, fork, read, write, dup2, execl, close function
#include <sys/wait.h> //for kill function

#include "ipcproc.hpp"

IPCproc::IPCproc() {}

IPCproc::IPCproc(std::string pathofcomm) {
  setpath(pathofcomm);
  init();
}

IPCproc::~IPCproc() {
  if (pid > 0) {//parent kills his child
    kill(pid, SIGTERM);
  }
}

//initialize with fork, exec and pipes to start and setup the process 
void IPCproc::init() {
  //two pipes are needed to allow parent and child to communicate bidirectionally
  if (pipe(pipe_ptoc) == -1) {//creating the pipe from parent to child
    std::cerr << "pipe parent to child failed" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  if (pipe(pipe_ctop) == -1) {//creating the pipe from child to parent
    std::cerr << "pipe child to parent failed" << std::endl;
    std::exit(EXIT_FAILURE);
  }
  
  pid = fork(); //forking, from now two identical process exists with the same (this) code, only difference is the value of pid

  if (pid < 0) {
    std::cerr << "fork failed." << std::endl;
    std::exit(EXIT_FAILURE);
  }

  else if (pid == 0) {//code executed by child
    close(pipe_ptoc[1]); //closing write end side of the pipe
    close(pipe_ctop[0]); //closing read end side of the pipe
    
    dup2(pipe_ptoc[0], 0); //read end of pipe descriptor set equal to std in, will be inherited by the program spawned with execl
    dup2(pipe_ctop[1], 1); //write end of pipe descriptor set equal to std out, will be inherited by the program spawned with execl
    
    retex = execl(path.c_str(), command.c_str(), (char *)0); //(char *)0 is the array with options for the command, options not allowed in this case

  } else if (pid > 0) {//code executed by parent
    close(pipe_ptoc[0]); //closing read end side of the pipe
    close(pipe_ctop[1]); //closing write end side of the pipe
    isset = true;

  }
}

//setting the internal variables with the path of the executable to launch
void IPCproc::setpath(std::string pathofcomm) {
  path = pathofcomm;
  std::stringstream pcstream(pathofcomm);
  while (! pcstream.eof()) {
    std::getline(pcstream, command, '/');
  }
}

//send message to the child
void IPCproc::send_to_proc(std::string message) {
  if (pid > 0) {//for parent process
    std::string cc;

    if (message.back() == '\n') {cc = message;}
    else {cc = message + "\n";}
    
    write(pipe_ptoc[1], cc.c_str(), cc.size());
  }
}

//get message from the child
std::string IPCproc::get_from_proc() {
  std::string answ;
  if (pid > 0) {//for parent process
    bool morechar = true;
    bool reading = false;
    while (morechar) {
      for (int i = 0; i < IPCPROC_BUFSIZE; i++) {readbuffer[i] = '\0';} //clearing the buffer char by char, to avoid echo of old data after rewriting
      read(pipe_ctop[0], readbuffer, IPCPROC_BUFSIZE-1);
      std::string pansw(readbuffer);
      answ.append(pansw);
      if (pansw.size() > 0) {reading = true;}
      if (reading) {
        if (pansw.size() < IPCPROC_BUFSIZE-1) {morechar = false;}
        else if (pansw.back() == '\n') {morechar = false;}
      }
    }
  }
  
  return answ;
}

/* Non member functions */
//overloading non member operator<< for IPCproc, it uses get_from_proc
std::ostream& operator<<(std::ostream& os, IPCproc& ipc) {
  std::string ret;
  ret = ipc.get_from_proc();
  os << ret;
  return os;
}

//overloading non member operator>> for IPCproc, it uses send_to_proc
std::istream& operator>>(std::istream &is, IPCproc &ipc) {
  std::string mm;
  is >> mm;
  ipc.send_to_proc(mm);
  return is;
}

