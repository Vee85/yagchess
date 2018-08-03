/*
 * gui_interface.cpp
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


#include <iomanip>

#include "gui_interface.hpp"

//static global variables (it is not an instance of any class), their scope is this file only
static bool atwork(false);
static std::string baserespath(std::string(yagdir) + "/resources/");

/* Methods of class ChessPGNGui
 */
ChessPGNGui::ChessPGNGui(std::string fn, char m) : ChessPGN(fn, m) {}

ChessPGNGui::~ChessPGNGui() {}

//select the game from the PGN file, call the appropriate dialog only if there is more than one game
unsigned int ChessPGNGui::selectgame() {
  unsigned int res = 0;
  gameok = true;

  if (numgames() > 1) {
    PGNselgame seldialog(this);
    seldialog.run();

    if (seldialog.choicemade()) {
      res = seldialog.chosen();
    } else {gameok = false;}
  }
  
  return res;
}


/* Methods of the nested class PGNselgame
 */
ChessPGNGui::PGNselgame::PGNselgame() {}

ChessPGNGui::PGNselgame::PGNselgame(ChessPGNGui* ptog) {
  ptopgn = ptog;
  set_title("Choose a game");
  set_size_request(500, 250);
  chmade = false;
  container.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  
  insidels = Gtk::ListStore::create(modelgamelist);
  insidetv.set_model(insidels);
  
  buildlist(); //filling the liststore
  
  //here we select which columns are actually shown in the window
  insidetv.append_column("Index", modelgamelist.col_index);
  insidetv.append_column("Event", modelgamelist.col_event);
  insidetv.append_column("Site", modelgamelist.col_site);
  insidetv.append_column("Date", modelgamelist.col_date);
  insidetv.append_column("Round", modelgamelist.col_round);
  insidetv.append_column("White", modelgamelist.col_white);
  insidetv.append_column("Black", modelgamelist.col_black);
  insidetv.append_column("Result", modelgamelist.col_result);
  
  container.add(insidetv);
  mainbox->pack_start(container);
  
  bopen.set_label("Open");
  bclose.set_label("Close");
  bopen.signal_clicked().connect(sigc::bind<bool>(sigc::mem_fun(*this, &PGNselgame::on_clicked_button), true));
  bclose.signal_clicked().connect(sigc::bind<bool>(sigc::mem_fun(*this, &PGNselgame::on_clicked_button), false));
  
  bbox.pack_start(bclose);
  bbox.pack_start(bopen);
  mainbox->pack_start(bbox, Gtk::PACK_SHRINK);
  
  show_all_children();
}

ChessPGNGui::PGNselgame::~PGNselgame() {}

//building the page with the list of games
void ChessPGNGui::PGNselgame::buildlist() {
  insidels->clear();
  
  //filling the liststore with the games
  Gtk::TreeModel::Row row;
  for (unsigned int i = 0; i < ptopgn->numgames(); i++) {
    PGNgame* cgame = ptopgn->getgame(i);
    row = *(insidels->append());
    row[modelgamelist.col_index] = i;
    row[modelgamelist.col_event] = cgame->getfield("Event");
    row[modelgamelist.col_site] = cgame->getfield("Site");
    row[modelgamelist.col_date] = cgame->getfield("Date");
    row[modelgamelist.col_round] = cgame->getfield("Round");
    row[modelgamelist.col_white] = cgame->getfield("White");
    row[modelgamelist.col_black] = cgame->getfield("Black");
    row[modelgamelist.col_result] = cgame->getfield("Result");
  }
}

//signal handler
void ChessPGNGui::PGNselgame::on_clicked_button(bool op) {
  if (op) {
    //get the selected row
    Glib::RefPtr<Gtk::TreeSelection> seltv = insidetv.get_selection();
    Gtk::TreeModel::iterator iter = seltv->get_selected();

    if (iter) {//if anything is selected
      Gtk::TreeModel::Row row = *iter;
      chogame = row[modelgamelist.col_index];
      chmade = true;
    }  
  }
  hide();
}


/* Methods of class ChessUCIGui
 */
ChessUCIGui::ChessUCIGui() : ChessUCI() {}

ChessUCIGui::ChessUCIGui(std::string cen, std::string ptc) : ChessUCI(cen, ptc) {}

ChessUCIGui::~ChessUCIGui() {}

//building the gui to allow the user to set the chess engine options
void ChessUCIGui::set_options(std::string tle) {
  if (checkvector.size() > 0) {
    ChessEngOptGui optdialog = ChessEngOptGui(this, tle);
    optdialog.run();
  } else {
    Gtk::MessageDialog mess("Warning");
    mess.set_secondary_text("It seems that your chess engine do not have any option you can set or the options have not been loaded by the GUI");
    mess.run();
  }
}

//display the chess engine message in the dedicated textarea
void ChessUCIGui::ch_eng_communication(std::string stmess) {
  stmess.append("\n");
  send_cemessage()(stmess);
}

//constructors of the nested class
ChessUCIGui::ChessEngOptGui::ChessEngOptGui() {}

ChessUCIGui::ChessEngOptGui::ChessEngOptGui(ChessUCIGui* encc, std::string title) {
  ptoucigui = encc;
  set_title(title);
  
  /* the iterator in these 'for' loops is a pointer to a pointer */
  //building the check options
  for (auto it = ptoucigui->checkvector.begin(); it != ptoucigui->checkvector.end(); ++it) {
    Gtk::Box* boxcont = Gtk::manage(new Gtk::Box);
    auto chb = Gtk::manage(new Gtk::CheckButton());
    chb->set_label((*it)->name);
    chb->set_active((*it)->value);
    chb->signal_toggled().connect(sigc::bind(sigc::mem_fun(*this, &ChessEngOptGui::on_type_check_toggled), *it, chb));
    boxcont->pack_start(*chb);
    mainbox->pack_start(*boxcont, Gtk::PACK_SHRINK);
  }
  
  //building the spin options
  for (auto it = ptoucigui->spinvector.begin(); it != ptoucigui->spinvector.end(); ++it) {
    Gtk::Box* boxcont = Gtk::manage(new Gtk::Box);
    auto labent = Gtk::manage(new Gtk::Label((*it)->name));
    auto spb = Gtk::manage(new Gtk::SpinButton());
    spb->set_digits(0);
    spb->set_increments(1.0, 10.0);
    spb->set_range((*it)->min, (*it)->max);
    spb->set_value((*it)->value);
    spb->set_numeric(true); //to allow only numbers if the user types manually
    spb->signal_value_changed().connect(sigc::bind(sigc::mem_fun(*this, &ChessEngOptGui::on_type_spin_changed), *it, spb));
    boxcont->pack_start(*labent);
    boxcont->pack_start(*spb);
    mainbox->pack_start(*boxcont, Gtk::PACK_SHRINK);
  }
  
  //building the combo options
  for (auto it = ptoucigui->combovector.begin(); it != ptoucigui->combovector.end(); ++it) {
    Gtk::Box* boxcont = Gtk::manage(new Gtk::Box);
    auto labcom = Gtk::manage(new Gtk::Label((*it)->name));
    auto cobox = Gtk::manage(new Gtk::ComboBox());
    ComboModel* cobmod = new ComboModel();
    storecombomodels.push_back(cobmod); //remembering the dynamically created model
    Glib::RefPtr<Gtk::ListStore> cmtable = Gtk::ListStore::create(*cobmod);
    cobox->set_model(cmtable);
    
    //filling the options
    Gtk::TreeModel::Row row;
    for (unsigned int j = 0; j < (*it)->allowedvals.size(); j++) {
      row = *(cmtable->append());
      row[cobmod->optval] = (*it)->allowedvals[j];
      if (row[cobmod->optval] == (*it)->value) {cobox->set_active(j);} //to set the initial value in the combobox
    }
    
    cobox->pack_start(cobmod->optval);
    cobox->signal_changed().connect(sigc::bind(sigc::mem_fun(*this, &ChessEngOptGui::on_type_combo_changed), *it, cobmod, cobox));
    
    boxcont->pack_start(*labcom);
    boxcont->pack_start(*cobox);
    mainbox->pack_start(*boxcont, Gtk::PACK_SHRINK);
  }
   
  //building the button options
  for (auto it = ptoucigui->buttonvector.begin(); it != ptoucigui->buttonvector.end(); ++it) {
    Gtk::Box* boxcont = Gtk::manage(new Gtk::Box);
    auto bt = Gtk::manage(new Gtk::Button());
    bt->set_label((*it)->name);
    bt->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &ChessEngOptGui::on_type_button_clicked), *it));
    boxcont->pack_start(*bt);
    mainbox->pack_start(*boxcont, Gtk::PACK_SHRINK);
  }
  
  //building the string / entry options
  for (auto it = ptoucigui->entryvector.begin(); it != ptoucigui->entryvector.end(); ++it) {
    Gtk::Box* boxcont = Gtk::manage(new Gtk::Box);
    auto labent = Gtk::manage(new Gtk::Label((*it)->name));
    auto ent = Gtk::manage(new Gtk::Entry());
    ent->set_text((*it)->value);
    ent->signal_changed().connect(sigc::bind(sigc::mem_fun(*this, &ChessEngOptGui::on_type_entry_changed), *it, ent));
    boxcont->pack_start(*labent);
    boxcont->pack_start(*ent);
    mainbox->pack_start(*boxcont, Gtk::PACK_SHRINK);
  }
  
  //adding last buttons
  applybut.set_label("Apply changes");
  applybut.signal_clicked().connect(sigc::bind<bool>(sigc::mem_fun(*this, &ChessEngOptGui::on_abutton_clicked), false));
  lastbox.pack_start(applybut, Gtk::PACK_SHRINK);
  
  godefbut.set_label("Reset default values");
  godefbut.signal_clicked().connect(sigc::bind<bool>(sigc::mem_fun(*this, &ChessEngOptGui::on_abutton_clicked), true));
  lastbox.pack_start(godefbut, Gtk::PACK_SHRINK);

  savebut.set_label("Save changes");
  savebut.signal_clicked().connect(sigc::mem_fun(*this, &ChessEngOptGui::on_save_pref_clicked));
  lastbox.pack_start(savebut, Gtk::PACK_SHRINK);
  
  mainbox->pack_start(lastbox);
  show_all_children();
}

ChessUCIGui::ChessEngOptGui::~ChessEngOptGui() {
  for (unsigned int i = 0; i < storecombomodels.size(); i++) {delete storecombomodels[i];}
}

//signal handler for checkbutton built by nested class
void ChessUCIGui::ChessEngOptGui::on_type_check_toggled(CheckOption* chstru, Gtk::CheckButton* chbut) {chstru->value = chbut->get_active();}

//signal handler for button built by nested class
void ChessUCIGui::ChessEngOptGui::on_type_spin_changed(SpinOption* spstru, Gtk::SpinButton* spbut) {spstru->value = spbut->get_value();}

//signal handler for combobox built by the nested class
void ChessUCIGui::ChessEngOptGui::on_type_combo_changed(ComboOption* costru, ComboModel* cmm, Gtk::ComboBox* cobut) {
  Gtk::TreeModel::iterator irow = cobut->get_active();
  if (irow) {
    Gtk::TreeModel::Row row = *irow;
    Glib::ustring tempstr = row[cmm->optval];
    costru->value = tempstr;
  }
}

//signal handler for button built by nested class
void ChessUCIGui::ChessEngOptGui::on_type_button_clicked(ButtonOption* bustru) {
  std::string oname = bustru->name;
  ptoucigui->paramoptions(oname, "");
  ptoucigui->sendcomm(3);
}

//signal handler for entry built by nested class
void ChessUCIGui::ChessEngOptGui::on_type_entry_changed(EntryOption* enstru, Gtk::Entry* ent) {enstru->value = ent->get_text();}

//signal handler for apply and set default button
void ChessUCIGui::ChessEngOptGui::on_abutton_clicked(bool todef) {ptoucigui->applycepref(todef);}

//signal handler to save options in dedicated file
void ChessUCIGui::ChessEngOptGui::on_save_pref_clicked() {
  ptoucigui->applycepref(false); //applying current options
  ptoucigui->writecepref(); //saving current options in the file
}


/*Methods of ChessPlayerGui
 */
ChessPlayerGui::ChessPlayerGui() {binding_signals();}

ChessPlayerGui::ChessPlayerGui(c_color cc) : ChessPlayer(cc) {binding_signals();}

ChessPlayerGui::ChessPlayerGui(c_color cc, bool ish) : ChessPlayer(cc, ish) {binding_signals();}

ChessPlayerGui::~ChessPlayerGui() {}

//signal handler: turnation_end can not be called directly as signal handler by ChessPlayerGui because
//the pointer is not setted when the class is istantiated, resuling in a segmentation fault error at runtime
void ChessPlayerGui::on_status_event_sent(bool st) {
  ownboard->turnation_end(st);
}

//signalhandler
bool ChessPlayerGui::on_firstsel_event_sent(c_color cc, ChessSquare* firstsel) {
  ChessBoardGui* guiboard = dynamic_cast<ChessBoardGui*>(ownboard);
  
  bool res = guiboard->check_startinggui(cc, firstsel);
  return res;
}

//binding the signals to the ChessBoard method
void ChessPlayerGui::binding_signals() {
  send_status_signal().connect(sigc::mem_fun(*this, &ChessPlayerGui::on_status_event_sent));
  send_firstsel_signal().connect(sigc::mem_fun(*this, &ChessPlayerGui::on_firstsel_event_sent));
}

//managing the human player action, setting the pointers for the moving
bool ChessPlayerGui::player_act() {
  if (move_from == nullptr) {
    //action for setting the starting of the move (starting square)
    ChessSquare* mf = getsquare(sel_x, sel_y);
    bool okfs = send_firstsel_signal()(color, mf);
    
    if (okfs) {
      move_from = getsquare(sel_x, sel_y);
      send_status_signal()(false);
    }
    
  } else {
    //action to setting the conclusion of the move (arrival square)
    move_to = getsquare(sel_x, sel_y);
    send_status_signal()(true);
    
    move_from = nullptr;
    move_to = nullptr;
  }
  
  return true;
}


/*Methods of ChessSquareGui
 */
bool ChessSquareGui::allowclick = false; //static member initialization

ChessSquareGui::ChessSquareGui() {
  set_border_width(0);

  //set the pointer to the css external file (just for the background color), and the style context for this frame. The widget name is set by ChessBoardGui
  square_cssprov = Gtk::CssProvider::create();
  auto stcon = get_style_context();
  stcon->add_provider(square_cssprov, GTK_STYLE_PROVIDER_PRIORITY_USER);
  std::string csspath = baserespath + "chesstyle.css";
  square_cssprov->load_from_path(csspath);
    
  //add eventbox and image widget as child of the frame, to be shown.
  add(eventsq);
  eventsq.add(insidesq);
  
  //bind actions to event box.
  eventsq.set_events(Gdk::BUTTON_PRESS_MASK);
  eventsq.signal_button_press_event().connect(sigc::mem_fun(*this, &ChessSquareGui::onclick));
  
  //creating the content of the popup menu
  auto item = Gtk::manage(new Gtk::MenuItem("_Menacing", true));
  item->signal_activate().connect(sigc::bind<bool>(sigc::mem_fun(*this, &ChessSquareGui::onpopmenacing), true));
  popm.append(*item);

  item = Gtk::manage(new Gtk::MenuItem("_Protecting", true));
  item->signal_activate().connect(sigc::bind<bool>(sigc::mem_fun(*this, &ChessSquareGui::onpopmenacing), false));
  popm.append(*item);

  item = Gtk::manage(new Gtk::MenuItem("_Menaced by", true));
  item->signal_activate().connect(sigc::bind<bool>(sigc::mem_fun(*this, &ChessSquareGui::onpopmenacedby), true));
  popm.append(*item);
  
  item = Gtk::manage(new Gtk::MenuItem("_Protected by", true));
  item->signal_activate().connect(sigc::bind<bool>(sigc::mem_fun(*this, &ChessSquareGui::onpopmenacedby), false));
  popm.append(*item);
  
  popm.show_all(); //Show all menu items when the menu pops up
}

ChessSquareGui::ChessSquareGui(c_color c, ChessBoardGui* pp, int a, int b) : ChessSquareGui() {
  setcolor(c);
  setcoord(a, b);
  setpointer(pp);
}

ChessSquareGui::~ChessSquareGui() {}

//set the name of the ChessSquareGui widget to load che correct CSS style to give the proper background color
void ChessSquareGui::setcolor(c_color cc) {
  col = cc;
  if (col == black) {set_name("blacksquare");}
  else if (col == white) {set_name("whitesquare");}
}

//set internal coordinates of the square
void ChessSquareGui::setcoord(int a, int b) {
  x = a;
  y = b;
}

//show the image corresponding to a piece in the square
void ChessSquareGui::showpiece(Piece* pc) {
  if (pc != nullptr) {
    if (pc->geticon() == "noicon") {insidesq.clear();}
    else {
      std::string base = baserespath + "chess_pieces_icons/";
      std::string path = base + pc->geticon();
      insidesq.set(path);
    }
  } else {
    insidesq.clear();
  }
}

//change background color to indicate a selected square
bool ChessSquareGui::applyselect(bool ss) {
  selected = ss;
  std::string prefix;
  
  if (selected) {prefix = "selected";}
  else {prefix = "";}
  
  if (col == black) {set_name(prefix + "blacksquare");}
  else if (col == white) {set_name(prefix + "whitesquare");}
  
  return selected;
}

//signal handler for a click in the event box surrounding the image (should be coincident with the frame / square).
bool ChessSquareGui::onclick(GdkEventButton* evbut) {
  if (ownboard != nullptr) {
    if (allowclick) {
      if (! atwork) {
        atwork = true;
        if (evbut->button == 1) {//left click
          ownboard->player_moving->sel_x = x;
          ownboard->player_moving->sel_y = y;
          ownboard->player_moving->player_act();
        } else if (evbut->button == 3) {//right click
          ownboard->unselectallsq();
          popm.popup(evbut->button, evbut->time);
        }
        atwork = false;
      } else {
        if (evbut->button == 3) {ownboard->engine_stop();} //sending a stop command with the right click only
      }
    }
  }
  return true;
}

//signal handler for popup menu entry, to select pieces menaced / protected by the piece in the square
void ChessSquareGui::onpopmenacing(bool seladv) {
  //no need to check for ownboard pointer, the method cannot be called if it is not set (the popup menu will not appear)
  Piece* examp = ownboard->getsquare(x, y)->p;

  if (examp != nullptr) {
    c_color cofp, colofint;
    cofp = examp->getcolor();
    if (seladv) {colofint = !cofp;}
    else {colofint = cofp;}
    
    std::vector<ChessSquare*> allmen, tbs; 
    allmen = ownboard->ismenacing(examp, 2);
    for (unsigned int i = 0; i < allmen.size(); i++) {
      if (allmen[i]->p != nullptr) {
        if (allmen[i]->p->getcolor() == colofint) {tbs.push_back(allmen[i]);}
      }
    }
    ownboard->selectguisquares(tbs);

  } else {
    std::string mess = "No piece here!";
    Gtk::MessageDialog dial(mess);
    dial.run();
  }
}

//signal handler for popup menu entry, to select pieces who are menacing / protecting the square
void ChessSquareGui::onpopmenacedby(bool seladv) {
  //no need to check for ownboard pointer, the method cannot be called if it is not set (the popup menu will not appear)
  ChessSquare* chs = ownboard->getsquare(x, y);
  c_color cref, cex;
  if (chs->p == nullptr) {cref = ownboard->player_moving->wpcolor();}
  else {cref = chs->p->getcolor();}
    
  if (seladv) {cex = !cref;}
  else {cex = cref;}
  
  std::vector<Piece*> pieceset = ownboard->ismenacedby(cex, chs, false, 2);
  std::vector<ChessSquare*> tbs;
  for (unsigned int i = 0; i < pieceset.size(); i++) {tbs.push_back(ownboard->getsquare(pieceset[i]->getx(), pieceset[i]->gety()));}
  
  ownboard->selectguisquares(tbs);
}


/* Methods of class ChessTimer
 */
ChessTimer::ChessTimer() {
  set_border_width(0);

  //set the pointer to the css external file (just for the background color), and the style context for this frame.
  t_cssprov = Gtk::CssProvider::create();
  auto stcon = get_style_context();
  stcon->add_provider(t_cssprov, GTK_STYLE_PROVIDER_PRIORITY_USER);
  std::string csspath = baserespath + "chesstyle.css";
  t_cssprov->load_from_path(csspath);

  //creating the slot (i.e. type-safe representations of callback methods and functions) to be called at timeout
  sigc::slot<bool> tslot = sigc::mem_fun(*this, &ChessTimer::updatetimerval);
  
  //connecting the slot to the timeout. Note that SignalTimeout connect method requires an extra int argument: the time interval in milliseconds between a call and the next one
  tconn = Glib::signal_timeout().connect(tslot, 1000);
  
  add(tdisplay);
}

ChessTimer::~ChessTimer() {}

//formatting the timer, from all seconds to a string with hours, minutes, seconds
std::string ChessTimer::formattimer(int t) {
  std::string res;
  if (t < 1) {res = "---";}
  else {
    int hours, minutes, seconds;
    std::ostringstream txt;
    hours = t / 3600;
    if (hours > 0) {txt << hours << ":";}
    minutes = (t % 3600) / 60;
    txt << std::setfill('0') << std::setw(2) << minutes << ":";
    seconds = (t % 3600) % 60;
    txt << std::setfill('0') << std::setw(2) << seconds;
    res = txt.str();
  }
  
  return res;
}

//update label showing the time
void ChessTimer::updatetimerlab(void) {
  std::string ft = formattimer(chesstimer);
  tdisplay.set_label(ft);
}

//update internal variable counting the time
bool ChessTimer::updatetimerval() {
  if (running && chesstimer > 0) {
    chesstimer--;
    updatetimerlab();

    if (chesstimer == 0) {endontime(color, true, true);}
  }
  
  return true;
}

//initialize the timer
void ChessTimer::init(ChessWindowGui* p, c_color cc, int t) {
  ptowin = p;
  color = cc;
  chesstimer = t;
  running = false;
  
  std::string name;
  
  if (color == white) {name = "whitesquare";}
  else if (color == black) {name = "blacksquare";}
  set_name(name);
  
  //connecting the gameend signal to the callback method 
  endontime.connect(sigc::mem_fun(*ptowin, &ChessWindowGui::on_game_end));
  
  updatetimerlab();
}

//stop a timer for ever, to be used when the game is concluded
void ChessTimer::deltimer() {
  chesstimer = -1;
} 

//stop / go the timer
void ChessTimer::stopgotimer() {
  running = ! running;
}

/* Methods of ChessWindowGui class
 */
ChessWindowGui::ChessWindowGui() {
  std::cout << baserespath << std::endl;
  cnfgf = new ChessConfig();
  cnfgf->initcc(std::string(yagdir) + "/.yagchess_config", std::string(yagdir) + "/.yagchess_starthum");
  starthumpl = cnfgf->getstarthumpl();
  ucianalys = nullptr;
  
  //handling the graphical parameters of the chessboard
  set_title("Yagchess");
  set_default_size(730, 930);
  set_border_width(15);
  
  wideboard.set_size_request(715, 715);
  board.set_size_request(700, 700);
  
  //packing the widgets
  add(mainbox);
  
  //building the menu actions and binding the signal handlers
  menuactiongroup = Gio::SimpleActionGroup::create();
  menuactiongroup->add_action("new", sigc::bind<std::string>(sigc::mem_fun(*this, &ChessWindowGui::on_action_game_new), ""));
  menuactiongroup->add_action("load", sigc::mem_fun(*this, &ChessWindowGui::on_action_game_load));
  menuactiongroup->add_action("readfen", sigc::mem_fun(*this, &ChessWindowGui::on_action_game_readfen));
  menuactiongroup->add_action("save", sigc::mem_fun(*this, &ChessWindowGui::on_action_game_save));
  menuactiongroup->add_action("close", sigc::mem_fun(*this, &ChessWindowGui::on_action_game_close));
  menuactiongroup->add_action("quit", sigc::mem_fun(*this, &ChessWindowGui::on_action_game_quit));
  menuactiongroup->add_action("preferences", sigc::mem_fun(*this, &ChessWindowGui::on_action_game_preferences));

  menuactiongroup->add_action("start", sigc::mem_fun(*this, &ChessWindowGui::on_action_game_start));
  menuactiongroup->add_action("back", sigc::bind<int>(sigc::mem_fun(*this, &ChessWindowGui::on_action_game_back), 1));
  menuactiongroup->add_action("backall", sigc::bind<int>(sigc::mem_fun(*this, &ChessWindowGui::on_action_game_back), -1));
  menuactiongroup->add_action("forward", sigc::bind<int>(sigc::mem_fun(*this, &ChessWindowGui::on_action_game_forward), 1));
  menuactiongroup->add_action("forwardall", sigc::bind<int>(sigc::mem_fun(*this, &ChessWindowGui::on_action_game_forward), -1));
  menuactiongroup->add_action("resign", sigc::mem_fun(*this, &ChessWindowGui::on_action_game_resign));

  menuactiongroup->add_action("prefcewh", sigc::bind<c_color>(sigc::mem_fun(*this, &ChessWindowGui::on_action_setceoptions), white));
  menuactiongroup->add_action("prefcebl", sigc::bind<c_color>(sigc::mem_fun(*this, &ChessWindowGui::on_action_setceoptions), black));
  menuactiongroup->add_action("stopchen", sigc::mem_fun(*this, &ChessWindowGui::on_stop_chess_engine));
  menuactiongroup->add_action("analysechen", sigc::mem_fun(*this, &ChessWindowGui::on_chess_engine_analyse));

  menuactiongroup->add_action("historysaveshort", sigc::bind<bool>(sigc::mem_fun(*this, &ChessWindowGui::on_action_game_history_save), false));
  menuactiongroup->add_action("historysavelong", sigc::bind<bool>(sigc::mem_fun(*this, &ChessWindowGui::on_action_game_history_save), true));
  menuactiongroup->add_action("historyshowshort", sigc::bind<bool>(sigc::mem_fun(*this, &ChessWindowGui::on_action_game_history_show), false));
  menuactiongroup->add_action("historyshowlong", sigc::bind<bool>(sigc::mem_fun(*this, &ChessWindowGui::on_action_game_history_show), true));
  menuactiongroup->add_action("fennot", sigc::mem_fun(*this, &ChessWindowGui::on_action_gen_fen));
  
  menuactiongroup->add_action("about", sigc::mem_fun(*this, &ChessWindowGui::on_action_printabout));
  menuactiongroup->add_action("help", sigc::mem_fun(*this, &ChessWindowGui::on_action_printhelp));
  
  insert_action_group("chess", menuactiongroup);
  
  //building the menu layout
  menubuilder = Gtk::Builder::create();
    
  try {
    std::string xmlpath = baserespath + "menulayout.xml";
    menubuilder->add_from_file(xmlpath);
  } catch (const Glib::Error& ex) {
    std::cerr << "Building menu failed: " << ex.what();
  }
  
  auto menuobj = menubuilder->get_object("menubar");
  auto gmenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(menuobj);
  
  if (!gmenu) {
    g_warning("GMenu not found");
  } else {
    auto pmenubar = Gtk::manage(new Gtk::MenuBar(gmenu));

    //Add the MenuBar to the window:
    mainbox.pack_start(*pmenubar, Gtk::PACK_SHRINK); //here the menubar, on top of everything
  }

  //packing the timers
  upperbar.pack_start(whtimer);
  upperbar.pack_start(bltimer);
  mainbox.pack_start(upperbar, Gtk::PACK_SHRINK);
  
  //creating the gui chessboard
  ChessSquareGui* sq;
  c_color sqc;
  
  for (int i = CHVector::min_x; i < CHVector::max_x; i++) {
    for (int j = CHVector::min_y; j < CHVector::max_y; j++) {
      sq = getsquaregui(i, j);

      if ((i+j) % 2 == 0) {sqc = white;}
      else {sqc = black;}
      sq->setcolor(sqc);
      sq->setcoord(i, j);
      
      board.attach(*sq, i, j, 1, 1);
    }
  }
  
  //creating the coordinate labels
  for (int i = CHVector::min_x; i < CHVector::max_x; i++) {
    std::string labtexta(1, ChessCoordinates::xout(i));
    xlaba[i].set_label(labtexta);
    xlabb[i].set_label(labtexta);
    upxlabelbox.pack_start(xlaba[i]);
    loxlabelbox.pack_start(xlabb[i]);
  }
  
  for (int i = CHVector::min_y; i < CHVector::max_y; i++) {
    std::string labtextb(1, ChessCoordinates::yout(i));
    ylaba[i].set_label(labtextb);
    ylabb[i].set_label(labtextb);
    leylabelbox.pack_start(ylaba[i]);
    riylabelbox.pack_start(ylabb[i]);
  }
  
  //packing coordinate labels and chessboard
  wideboard.attach(upxlabelbox, 1, 0, 8, 1);
  wideboard.attach(leylabelbox, 0, 1, 1, 8);
  wideboard.attach(board, 1, 1, 8, 8);
  wideboard.attach(riylabelbox, 9, 1, 1, 8);
  wideboard.attach(loxlabelbox, 1, 9, 8, 1);
  
  board.set_row_homogeneous(true);
  board.set_column_homogeneous(true);
  
  centralgroup.pack_start(wideboard, Gtk::PACK_SHRINK, 5); //packing the chessboard
  
  //buttons for chess engine control
  butstopcheng.set_label("Stop CE");
  butstopcheng.set_tooltip_text("Stop chess engine thinking and perform the best move found until now\nRight click in the chessboard has the same effect");
  butstopcheng.signal_clicked().connect(sigc::mem_fun(*this, &ChessWindowGui::on_stop_chess_engine)); //connecting the stop chessengine button signal clicked to the signal handler
  butanalyse.set_label("Analyse move");
  butanalyse.set_tooltip_text("Ask chess engine to analyse game for human player");
  butanalyse.signal_clicked().connect(sigc::mem_fun(*this, &ChessWindowGui::on_chess_engine_analyse)); //connecting the analyze chessengine button signal clicked to the signal handler
  sidebara.pack_start(butstopcheng);
  sidebara.pack_start(butanalyse);
  rightpanel.pack_start(sidebara, Gtk::PACK_SHRINK, 5);
    
  limittime.set_label("Limit CE thinking time  ");
  limittime.set_active(true);
  limittime.signal_toggled().connect(sigc::mem_fun(*this, &ChessWindowGui::on_toggled_cetime_button));
  sidebarb.pack_start(limittime, Gtk::PACK_SHRINK, 5);
  labttime.set_label("Thinking time for CE (seconds):");
  sidebarb.pack_start(labttime, Gtk::PACK_SHRINK, 5);
  spbttime.set_digits(2);
  spbttime.set_increments(0.01, 1.0);
  spbttime.set_range(3.0, 60.0);
  spbttime.set_value(5.0);
  spbttime.set_numeric(true); //to allow only numbers if the user types manually
  spbttime.signal_value_changed().connect(sigc::mem_fun(*this, &ChessWindowGui::on_set_cetime));
  sidebarb.pack_start(spbttime);
  rightpanel.pack_start(sidebarb, Gtk::PACK_SHRINK, 5);
  
  //buttons for game control
  butnew.set_label("New");
  butstart.set_label("Start");
  butback.set_label("Back");
  butforward.set_label("Forward");
  butresign.set_label("Resign");
  actionbuttons.add(butnew);
  actionbuttons.add(butstart);
  actionbuttons.add(butback);
  actionbuttons.add(butforward);
  actionbuttons.add(butresign);

  butnew.signal_clicked().connect(sigc::bind<std::string>(sigc::mem_fun(*this, &ChessWindowGui::on_action_game_new), "")); //connecting the new button signal clicked to the signal handler
  butstart.signal_clicked().connect(sigc::mem_fun(*this, &ChessWindowGui::on_action_game_start)); //connecting the start button signal clicked to the signal handler  
  butback.signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &ChessWindowGui::on_action_game_back), 1)); //connecting the back button signal clicked to the signal handler
  butforward.signal_clicked().connect(sigc::bind<int>(sigc::mem_fun(*this, &ChessWindowGui::on_action_game_forward), 1)); //connecting the forward button signal clicked to the signal handler
  butresign.signal_clicked().connect(sigc::mem_fun(*this, &ChessWindowGui::on_action_game_resign)); //connecting the resign button signal clicked to the signal handler  
  butforward.set_sensitive(false);
  butback.set_sensitive(false);
  
  actionbuttons.set_layout(Gtk::BUTTONBOX_SPREAD);
  
  sidebarc.pack_start(actionbuttons); //packing the button bar in the lowerbar
  sidebarc.pack_start(whopl, Gtk::PACK_SHRINK, 25); //packing colored frame showing who moves (the color) in the lower bar
  rightpanel.pack_start(sidebarc, Gtk::PACK_SHRINK, 5); //packing the lower bar

  //packing the textviews
  textsidearea.set_editable(false); //make the textarea read only: user cannot write directly in the Gtk::TextView
  sidecontainer.add(textsidearea);
  rightpanel.pack_start(sidecontainer);
  
  centralgroup.pack_start(rightpanel);
  mainbox.pack_start(centralgroup, Gtk::PACK_SHRINK, 5);
  
  textdownarea.set_editable(false);
  downcontainer.add(textdownarea);
  mainbox.pack_start(downcontainer);
  mainbox.set_homogeneous(false);
  
  //to show the scroll bar only when needed
  sidecontainer.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  downcontainer.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  
  show_all_children();
}

//use this constructor to connect the keyboard action
ChessWindowGui::ChessWindowGui(const Glib::RefPtr<Gtk::Application>& app) : ChessWindowGui() {
  app->set_accel_for_action("chess.new", "<Primary>n");
  app->set_accel_for_action("chess.load", "<Primary>l");
  app->set_accel_for_action("chess.readfen", "<Primary>d");
  app->set_accel_for_action("chess.save", "<Primary>s");
  app->set_accel_for_action("chess.close", "<Primary>c");
  app->set_accel_for_action("chess.quit", "<Primary>q");
  app->set_accel_for_action("chess.preferences", "<Primary>p");

  app->set_accel_for_action("chess.start", "<Primary>e");
  app->set_accel_for_action("chess.back", "<Primary>k");
  app->set_accel_for_action("chess.backall", "<Primary>z");
  app->set_accel_for_action("chess.forward", "<Primary>f");
  app->set_accel_for_action("chess.forwardall", "<Primary>x");
  app->set_accel_for_action("chess.resign", "<Primary>r");
  
  app->set_accel_for_action("chess.prefcewh", "<Primary>w");
  app->set_accel_for_action("chess.prefcebl", "<Primary>b");
  app->set_accel_for_action("chess.stopchen", "<Primary>o");
  app->set_accel_for_action("chess.analysechen", "<Primary>a");

  app->set_accel_for_action("chess.historysaveshort", "<Primary>i");
  app->set_accel_for_action("chess.historysavelong", "<Primary>j");
  app->set_accel_for_action("chess.historyshowshort", "<Primary>t");
  app->set_accel_for_action("chess.historyshowlong", "<Primary>u");
  app->set_accel_for_action("chess.fennot", "<Primary>y");
  
  app->set_accel_for_action("chess.about", "<Primary>m");
  app->set_accel_for_action("chess.help", "<Primary>h");
}

ChessWindowGui::~ChessWindowGui() {
  cnfgf->writestarthum(starthumpl); //saving last human player
  delete pgameboard;
  delete cnfgf;
  if (ucianalys != nullptr) {delete ucianalys;}
}

//menu signal handler new
void ChessWindowGui::on_action_game_new(std::string inifen) {
  bool createnew = true;
  
  if (pgameboard != nullptr) {
    std::string question = "The current game will be lost. Are you sure do you want to create a new game?\n";
    ChessAskyn dial = ChessAskyn(this, "New game", question);
    dial.run();
    createnew = dial.getanswer(); 
  }
  
  if (createnew) {
    on_action_game_close();
    
    //selecting who is human and who is chess engine
    std::array<bool, 2> ishuopt = cnfgf->getifhumanbool(starthumpl);
    if (cnfgf->getifhuman() == alte) {starthumpl = ! starthumpl;}

    ChessPlayerGui* whiteplayer = new ChessPlayerGui(white, ishuopt[0]);
    ChessPlayerGui* blackplayer = new ChessPlayerGui(black, ishuopt[1]);
    
    ChessUCIGui* whcheng = new ChessUCIGui(cnfgf->getchengname(0), cnfgf->getchengpath(0));
    ChessUCIGui* blcheng = new ChessUCIGui(cnfgf->getchengname(1), cnfgf->getchengpath(1));
    ucianalys = new ChessUCIGui(cnfgf->getchengname(2), cnfgf->getchengpath(2));
    whcheng->setpondering(cnfgf->getponder());
    blcheng->setpondering(cnfgf->getponder());
    
    //connecting signals to allow direct reception of chess engines messages
    whcheng->send_cemessage().connect(sigc::mem_fun(*this, &ChessWindowGui::displaycemess));
    blcheng->send_cemessage().connect(sigc::mem_fun(*this, &ChessWindowGui::displaycemess));
    
    pgameboard = new ChessBoardGui(this, whiteplayer, blackplayer, whcheng, blcheng, inifen);

    if (inifen.size() > 0) {//telling chess engines to use FEN when setting the position
      whcheng->setusefen(true);
      blcheng->setusefen(true);
      ucianalys->setusefen(true);
    }

    //initializing the timers
    whtimer.init(this, white, cnfgf->getgametime());
    bltimer.init(this, black, cnfgf->getgametime());

    doprintmess("Press Start button or select Start from the Actions menu to begin the game\n", false);
  }
}

//menu signal handler load
void ChessWindowGui::on_action_game_load() {
  std::string filename;
  
  //creating a filechooserdialog object
  Gtk::FileChooserDialog chosefile("Load a file", Gtk::FILE_CHOOSER_ACTION_OPEN);
  chosefile.set_transient_for(*this);
  
  chosefile.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
  chosefile.add_button("_Open", Gtk::RESPONSE_OK);
  
  auto filter_text = Gtk::FileFilter::create();
  //filter_text->set_name("Plain Text");
  //filter_text->add_mime_type("text/plain");
  filter_text->set_name("PGN notation for Chess");
  filter_text->add_mime_type("application/x-chess-pgn");
  chosefile.add_filter(filter_text);

  //show dialog and get response
  int choice = chosefile.run();
  
  //handle response
  if (choice == Gtk::RESPONSE_OK) {
    filename = chosefile.get_filename();
    
    //searching for fen argument in the header
    ChessPGNGui pgnf(filename, 'r');
    unsigned int gamepos = pgnf.selectgame();

    if (! pgnf.isgameok()) {return;} //if the dialog widget is closed before making a choice, the method ends here

    std::string initfen = pgnf.readfield(gamepos, "FEN");
    on_action_game_new(initfen); //@@@may be changed, parameters are not those of config file but those of the pgn file

    ChessPGN::pgnmoves cmoves = pgnf.readmoves(gamepos);
    bool loadok = pgameboard->wrploadgame(cmoves);
    
    if (! loadok) {
      std::string eml = "Error with loading file. Are you sure it is a valid file?";
      Gtk::MessageDialog mess(*this, "Error");
      mess.set_secondary_text(eml);
      mess.run();
      
      on_action_game_close();
    } else {
      pgameboard->printcb();
      ChessSquareGui::allowclick = true;
      pgameboard->turnation();
      setbaftermove();
    }
  }
  chosefile.hide();
}

//menu signal handler read from FEN notation
void ChessWindowGui::on_action_game_readfen() {
  ChessgetFEN dial = ChessgetFEN(this);
  dial.run();
  std::string fenstr;
  
  if (dial.getanswer()) {
    fenstr = dial.getfenrep();
    on_action_game_new(fenstr);
  }
}

//menu signal handler save
void ChessWindowGui::on_action_game_save() {
  if (pgameboard != nullptr) {
    std::string filename;
    
    //creating a filechooserdialog object
    Gtk::FileChooserDialog chosefile("Choose a file", Gtk::FILE_CHOOSER_ACTION_SAVE);
    chosefile.set_transient_for(*this);
    
    chosefile.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    chosefile.add_button("_Save", Gtk::RESPONSE_OK);
    
    auto filter_text = Gtk::FileFilter::create();
    //filter_text->set_name("Plain Text");
    //filter_text->add_mime_type("text/plain");
    filter_text->set_name("Notazione partita a scacchi PGN");
    filter_text->add_mime_type("application/x-chess-pgn");
    chosefile.add_filter(filter_text);

    //show dialog and get response
    int choice = chosefile.run();
    
    //handle response
    if (choice == Gtk::RESPONSE_OK) {
      filename = chosefile.get_filename();
      
      //checking if file already exists
      std::ifstream test(filename.c_str());
      if (test.good()) {
        //asking if player wants to overwrite
        std::stringstream message;
        size_t slashpos = filename.find_last_of('/');
        std::string shfn = filename.substr(slashpos+1);
        message << "Do you really want to overwrite " << shfn << "?";
        ChessAskyn adialog = ChessAskyn(this, "Confirmation", message.str());
        adialog.run();
      
        if (adialog.getanswer()) {pgameboard->guisavegame(filename);}
      } else {pgameboard->guisavegame(filename);}
    }
    
    chosefile.hide();
  }
}

//menu signal handler close
void ChessWindowGui::on_action_game_close() {
  if (pgameboard != nullptr) {
    whopl.set_name(""); //deassociating from a css style giving the background color, to have the square empty when there is no game
    delete ucianalys; delete pgameboard;
    ucianalys = nullptr; pgameboard = nullptr;
    clearboard();
    
    //stop timers
    whtimer.deltimer();
    bltimer.deltimer();

    ChessSquareGui::allowclick = false; //disabilitate the click for pieces
    
    //cleaning the textarea widgets
    Gtk::TextIter bufbegin = sbuffer->begin();
    Gtk::TextIter bufend = sbuffer->end();
    sbuffer->erase(bufbegin, bufend);
    txtspos = sbuffer->end();
    
    bufbegin = dbuffer->begin();
    bufend = dbuffer->end();
    dbuffer->erase(bufbegin, bufend);
    txtdpos = dbuffer->end();
  }
}

//menu signal handler quit
void ChessWindowGui::on_action_game_quit() {
  on_action_game_close();
  hide();
}

//menu signal handler preferences
void ChessWindowGui::on_action_game_preferences() {
  ChessPreferGui prefdialog = ChessPreferGui(this, cnfgf);
  prefdialog.run();
}

//menu signal handler back
void ChessWindowGui::on_action_game_back(int n) {
  if (pgameboard != nullptr) {
    if (! atwork) {
      atwork = true;
      bool act = pgameboard->goback(n);
      pgameboard->printcb();
      showwhom(pgameboard->player_moving->wpcolor());
      
      //setting sensitivity of back / forward buttons
      if (! act) {butback.set_sensitive(false);}
      if (! butforward.get_sensitive()) {butforward.set_sensitive(true);}
      
      atwork = false;
    }
  }
}

//menu signal handler forward
void ChessWindowGui::on_action_game_forward(int n) {
  if (pgameboard != nullptr) {
    if (! atwork) {
      atwork = true;
      bool act = pgameboard->goforward(n);
      pgameboard->printcb();
      showwhom(pgameboard->player_moving->wpcolor());
      
      //setting sensitivity of back / forward buttons
      if (! act) {butforward.set_sensitive(false);}
      if (! butback.get_sensitive()) {butback.set_sensitive(true);}
      
      atwork = false;
    }
  }
}

//menu signal handler resign
void ChessWindowGui::on_action_game_resign() {
  if (pgameboard != nullptr) {
    if (! atwork) {
      atwork = true;
    
      //asking if player is sure
      std::string message = pgameboard->player_moving->whoplay() + ", do you really want to resign?\nIf you confirm, the game will be closed.";
      ChessAskyn dialog = ChessAskyn(this, "Resign menu", message);
      dialog.run();
      
      if (dialog.getanswer()) {
        pgameboard->resignmess();
        on_action_game_close();
      }
      atwork = false;
    }
  }
}

//menu signal handler start game
void ChessWindowGui::on_action_game_start() {
  if (pgameboard != nullptr) {
    if (! ChessSquareGui::allowclick) {
      whtimer.stopgotimer(); //to let the white timer to start
      on_toggled_cetime_button(); //to set the thinking time now (we are sure that nullptr is not null)
      pgameboard->turnation();
      ChessSquareGui::allowclick = true;
    }
  }
}

//menu signal hanlder set chess engine option
void ChessWindowGui::on_action_setceoptions(c_color cc) {
  if (pgameboard != nullptr) {
    pgameboard->setceoptions(cc);
  } else {
    std::string mess = "To see and set the chess engine options, the chess engine must first be started.\nIt is done when you start a new game.";
    Gtk::MessageDialog dial(*this, "Warning!");
    dial.set_secondary_text(mess);
    dial.run();
  }
}

//signal handler for toggled cetime checkbutton
void ChessWindowGui::on_toggled_cetime_button() {
  if (pgameboard != nullptr) {
    if (limittime.get_active()) {on_set_cetime();}
    else {pgameboard->set_cetime(0.0);}
  }
}

//signal handler to call ChessBoard set_cetime
void ChessWindowGui::on_set_cetime() {
  if (pgameboard != nullptr && limittime.get_active()) {
    double thtime = spbttime.get_value();
    pgameboard->set_cetime(thtime);
  }
}

//menu signal handler stop chess engine
void ChessWindowGui::on_stop_chess_engine() {
  if (pgameboard != nullptr) {
    pgameboard->engine_stop();
  }
}

//menu signal handler chess engine analyse
void ChessWindowGui::on_chess_engine_analyse() {
  if (pgameboard != nullptr) {
    std::string cepath = cnfgf->getchengname(2); //used only to check if a set engine for analysis is set or not
    if (cepath == "") {
      Gtk::MessageDialog popm(*this, "Error, no chess engine is set for analysis!");
      popm.run();
      
    } else {
      bool isok = false;
      if (ucianalys->isactive()) {isok = true;}
      else {
        ucianalys->init();
        ucianalys->sendcomm(0); //sending uci
        
        int cc = 0;
        while ((! ucianalys->checkanswok(1)) && cc < 100) {//checking for uciok
          sleep(0.1);
          ucianalys->geteansw(true);
          cc++;
        }
  
        if (cc == 100) {isok = false;} else {isok = true;}
      }

      if (isok) {
        if (ucianalys->getusefen()) {
          std::string fenrep = pgameboard->genFEN();
          ucianalys->setfenchb(fenrep);
        } else {
          std::string chist = pgameboard->wrpgethistory();
          std::string cchist = ucianalys->formatprevmoves(chist);
          ucianalys->setfixedhist(cchist);
        }
        
        ChessAnalysisGui analysdial(this, ucianalys);
        analysdial.run();
      }
    }
  }
}

//menu signal handler history
void ChessWindowGui::on_action_game_history_save(bool lnot) {
  if (pgameboard != nullptr) {
    std::string filename;
    
    //creating a filechooserdialog object
    Gtk::FileChooserDialog chosefile("Choose a file", Gtk::FILE_CHOOSER_ACTION_SAVE);
    chosefile.set_transient_for(*this);
    
    chosefile.set_do_overwrite_confirmation(true);
    chosefile.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    chosefile.add_button("_Open", Gtk::RESPONSE_OK);
    
    auto filter_text = Gtk::FileFilter::create();
    filter_text->set_name("Plain Text");
    filter_text->add_mime_type("text/plain");
    chosefile.add_filter(filter_text);

    //show dialog and get response
    int choice = chosefile.run();
    
    //handle response
    if (choice == Gtk::RESPONSE_OK) {
      filename = chosefile.get_filename();
      pgameboard->printhist(filename, lnot);
    }
  }
}

//menu signal handler history
void ChessWindowGui::on_action_game_history_show(bool lnot) {
  if (pgameboard != nullptr) {
    std::string histlabel, sectxt;
    
    histlabel = pgameboard->printhist("", lnot);
    if (histlabel.size() == 0) {sectxt = "No move has been done yet.";}
    else {sectxt = histlabel;}
    
    Gtk::MessageDialog histpopup(*this, "History of the game");
    histpopup.set_secondary_text(sectxt);
    
    histpopup.run();
  }
}

//generate FEN string of the current status of the game
void ChessWindowGui::on_action_gen_fen() {
  if (pgameboard != nullptr) {
    std::string fenstr = pgameboard->genFEN();
    Gtk::MessageDialog fenpop(*this, "FEN notation");
    fenpop.set_secondary_text(fenstr);
    fenpop.run();
  }
}

//print about
void ChessWindowGui::on_action_printabout() {
  std::stringstream aboutmess;
  
  aboutmess << "Yagchess  Copyright (C) 2017  Valentino Esposito\n";
  aboutmess << "This program comes with ABSOLUTELY NO WARRANTY; ";
  aboutmess << "This is free software, and you are welcome to redistribute it ";
  aboutmess << "under certain conditions. See the GNU Public License for more details.";
  
  Gtk::MessageDialog aboutpopup(*this, "About Yagchess");
  aboutpopup.set_secondary_text(aboutmess.str());
  
  aboutpopup.run();
}

//print help
void ChessWindowGui::on_action_printhelp() {
  std::stringstream helpmess;

  helpmess << "The graphic interface should be quite intuitive: ";
  helpmess << "to play, simply left click on the piece you want to move, and then left click on the square you want to put it / piece you want to eat. ";
  helpmess << "if the move is valid, it will be carried out.\n";
  helpmess << "Right click can be used to show a popup menu with possibly useful tools for beginner players.\n";
  helpmess << "\nIf you are playing against a chess engine, right click can be used to stop the chess engine calculation and let it to move using the bestmove it found. ";
  helpmess << "This is mandatory in case no time limit has been fixed for the chess engine: it will think infinitely until it is not told to stop.\n";
  helpmess << "\n\"Start\" button must be pressed to start the game. If you play against a chess engine, please set any engine option before pressing Start. ";
  helpmess << "You can use the \"Back\" and \"Forward\" buttons or the equivalent menu actions to move along the history of the game, ";
  helpmess << "in case you want to change any move. \"Back All\" and \"Forward All\" go directly to the beginning and last played move of the game. ";
  helpmess << "\"Resign\" button can be used to surrend.\n";
  helpmess << "\nFrom \"Preferences\" in the \"Game\" menu, you can assign a chess engine to white or black, set the time game, and change other options.\n";
  helpmess << "From \"Chess Engine Preferences\" you can get the customizable options of the selected chess engine. This allows you to set the internal parameters ";
  helpmess << "of the chess engine, as (if supported) the engine skill. The actual options can vary from engine to engine, ";
  helpmess << "so check the reference of the chess engine you are using to understand them. Default values are used if the options are not manually set.\n";
  helpmess << "\nFor more detailed informations, read the manual. A copy of the manual should be included in the distribution.\n";
  
  Gtk::MessageDialog helpopup(*this, "Yagchess Help");
  helpopup.set_secondary_text(helpmess.str());
  
  helpopup.run();
}

//change che color of the square to show the player who moves
void ChessWindowGui::showwhom(c_color cc) {whopl.setcolor(cc);}

//stop a timer, start the other one, to be called at the end of the turn
void ChessWindowGui::presstimer() {
  whtimer.stopgotimer();
  bltimer.stopgotimer();
}

//setting the back / forward buttons after a move
void ChessWindowGui::setbaftermove() {
  if (! butback.get_sensitive()) {butback.set_sensitive(true);}
  if (butforward.get_sensitive()) {butforward.set_sensitive(false);}
}

//get time on the clocks
std::array<int, 2> ChessWindowGui::gettimers() {
  std::array<int, 2> res;
  res[0] = whtimer.gettime();
  res[1] = bltimer.gettime();
  return res;
}

//signal handler to claim the end of game
void ChessWindowGui::on_game_end(c_color cp, bool iswinner, bool winontime) {
  //stop timers
  whtimer.deltimer();
  bltimer.deltimer();

  std::string winner, loser, popmess;
 
  if (! iswinner) {popmess = "Game tie!";}
  else {
    if (cp == white) {winner = "Black"; loser = "White";}
    else if (cp == black) {winner = "White"; loser = "Black";}
    
    if (winontime) {popmess = loser + " has no more time to play! " + winner + " wins!";}
    else {popmess = "Checkmate! " + winner + " wins!";}
  }
  
  doprintmess(popmess, true);
}

//called by ChessSquareGui through the pointer to print the message in the textarea
void ChessWindowGui::doprintmess(std::string txtmess, bool popup) {
  txtspos = sbuffer->insert(txtspos, txtmess); //insert the string in the stream to the position. The position is updated at the end of the added text (return of the the insert function)
  textsidearea.scroll_to(txtspos);
    
  if (popup) {
    Gtk::MessageDialog winpopup(*this, txtmess);
    winpopup.run();
  }
}

//display messages of the chess engine in the dedicated textarea
void ChessWindowGui::displaycemess(std::string cemess) {
  txtdpos = dbuffer->insert(txtdpos, cemess); //insert the string in the stream to the position.
  textdownarea.scroll_to(txtdpos);
}

//remove all the icons of the pieces from the chessboard
void ChessWindowGui::clearboard(void) {
  ChessSquareGui* ps;
  Piece* nullp = nullptr;
  
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      ps = getsquaregui(i, j);
      ps->showpiece(nullp); //a trick to empty the square
      ps->deassociate_from_board(); //removing the association with the ChessBoardGui object (setting pointer to null)
    }
  }
}


/* Implementing the virtual functions of the chessboard for the graphical game
 * Methods of ChessBoardGui
 */
ChessBoardGui::ChessBoardGui(std::string fenpos) : ChessBoard(fenpos) {}

ChessBoardGui::ChessBoardGui(ChessWindowGui* parent, ChessPlayerGui* ppa, ChessPlayerGui* ppb, ChessUCIGui* wce, ChessUCIGui* bce, std::string fenpos) : ChessBoardGui(fenpos) {
  pwindow = parent;
  ChessSquareGui* sq;

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      sq = pwindow->getsquaregui(i, j);
      sq->setpointer(this);
    }
  }

  gather_players(ppa, ppb);
  set_board_for_players();
  
  bool eok = check_engine(wce, bce);
  if (! eok) {
    Gtk::MessageDialog erm("Something wrong when starting a chess engine, maybe is a path not correct?");
    erm.run();
    return;
  }

  send_gameend_signal().connect(sigc::mem_fun(*pwindow, &ChessWindowGui::on_game_end));
  
  saver->autosavegame(*this, true);
  printcb();
}

ChessBoardGui::~ChessBoardGui() {
  for (unsigned int j = 0; j < players.size(); j++) {delete players[j];}
}

//wrapper for check_stargting, only used to print a message in the textarea and deselect squares indicating a move
bool ChessBoardGui::check_startinggui(c_color cc, ChessSquare* firstsel) {
  bool res = check_starting(cc, firstsel);
  if (!res) {
    unselectallsq();
    cbbuf << "Nothing that you can move here. Please select another square.";
    printmess();
  }
  return res;
}

//filling the players (overriding non virtual function)
void ChessBoardGui::gather_players(ChessPlayerGui* pa, ChessPlayerGui* pb) {
  players[0] = pa;
  players[1] = pb;
  player_moving = players[0];
}

//public wrapper to use the saver->savegame method
void ChessBoardGui::guisavegame(std::string sfn) {

  ChessPlayerGui* player_moving_gui;
  player_moving_gui = dynamic_cast<ChessPlayerGui*>(player_moving);
  player_moving_gui->wrappersetsavefn(sfn);

  saver->savegamepgn(*this, player_moving_gui->getsavefn(), pwindow->getconfclass());
}

//apply selection flag to an ensemble of squares and show it
void ChessBoardGui::selectguisquares(std::vector<ChessSquare*> tbsvector) {
  ChessSquare* tbsel;
  ChessSquareGui* guitbs;
  
  for (unsigned int i = 0; i < tbsvector.size(); i++) {
    tbsel = tbsvector[i];
    guitbs = pwindow->getsquaregui(tbsel->getx(), tbsel->gety());
    guitbs->applyselect(true);
  }
}

//unselect all squares
void ChessBoardGui::unselectallsq() {
  ChessSquareGui* ps;
  
  for (int i = CHVector::min_x; i < CHVector::max_x; i++) {
    for (int j = CHVector::min_y; j < CHVector::max_y; j++) {
      ps = pwindow->getsquaregui(i, j);

      ps->selected = false;
      ps->applyselect(false);
    }
  }
}

//calling the ChessUCIGui dialog to set options of the chess engine, here we use an integer and not the c_color enumeration because we had to handle also the analyser
void ChessBoardGui::setceoptions(c_color who) {
  ChessPlayerGui* pl;
  ChessUCIGui* ucidi;
  std::string dtitle;
  
  if (who == white) {
    pl = dynamic_cast<ChessPlayerGui*>(players[0]);
    ucidi = dynamic_cast<ChessUCIGui*>(uciwh);
    dtitle = "White options";
  } else if (who == black) {
    pl = dynamic_cast<ChessPlayerGui*>(players[1]);
    ucidi = dynamic_cast<ChessUCIGui*>(ucibl);
    dtitle = "Black options";
  }

  if (pl->isplayerhuman()) {
    std::string mess = pl->whoplay() + " is human!";
    Gtk::MessageDialog dlg(mess);
    dlg.run();
  } else {ucidi->set_options(dtitle);}
}

//handling a turnation, actually sending only the messages before the move
bool ChessBoardGui::turnation() {
  bool iscm, isd, gameoff;
  std::string txtcolmove;

  //verifying checkmate
  iscm = ischeckmate(player_moving);
  if (iscm) {isawinner = true;}

  //verifying draw condition
  isd = isdraw(player_moving);
  if (isd) {isawinner = false;}
  
  gameoff = (iscm || isd);

  if (! gameoff) {
    cbbuf << "Current turn: " << turn << ". No eatings or pawn movements since " << drawffcounter << " moves.";
    printmess();
    
    cbbuf << player_moving->whoplay() << " moves.";
    printmess();
    
    //show the color of the player who moves in the dedicated square 
    pwindow->showwhom(player_moving->wpcolor());

    //call for engine_act if the player is the engine
    if (! player_moving->isplayerhuman()) {
      ChessPlayerGui* player_moving_gui;
      player_moving_gui = dynamic_cast<ChessPlayerGui*>(player_moving);
      std::array<int, 2> tclocks = pwindow->gettimers(); 
      player_moving_gui->ucieng->get_lefttimes(tclocks[0], tclocks[1]);
      engine_act(player_moving_gui, player_moving_gui->ucieng);
      player_moving_gui->send_status_signal()(true);
      engine_act(player_moving_gui, player_moving_gui->ucieng, true); //call it again just to launch the ponder command
      
      player_moving_gui->move_from = nullptr;
      player_moving_gui->move_to = nullptr;
    }
  }
  
  return gameoff;
}

//displaying text messages in the dedicated area
void ChessBoardGui::printmess(bool specialmess) {
  cbbuf << std::endl;
  std::string copymess = cbbuf.str();
  pwindow->doprintmess(cbbuf.str(), specialmess);
  cbbuf.str("");
}

//concluding the turn
void ChessBoardGui::turnation_end(bool domove) {
  if (domove) {
    //action after the ending of the move is set, complete the move and start the next turnation or end the game
    if (player_moving->isplayerhuman()) {
      cbbuf << "Moving to: " << ChessCoordinates::xout(player_moving->sel_x) << " " << ChessCoordinates::yout(player_moving->sel_y);
      printmess();
    }
    
    bool editgame = true;
    if (saver->clearfuture(true)) {
      std::string question = "You are changing previous moves of the game. Are you sure?\n";
      ChessAskyn dial = ChessAskyn(pwindow, "Confirmation", question);
      dial.run();
      editgame = dial.getanswer(); 
    }

    unselectallsq();
    if (editgame) {
      //apply the move
      bool okm = chessmove(player_moving);
      
      if (okm) {
        printcb();
        
        pwindow->presstimer(); //switching timer
        pwindow->setbaftermove(); //setting activity of back/forward buttons
        
        drawffcounter++;
        saver->autosavegame(*this, false); //this should be done here, after the increment of drawffcounter and before that of turn
        if (player_moving->wpcolor() == white) {
          player_moving = players[1];
          saver->turnlost = false;
        } else if (player_moving->wpcolor() == black) {player_moving = players[0];
          turn++;
          saver->turnlost = true;
        }
      }

      bool stopgame = turnation();
      if (stopgame) {
        send_gameend_signal()(player_moving->wpcolor(), isawinner, false);
      }
    }
  } else {
    //action after the starting of the move is set
    if (player_moving->isplayerhuman()) {
      cbbuf << "Moving from: " << ChessCoordinates::xout(player_moving->sel_x) << " " << ChessCoordinates::yout(player_moving->sel_y);
      printmess();
    }
    
    //retrieving squares to be selected considering moves, eatings and special moves when applicable, this is to show where the piece can move
    unselectallsq(); //to clear selection of piece clicked and not moved in case of wrong click
    Piece* movp = player_moving->move_from->p;
    
    std::vector<ChessSquare*> sqtsel;
    sqtsel = exploremoveats(movp);
    
    selectguisquares(sqtsel);
  }
}

//display pieces in the chessboard at their current position.
void ChessBoardGui::printcb() {
  Piece* pc;
  ChessSquareGui* ps;
  
  for (int i = CHVector::min_x; i < CHVector::max_x; i++) {
    for (int j = CHVector::min_y; j < CHVector::max_y; j++) {
      ps = pwindow->getsquaregui(i, j);
      pc = getsquare(i, j)->p;

      ps->showpiece(pc);
    }
  }
  
  while (Gtk::Main::events_pending()) {Gtk::Main::iteration();} //to let gtkmm to update the graphical interface
}

//allow the player to choose the promoted piece
wpiece ChessBoardGui::choose_promotion(c_color cc) {
  
  ChessPromotionGui promwindow = ChessPromotionGui(pwindow, cc);
  promwindow.run();
  
  //works because is a dialog: the run method freeze the execution of the method until the "close" method of the dialog is called from the callback, but the ChessPromotionGui object is not destroyed until the scope is terminated
  return promwindow.pselected;
}

//ask for drawing
bool ChessBoardGui::askdraw(int m) {
  bool res;
  c_color cc = player_moving->wpcolor();
  std::string pcolt;
  if (cc == white) {pcolt = "White";}
  else if (cc == black) {pcolt = "Black";}
  
  std::stringstream message;
  if (m == 0) {message << "50 moves have been done without eating or moving a pawn.\n";}
  else if (m == 1) {message << "Threefold repetition: an identical position has occurred at least three times during the course of the game.\n";}
  else if (m == 2) {message << pcolt << " is in Stalemate. Game draw.\n";}
  else if (m == 3) {message << "Only the kings are on the chessboard. Game draw.\n";}
  else {std::cerr << "Something wrong with valid options for askdraw(). The value " << m << " was received, but 0, 1 or 2 are the only valid options." << std::endl;}

  if (m == 0 || m == 1) {
    message << "Do you want to end the game in a tie?";
    ChessAskyn ppw = ChessAskyn(pwindow, "Draw game", message.str());
    ppw.run();
    //works because is a dialog
    res = ppw.getanswer();
    
  } else if (m == 2 || m == 3) {
    Gtk::MessageDialog mpw(*pwindow, "Draw game");
    mpw.set_secondary_text(message.str());
    mpw.run();
    res = true;
  }

  return res;
}

//print history of the game in algebric notation
std::string ChessBoardGui::printhist(std::string hfn, bool lnot) {
  std::string fullhist = "";

  if (hfn.size() > 0) {
    ChessPlayerGui* player_moving_gui;
    player_moving_gui = dynamic_cast<ChessPlayerGui*>(player_moving);
    player_moving_gui->wrappersethistoryfn(hfn);
    saver->writefhist(player_moving_gui->gethistoryfn(), lnot);
  } else {fullhist = saver->gethistory(true, lnot);}
  
  return fullhist;
}

//to be called inside any method (tipically from inside a loop) when needed to check regurlarly if other GUI stuffs need to be executed
void ChessBoardGui::letgenupdate() {
  while (Gtk::Main::events_pending()) {Gtk::Main::iteration();} //to let gtkmm to do other stuffs
}


/* Methods of ChessPromotionGui
 */
ChessPromotionGui::ChessPromotionGui(ChessWindowGui* windowparent, c_color cc) {
  set_transient_for(*windowparent);
  //set_deletable(false); should prevent to show the close button, but it does not work because (on linux) it may not work, depending on the window manager
  set_decorated(false); //this works, title bar and all stuffs are removed completely
  
  set_title("Pawn Promotion");
  set_default_size(200, 80);
  set_border_width(15);
  
  explain.set_text("Click to select the piece.");
  mainbox->add(explain);
  
  std::string coltxt;
  std::string base = baserespath + "chess_pieces_icons/";
  
  if (cc == white) {coltxt = "White.png";}
  else if (cc == black) {coltxt = "Black.png";}
  
  //getting the images of the buttons
  rockim.set(base + "Rock" + coltxt);
  knightim.set(base + "Knight" + coltxt);
  bishopim.set(base + "Bishop" + coltxt);
  queenim.set(base + "Queen" + coltxt);
  
  //assigning the images to the buttons 
  rockb.set_image(rockim);
  knightb.set_image(knightim);
  bishopb.set_image(bishopim);
  queenb.set_image(queenim);

  //binding the signal handler with the appropriate parameter
  rockb.signal_clicked().connect(sigc::bind<wpiece>(sigc::mem_fun(*this, &ChessPromotionGui::on_choice_clicked), rock));
  knightb.signal_clicked().connect(sigc::bind<wpiece>(sigc::mem_fun(*this, &ChessPromotionGui::on_choice_clicked), knight));
  bishopb.signal_clicked().connect(sigc::bind<wpiece>(sigc::mem_fun(*this, &ChessPromotionGui::on_choice_clicked), bishop));
  queenb.signal_clicked().connect(sigc::bind<wpiece>(sigc::mem_fun(*this, &ChessPromotionGui::on_choice_clicked), queen));
    
  //adding the button to the box to be shown
  selpiece.add(rockb);
  selpiece.add(knightb);
  selpiece.add(bishopb);
  selpiece.add(queenb);
  
  mainbox->add(selpiece);
  
  show_all_children();
}

ChessPromotionGui::~ChessPromotionGui() {}

//signal handler, it is connected to more than a signal button and binded each time with a different parameter 
void ChessPromotionGui::on_choice_clicked(wpiece wp) {
  pselected = wp;
  close();
}


/* Methods of ChessAskyn
 */
ChessAskyn::ChessAskyn(Gtk::Window* windowparent, std::string mtitle, std::string mtext) {
  set_transient_for(*windowparent);
  
  set_title(mtitle);
  set_default_size(200, 80);
  set_border_width(15);
  
  explain.set_text(mtext);
  mainbox->add(explain);
  
  yesbutton.set_label("Yes");
  nobutton.set_label("No");
  
  yesbutton.signal_clicked().connect(sigc::bind<bool>(sigc::mem_fun(*this, &ChessAskyn::on_yesno_clicked), true));
  nobutton.signal_clicked().connect(sigc::bind<bool>(sigc::mem_fun(*this, &ChessAskyn::on_yesno_clicked), false));
  
  getansw.add(yesbutton);
  getansw.add(nobutton);
  
  mainbox->add(getansw);
  
  show_all_children();
}

ChessAskyn::~ChessAskyn() {}

//signal handler, it is connected to two button and binded each time with a different parameter
void ChessAskyn::on_yesno_clicked(bool res) {
  answer = res;
  close();
}

/* Methods of class ChessgetFEN
 */
ChessgetFEN::ChessgetFEN(Gtk::Window* windowparent) {
  set_transient_for(*windowparent);
  set_title("Build from FEN notation");
  set_default_size(350, 100);
  set_border_width(15);
  
  //the description label
  descr.set_text("Give a full FEN notation to place the pieces");
  mainbox->pack_start(descr, Gtk::PACK_SHRINK);
  
  //the entry
  mainbox->pack_start(fenent, Gtk::PACK_SHRINK);
  
  //the buttons
  cancel.set_label("Cancel");
  confirm.set_label("OK");
  cancel.signal_clicked().connect(sigc::bind<bool>(sigc::mem_fun(*this, &ChessgetFEN::on_button_clicked), false));
  confirm.signal_clicked().connect(sigc::bind<bool>(sigc::mem_fun(*this, &ChessgetFEN::on_button_clicked), true));
  buttons.add(cancel);
  buttons.add(confirm);
  mainbox->pack_start(buttons, Gtk::PACK_SHRINK);
  
  show_all_children();
}

ChessgetFEN::~ChessgetFEN() {}

//signal handler, connected to two buttons and binded
void ChessgetFEN::on_button_clicked(bool res) {
  answer = res;
  if (answer) {fenrep = fenent.get_text();}
  close();
}

/* Methods of ChessPreferGui
 */
ChessPreferGui::ChessPreferGui(ChessWindowGui* windowparent, ChessConfig* cm) {
  set_transient_for(*windowparent);
  confmanager = cm;
  
  set_title("Preferences");
  set_default_size(250, 450);
  set_border_width(15);
  
  //creating the view to see the current chess engine list
  topl.set_text("Chess Engine List");
  cetablels = Gtk::ListStore::create(cetablemodel);
  cetabletv.set_model(cetablels);
  cetabletv.signal_button_press_event().connect_notify(sigc::mem_fun(*this, &ChessPreferGui::on_click_in_treeview));
  
  scwinfortable.add(cetabletv);
  scwinfortable.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC); //to show the bar only when is needed  

  populatetreeview();
  cetabletv.append_column("Name", cetablemodel.col_name);
  cetabletv.append_column("Path", cetablemodel.col_path);
  cetabletv.append_column("Used by", cetablemodel.col_usedby);  

  //creating the content of the popup menu to manipuate the chess engine list
  auto item = Gtk::manage(new Gtk::MenuItem("_Set for White", true));
  item->signal_activate().connect(sigc::bind<unsigned int>(sigc::mem_fun(*this, &ChessPreferGui::on_popup_menu_setfor), 0));
  popmenu.append(*item);

  item = Gtk::manage(new Gtk::MenuItem("_Set for Black", true));
  item->signal_activate().connect(sigc::bind<unsigned int>(sigc::mem_fun(*this, &ChessPreferGui::on_popup_menu_setfor), 1));
  popmenu.append(*item);

  item = Gtk::manage(new Gtk::MenuItem("_Set for Analyser", true));
  item->signal_activate().connect(sigc::bind<unsigned int>(sigc::mem_fun(*this, &ChessPreferGui::on_popup_menu_setfor), 2));
  popmenu.append(*item);
  
  item = Gtk::manage(new Gtk::MenuItem("_Remove", true));
  item->signal_activate().connect(sigc::mem_fun(*this, &ChessPreferGui::on_popup_menu_remove));
  popmenu.append(*item);
  
  item = Gtk::manage(new Gtk::MenuItem("_Options", true));
  item->signal_activate().connect(sigc::mem_fun(*this, &ChessPreferGui::on_popup_menu_options));
  popmenu.append(*item);
  
  popmenu.accelerate(*this); //ChessPreferGui inherits by Gtk::Dialog which inherit by Gtk::Window (requested by accelerate), so it works
  popmenu.show_all(); //Show all menu items when the menu pops up

  //creating the interface to add new engine to the list
  tlab_a.set_text("New chess engine name");
  tlab_b.set_text("New chess engine path");
  ceaddb.set_label("Add to the list");
  ceaddb.signal_clicked().connect(sigc::mem_fun(*this, &ChessPreferGui::on_add_button_clicked));
  fipathb.set_label("Select CE");
  fipathb.signal_clicked().connect(sigc::bind<Gtk::Entry*>(sigc::mem_fun(*this, &ChessPreferGui::on_find_path_clicked), &cepathe));
  
  gridcontainer.set_border_width(10);
  gridcontainer.attach(tlab_a, 0, 0, 1, 1);
  gridcontainer.attach(tlab_b, 1, 0, 2, 1);
  gridcontainer.attach(cenamee, 0, 1, 1, 1);
  gridcontainer.attach(cepathe, 1, 1, 1, 1);
  gridcontainer.attach(fipathb, 2, 1, 1, 1);
  gridcontainer.attach(ceaddb, 0, 2, 3, 1);
  
  //creating the interface to set the game time
  timel.set_text("Timed game:");
  gttablels = Gtk::ListStore::create(gttablemodel);
  gttablecb.set_model(gttablels);
  populatecomboboxtime();
  
  gttablecb.pack_start(gttablemodel.col_name); //here we do not use append_column, as this is not a treeview but a combobox
  gttablecb.signal_changed().connect(sigc::mem_fun(*this, &ChessPreferGui::on_timecombobox_changed)); 
  
  //setting shown column and initial value for the time combobox
  gttablecb.set_entry_text_column(gttablemodel.col_name);
  bool gtset = setcbtimeval();
  if (! gtset) {std::cerr << "Warning, the game time set does not have a correspondence in the options.";}
  
  //creating the interface to set if players are humans and specify their names
  whoplayl.set_text("Set players:");
  whoplaytablels = Gtk::ListStore::create(whoplaymodel);
  whoplaytablecb.set_model(whoplaytablels);
  populatecomboboxwhoplay();
  
  whoplaytablecb.pack_start(whoplaymodel.col_name);
  whoplaytablecb.signal_changed().connect(sigc::mem_fun(*this, &ChessPreferGui::on_whoplaycombobox_changed));

  //setting shown column and initial value for the whoplay combobox
  whoplaytablecb.set_entry_text_column(whoplaymodel.col_name);
  bool whopset = setcbwhopval();
  if (! whopset) {std::cerr << "Warning, the players set do not have a correspondence in the options.";}

  //setting labels and entries for the player's name
  namel_w.set_text("White name");
  setnamesasconf(whname, 0);
  whname.signal_changed().connect(sigc::bind<unsigned int>(sigc::mem_fun(*this, &ChessPreferGui::on_entry_changed), 0));
  namel_b.set_text("Black name");
  setnamesasconf(blname, 1);
  blname.signal_changed().connect(sigc::bind<unsigned int>(sigc::mem_fun(*this, &ChessPreferGui::on_entry_changed), 1));
  
  gridcontainer.attach(timel, 0, 3, 1, 1);
  gridcontainer.attach(gttablecb, 1, 3, 2, 1);
  gridcontainer.attach(whoplayl, 0, 4, 1, 1);
  gridcontainer.attach(whoplaytablecb, 1, 4, 2, 1);
  gridcontainer.attach(namel_w, 0, 5, 1, 1);
  gridcontainer.attach(whname, 1, 5, 2, 1);
  gridcontainer.attach(namel_b, 0, 6, 1, 1);
  gridcontainer.attach(blname, 1, 6, 2, 1);

  //adding checkbox for pondering
  checkb_ponder.set_label("Ponder");
  checkb_ponder.signal_toggled().connect(sigc::mem_fun(*this, &ChessPreferGui::on_ponder_checkbutton_toggled));
  setponderasconf();
  
  //adding button to save current impostation
  savecb.set_label("Save current settings");
  savecb.signal_clicked().connect(sigc::mem_fun(*this, &ChessPreferGui::on_save_custom_clicked));

  //adding button to set everything to default
  sedefb.set_label("Set default settings");
  sedefb.signal_clicked().connect(sigc::mem_fun(*this, &ChessPreferGui::on_set_default_clicked));

  //packing all togheter
  mainbox->pack_start(topl, Gtk::PACK_SHRINK);
  mainbox->pack_start(scwinfortable);
  mainbox->pack_start(gridcontainer, Gtk::PACK_SHRINK);
  mainbox->pack_start(checkb_ponder, Gtk::PACK_SHRINK);
  mainbox->pack_start(savecb, Gtk::PACK_SHRINK);
  mainbox->pack_start(sedefb, Gtk::PACK_SHRINK);
  
  show_all_children();
}

ChessPreferGui::~ChessPreferGui() {}

//building the threeview (populating the cetablels model instance)
void ChessPreferGui::populatetreeview() {
  //cleaning the treeview model
  cetablels->clear();

  //populating the treeview  
  ChessConfig::umapstrstr umapdata = confmanager->getcemap();
  std::string whuses = confmanager->getchengname(0);
  std::string bluses = confmanager->getchengname(1);
  std::string anuses = confmanager->getchengname(2);
  
  for (auto it = umapdata.begin(); it != umapdata.end(); ++it) {
    std::string who = "";
    Gtk::TreeModel::Row row = *(cetablels->append());
    row[cetablemodel.col_name] = it->first;
    row[cetablemodel.col_path] = it->second;
        
    if (it->first == whuses) {who.append("W");}
    if (it->first == bluses) {who.append("B");}
    if (it->first == anuses) {who.append("A");}
    row[cetablemodel.col_usedby] = who;
  }
}

//building the time combobox (populating the gttablels model instance)
void ChessPreferGui::populatecomboboxtime() {
  //cleaning the combobox model
  gttablels->clear();
  
  //populating the combobox, here we chose the time values offered by the program, time values are in seconds 
  Gtk::TreeModel::Row row = *(gttablels->append());
  row[gttablemodel.col_name] = "No limit";
  row[gttablemodel.col_value] = -1;
  
  row = *(gttablels->append());
  row[gttablemodel.col_name] = "10 minutes";
  row[gttablemodel.col_value] = 600;
  
  row = *(gttablels->append());
  row[gttablemodel.col_name] = "15 minutes";
  row[gttablemodel.col_value] = 900;
  
  row = *(gttablels->append());
  row[gttablemodel.col_name] = "30 minutes";
  row[gttablemodel.col_value] = 1800;
  
  row = *(gttablels->append());
  row[gttablemodel.col_name] = "45 minutes";
  row[gttablemodel.col_value] = 2700;
  
  row = *(gttablels->append());
  row[gttablemodel.col_name] = "1 hour";
  row[gttablemodel.col_value] = 3600;
  
  row = *(gttablels->append());
  row[gttablemodel.col_name] = "90 minutes";
  row[gttablemodel.col_value] = 5400;
  
  row = *(gttablels->append());
  row[gttablemodel.col_name] = "2 hours";
  row[gttablemodel.col_value] = 7200;

  row = *(gttablels->append());
  row[gttablemodel.col_name] = "3 hours";
  row[gttablemodel.col_value] = 10800;
}

//building the time combobox (populating the whoplaytablels model instance)
void ChessPreferGui::populatecomboboxwhoplay() {
  //cleaning the combobox model
  whoplaytablels->clear();
  
  //populating the combobox, here we assign label and values
  Gtk::TreeModel::Row row = *(whoplaytablels->append());
  row[whoplaymodel.col_name] = "Human vs Human";
  row[whoplaymodel.col_value] = huhu;
  
  row = *(whoplaytablels->append());
  row[whoplaymodel.col_name] = "Human (W) vs Engine (B)";
  row[whoplaymodel.col_value] = huce;
  
  row = *(whoplaytablels->append());
  row[whoplaymodel.col_name] = "Human (B) vs Engine (W)";
  row[whoplaymodel.col_value] = cehu;
  
  row = *(whoplaytablels->append());
  row[whoplaymodel.col_name] = "Engine vs Engine";
  row[whoplaymodel.col_value] = cece;
  
  row = *(whoplaytablels->append());
  row[whoplaymodel.col_name] = "Human vs Engine, alternate";
  row[whoplaymodel.col_value] = alte;
}

//setting the active row of the combobox to the time value indicated by configuration 
bool ChessPreferGui::setcbtimeval() {
  int i = 0;
  int ri;
  int time = confmanager->getgametime();
  bool found = false;

  //iterating over the rows to find which one has the value 
  Gtk::TreeModel::Children stl_likemodel = gttablels->children();
  for (Gtk::TreeModel::Children::iterator iter = stl_likemodel.begin(); iter != stl_likemodel.end(); iter++) {
    Gtk::TreeModel::Row row = *iter;
    if (row[gttablemodel.col_value] == time) {ri = i; found = true; break;}
    i++;
  }
  
  if (found) {gttablecb.set_active(ri);}
  return found;
}

//setting the active row of the combobox to the players option indicated by configuration 
bool ChessPreferGui::setcbwhopval() {
  int i = 0;
  int ri;
  bool found = false;
  howplopt wplopt = confmanager->getifhuman();
    
  //iterating over the rows to find which one has the value 
  Gtk::TreeModel::Children stl_likemodel = whoplaytablels->children();
  for (Gtk::TreeModel::Children::iterator iter = stl_likemodel.begin(); iter != stl_likemodel.end(); iter++) {
    Gtk::TreeModel::Row row = *iter;
    if (row[whoplaymodel.col_value] == wplopt) {ri = i; found = true; break;}
    i++;
  }
  
  if (found) {whoplaytablecb.set_active(ri);}
  return found;
}

//setting the player name to the value saved in the config class
void ChessPreferGui::setnamesasconf(Gtk::Entry& enn, unsigned int w) {
  std::string nn = confmanager->getplname(w);
  enn.set_text(nn);
}

//setting the ponder checkbutton to the corresponding state in the config class
void ChessPreferGui::setponderasconf() {
  bool pp = confmanager->getponder();
  checkb_ponder.set_active(pp);
}

//signal handler for ceaddb button, adding a chess engine to the list
void ChessPreferGui::on_add_button_clicked() {
  std::string name, path, txtmess;
  name = cenamee.get_text();
  path = cepathe.get_text();
  
  bool isadded = confmanager->addcheng(name, path);

  if (isadded) {txtmess = "Chess engine added to the list";}
  else {txtmess = "Chess engine cannot be added. May be you left an empty field?\nIf not, some other problem occurred.";}
  
  Gtk::MessageDialog winpopup(*this, txtmess);
  winpopup.run();
  if (isadded) {populatetreeview();}
}

//signal handler for sacecb button, save current settings in the config file
void ChessPreferGui::on_save_custom_clicked() {
  //asking if player is sure
  std::string txtmess = "This will save the current settings for future use\nYou may go back to default settings by clicking the Set default settings button";
  ChessAskyn dialog = ChessAskyn(this, "Confirm", txtmess);
  dialog.run();
    
  if (dialog.getanswer()) {confmanager->saveconf();}
}

//signal handler for sedefb button, setting to default
void ChessPreferGui::on_set_default_clicked() {
  //asking if player is sure
  std::string txtmess = "Warning! This will delete all the chess engines listed and set both players as human.\nDo you want to continue?";
  ChessAskyn dialog = ChessAskyn(this, "Warning", txtmess);
  dialog.run();
  
  if (dialog.getanswer()) {
    confmanager->writedefault();
    populatetreeview();
    setcbtimeval();
    setcbwhopval();
    setnamesasconf(whname, 0);
    setnamesasconf(blname, 1);
  }
}

//signal handler for fipathb button, searching the path by using a dialog
void ChessPreferGui::on_find_path_clicked(Gtk::Entry* entpath) {
  std::string pathofce;
  
  //creating a filechooserdialog object
  Gtk::FileChooserDialog choosecep("Find the binary - chess engine", Gtk::FILE_CHOOSER_ACTION_OPEN);
  choosecep.set_transient_for(*this);
  
  choosecep.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
  choosecep.add_button("_Select", Gtk::RESPONSE_OK);
  
  auto filter_text = Gtk::FileFilter::create();
  filter_text->set_name("Binary File");
  filter_text->add_mime_type("application/octet-stream");
  choosecep.add_filter(filter_text);

  //show dialog and get response
  int choice = choosecep.run();
  
  //handle response
  if (choice == Gtk::RESPONSE_OK) {
    pathofce = choosecep.get_filename();
    entpath->set_text(pathofce);
  }
}

//signal handler for a click in the treeview, showing the popup menu
void ChessPreferGui::on_click_in_treeview(GdkEventButton* evbut) {
  if((evbut->type == GDK_BUTTON_PRESS) && (evbut->button == 3)) {
    popmenu.popup(evbut->button, evbut->time);
  }
}

//signal handler for popup menu entry, allowing to set a chess engine as the one used by a player
void ChessPreferGui::on_popup_menu_setfor(unsigned int w) {  
  bool res = false;
  auto selrow = cetabletv.get_selection();
  if (selrow) {
    Gtk::TreeModel::iterator iter = selrow->get_selected();
    if (iter) {
      Glib::ustring uname = (*iter)[cetablemodel.col_name];
      std::string name = uname;
      res = confmanager->setcheng(w, name);
    }
  }
  
  if (res) {populatetreeview();}
}

//signal handler for popup menu entry, allowing to remove a chess engine from the list
void ChessPreferGui::on_popup_menu_remove() {
  bool res = false;
  auto selrow = cetabletv.get_selection();
  if (selrow) {
    Gtk::TreeModel::iterator iter = selrow->get_selected();
    if (iter) {
      Glib::ustring uname = (*iter)[cetablemodel.col_name];
      std::string name = uname;
      res = confmanager->removecheng(name);
    }
  }
  
  if (res) {populatetreeview();}
}

//signal handler for popup menu entry, it starts the chess engine and shows the options dialog 
void ChessPreferGui::on_popup_menu_options() {
  auto selrow = cetabletv.get_selection();
  if (selrow) {
    Gtk::TreeModel::iterator iter = selrow->get_selected();
    if (iter) {
      Glib::ustring uname = (*iter)[cetablemodel.col_name];
      std::string cename = uname;     
      Glib::ustring upath = (*iter)[cetablemodel.col_path];
      std::string cepath = upath;
      
      //launching the chess engine to read its options.
      ChessUCIGui* ucid = new ChessUCIGui(cename, cepath);
      ucid->init();
      ucid->sendcomm(0); //sending uci
      
      int cc = 0;
      while ((! ucid->checkanswok(1)) && cc < 100) {//checking for uciok
        sleep(0.1);
        ucid->geteansw(true);
        cc++;
      }
      
      //here we do not want to apply the options to the chess engine, but just to read and save the options in the file
      bool isok;
      if (cc == 100) {isok = false;} else {isok = true;}
      if (isok) {
        ucid->readcepref();
        ucid->set_options(cename + " options");
      }

      delete ucid;
    }
  }
}


//signal handler for the time combobox, to update the configuration with the selected value
void ChessPreferGui::on_timecombobox_changed() {
  Gtk::TreeModel::iterator iter = gttablecb.get_active();
  if (iter) {
    Gtk::TreeModel::Row row = *iter;
    if (row) {
      int tt = row[gttablemodel.col_value];
      confmanager->setgametime(tt);
    }
  }
}

//signal handler for the whoplay combobox, to update the configuration with the selected value
void ChessPreferGui::on_whoplaycombobox_changed() {
  bool res;
  Gtk::TreeModel::iterator iter = whoplaytablecb.get_active();
  if (iter) {
    Gtk::TreeModel::Row row = *iter;
    if (row) {
      howplopt opt = row[whoplaymodel.col_value];
      res = confmanager->setifhuman(opt);
      
      if (! res) {
        std::string txtmess = "Warning! Set before a chess engine for the player!\nFor now, we use both human";
        Gtk::MessageDialog winpopup(*this, txtmess);
        winpopup.run();
        
        confmanager->setifhuman(huhu);
        
        //iterating over the rows to find which one has the human human value
        Gtk::TreeModel::Children stl_likemodel = whoplaytablels->children();
        for (Gtk::TreeModel::Children::iterator iter = stl_likemodel.begin(); iter != stl_likemodel.end(); iter++) {
          Gtk::TreeModel::Row row = *iter;
          if (row[whoplaymodel.col_value] == huhu) {
            whoplaytablecb.set_active(iter);
            break;
          }
        }
      }
    }
  }
}

//signal handler for the ponder checkbuttons, to switch pondering on / off
void ChessPreferGui::on_ponder_checkbutton_toggled() {
  bool vv = checkb_ponder.get_active();
  confmanager->setponder(vv);
}

//signal handler for the entry, allowing to update the player name in the config class
void ChessPreferGui::on_entry_changed(unsigned int w) {
  Gtk::Entry* pte;
  
  if (w == 0) {pte = &whname;}
  else if (w == 1) {pte = &blname;}
  
  confmanager->setplname(w, pte->get_text());
}


/* Methods of ChessAnalyseGui
 */
ChessAnalysisGui::ChessAnalysisGui(ChessWindowGui* winparent, ChessUCIGui* uca) {
  set_transient_for(*winparent);
  set_size_request(500, 400);
  analys = uca;
  analys->send_cemessage().connect(sigc::mem_fun(*this, &ChessAnalysisGui::displaycemess));
  
  set_title("Analysis panel");
  
  //packing upper buttons
  ceopt.set_label("Chess Engine Options");
  ceopt.signal_clicked().connect(sigc::mem_fun(*this, &ChessAnalysisGui::on_setoptions_clicked));
  mainbox->pack_start(ceopt, Gtk::PACK_SHRINK, 5);

  //packing the entry
  searchmovlab.set_label("Restrict search: ");
  searchmovlab.set_tooltip_text("Search the bestmove between the moves provided. Moves must be in long algebraic notation separated by a space, e.g. e2e4 d2d4");
  rows.attach(searchmovlab, 0, 0, 1, 1); rows.attach(searchmovent, 1, 0, 1, 1);
  
  //packing the checkbuttons and spinbuttons
  timecbu.set_label("Search time limit: ");
  timecbu.set_tooltip_text("in seconds");
  timecbu.set_active(false);
  timespi.set_digits(1);
  timespi.set_increments(0.1, 1.0);
  timespi.set_range(1.0, 50.0);
  timespi.set_value(5.0);
  timespi.set_numeric(true);
  rows.attach(timecbu, 0, 1, 1, 1); rows.attach(timespi, 1, 1, 1, 1);
  
  depthcbu.set_label("Search depth limit: ");
  depthcbu.set_active(false);
  depthspi.set_digits(0);
  depthspi.set_increments(1.0, 5.0);
  depthspi.set_range(1, 50);
  depthspi.set_value(20);
  depthspi.set_numeric(true);
  rows.attach(depthcbu, 0, 2, 1, 1); rows.attach(depthspi, 1, 2, 1, 1);

  nodecbu.set_label("Search node limit: ");
  nodecbu.set_tooltip_text("in units of 100000");
  nodecbu.set_active(false);
  nodespi.set_digits(0);
  nodespi.set_increments(1.0, 5.0);
  nodespi.set_range(1, 100);
  nodespi.set_value(20);
  nodespi.set_numeric(true);
  rows.attach(nodecbu, 0, 3, 1, 1); rows.attach(nodespi, 1, 3, 1, 1);

  matecbu.set_label("Search mate in moves: ");
  matecbu.set_active(false);
  matespi.set_digits(0);
  matespi.set_increments(1.0, 2.0);
  matespi.set_range(1, 10);
  matespi.set_value(5);
  matespi.set_numeric(true);
  rows.attach(matecbu, 0, 4, 1, 1); rows.attach(matespi, 1, 4, 1, 1);
  
  mainbox->pack_start(rows, Gtk::PACK_START, 5);

  //packing the lower button
  suggm.set_label("Suggest move");
  suggm.signal_clicked().connect(sigc::mem_fun(*this, &ChessAnalysisGui::on_suggest_move_clicked));
  lbuttons.add(suggm);

  stopce.set_label("Stop CE");
  stopce.signal_clicked().connect(sigc::mem_fun(*this, &ChessAnalysisGui::on_stop_chess_engine));
  lbuttons.add(stopce);
  mainbox->pack_start(lbuttons, Gtk::PACK_SHRINK, 5);

  //packing the textarea
  infoarea.set_editable(false); //make the textarea read only: user cannot write directly in the Gtk::TextView
  container.add(infoarea);
  mainbox->pack_start(container);
  
  show_all_children();
}

ChessAnalysisGui::~ChessAnalysisGui() {}

//signal handler for configuring options of chess engine
void ChessAnalysisGui::on_setoptions_clicked() {
  analys->set_options("Analyser options");
}

//signal handler for suggesting the move
void ChessAnalysisGui::on_suggest_move_clicked() {
  CEGoSubcomm infos;
  
  std::string mm = searchmovent.get_text();
  while (mm.back() == ' ') {mm.erase(mm.size()-1, 1);} //erasing all last space characters
  infos.limitmoves = mm;

  if (timecbu.get_active()) {infos.limtime = timespi.get_value();} else {infos.limtime = -1.0;}  
  if (depthcbu.get_active()) {infos.limdepth = depthspi.get_value();} else {infos.limdepth = -1;}
  if (nodecbu.get_active()) {infos.limnodes = nodespi.get_value();} else {infos.limnodes = -1;}
  if (matecbu.get_active()) {infos.mateinmov = matespi.get_value();} else {infos.mateinmov = -1;}
  infos.useme = true;
  
  analys->setgosubcomm(infos);
  analys->sendcomm(6); //sending position
  analys->sendcomm(7); //sending go
  
  while (! analys->checkanswok(3)) {
    sleep(0.1);
    analys->geteansw();
    while (Gtk::Main::events_pending()) {Gtk::Main::iteration();} //to let gtkmm to do other stuffs and update the gui
  }
  
  std::string mmove = analys->getmovetodo();
  displaycemess("*** bestmove found: " + mmove + "\n");
}

//signal handler to stop the analyser
void ChessAnalysisGui::on_stop_chess_engine() {
  analys->sendcomm(8); //sending stop
}

//display messages of the chess engine in the dedicated textarea
void ChessAnalysisGui::displaycemess(std::string cemess) {
  txtipos = ibuffer->insert(txtipos, cemess); //insert the string in the stream to the position.
  infoarea.scroll_to(txtipos);
}

