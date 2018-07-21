/*
 * chess_gui.cpp
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


#include "gui_interface.hpp"

using namespace std;

//command line signal handler, as a stand alone function (not a method of a class)
int on_commline(const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line, Glib::RefPtr<Gtk::Application>& app) {
  int argc = 0;
  char** argv = command_line->get_arguments(argc);

  for (int i = 0; i < argc; ++i) {
    std::string cuarg = argv[i];
    if (cuarg == "--help") {
      std::cout << "Yagchess stands for \"Yet Another Gui for CHESS\"" << std::endl;
      std::cout << "You can play chess against another person or a chess engine." << std::endl;
      std::cout << "For more information, read the help panel inside the game or the manual.\n" << std::endl;
    }
    else if (cuarg == "--version") {
      std::cout << "yagchess version 1.0" << std::endl;
    }
  }
  
  //without activate() the window won't be shown, so it's shown only if no arguments are passed
  if (argc == 1) {app->activate();}
  
  return 0;
}

//main function, call the application
int main (int argc, char *argv[]) {
  auto curapp = Gtk::Application::create(argc, argv, "yagchess1-0.app", Gio::APPLICATION_HANDLES_COMMAND_LINE);
  curapp->signal_command_line().connect(sigc::bind(sigc::ptr_fun(&on_commline), curapp), false); //after = false, to call this handler before the default signal handler
  
  ChessWindowGui game = ChessWindowGui(curapp);

  return curapp->run(game);
}
