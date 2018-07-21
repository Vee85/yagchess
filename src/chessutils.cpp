/*
 * chessutils.cpp
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


#include <algorithm> //for std::find function

#include "chessutils.hpp"
#include "chessboard.hpp" //should go here and not in chessutils.hpp to avoid some compiler error related to circular dependencies

/* Static member initalization of class ChessPGN
 */
std::array<std::string, 4> ChessPGN::gresults = {{"*", "1/2-1/2", "0-1", "1-0"}};

/* Methods of class ChessPGN
 */
ChessPGN::ChessPGN(std::string fn, char fmod) {
  pgnfile = fn;
  //writing mode
  if (fmod == 'w') {
    modewrite = true;
    pgnstream.open(pgnfile, std::ios::out | std::ios::trunc); //ios::trunc guarantee that the file is overwritten
  }
  //reading mode
  else if (fmod == 'r') {
    modewrite = false;
    std::ifstream rstream;
    rstream.open(pgnfile, std::ios::in);
    if (rstream.fail()) {
      std::cerr << "Error when Opening the PGN file " << pgnfile << " in reading mode. The file cannot be open." << std::endl;
      std::exit(EXIT_FAILURE);
    }
    
    //scanning the pgn file
    std::string line, allmstr;
    ChessPGN::typetags tags;
    ChessPGN::pgnmoves movetext;
        
    while (! rstream.eof()) {
      std::getline(rstream, line);
      if (line.size() > 1) {
        if (line.front() == '[') {//saving tag pairs
          size_t separator = line.find(" ");
          std::string tagsymbol = line.substr(1, separator-1); //indexes algebra, to get only the not quoted part (square bracket and space excluded)
          
          size_t iniquote = line.find("\"");
          size_t cloquote = line.rfind("\"");
          std::string tagvalue = line.substr(iniquote+1, cloquote-iniquote-1); //indexes algebra, to get only the quoted part (quote excluded)

          tags.insert({{tagsymbol, tagvalue}});

        } else {//saving moves
          //removing comments, checking character by character the presence of a comment
          bool doapp = true;
          std::istringstream linebuf(line);
          while (! linebuf.eof()) {
            char c = linebuf.get();
            if (c == ';') {if (doapp) break;}
            else if (c == '{') {doapp = false;}
            else if (c == '}') {doapp = true;}
            else if (c == '\n') {} //never append the newline char
            else if (c == std::char_traits<char>::eof()) {} //never append the end-of-file char
            else {
              if (doapp) {allmstr.append(1, c);}
            }
          }
        }
      } else {//extracting the moves and storing them in the vector if allmstr is filled, save everything in a PGNgame instance and reset stuffs for the new game
        if (allmstr.size() > 0) {
          std::string strmov, *search;
          std::istringstream clbuf(allmstr);

          int i = 0;
          while (! clbuf.eof()) {
            clbuf >> strmov; //extract a world, using whitespace as separator (default implementation of operator>> )
            if (i % 3 != 0) {
              //this is to exclude the last token which is the result of the game, in case it is not located when i % 3 == 0
              search = std::find(std::begin(gresults), std::end(gresults), strmov);
              if (search == std::end(gresults)) {movetext.push_back(strmov);}
            }
            i++;
          }
    
          PGNgame* cgame = new PGNgame(tags, movetext);
          allgames.push_back(cgame);
          allmstr.clear(); tags.clear(); movetext.clear(); //containers are empty and ready to store info of the next game
        }
      }
    }
    
  } else {
    std::cerr << "Error in ChessPGN initialization: mode is " << fmod << " but could be only \"w\" (write file) or \"r\" (read file)." << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

ChessPGN::~ChessPGN() {
  for (unsigned int i = 0; i < allgames.size(); i++) {delete allgames[i];}
  if (modewrite) {pgnstream.close();}
}

//getting the value of a field, return empty string if the file does not exist
std::string ChessPGN::readfield(unsigned int g, std::string fieldname) {
  std::string res = "";
  if (! modewrite) {
    try {
      PGNgame* pg = allgames.at(g);
      res = pg->getfield(fieldname);

    } catch (const std::out_of_range& e) {
      std::cerr << e.what() << " exception in trying to read a field name, an index game outside the boundary has been provided." << std::endl;
    }
    
  }
  return res;
}

//writing a generic field, needed field name and value (defaul value is "?")
void ChessPGN::writefield(std::string fieldname, std::string fieldvalue) {
  if (modewrite) {
    pgnstream << "[" << fieldname << " \"" << fieldvalue << "\"]\n";
  }
}

//getting the moves, it returns a vector of strings containing the moves (a different string for each move, ordered)
ChessPGN::pgnmoves ChessPGN::readmoves(unsigned int g) {
  pgnmoves res;
  if (! modewrite) {
    try {
      PGNgame* pg = allgames.at(g);
      res = pg->getmovetext();
    } catch (const std::out_of_range& e) {
      std::cerr << e.what() << " exception in trying to read a field name, an index game outside the boundary has been provided." << std::endl;
    }
  }
  return res;
}

//writing the moves, it needs a string with the moves in algebraic notation already formatted and and intenger referring to the result 
void ChessPGN::writemoves(std::string moves, unsigned int rg) {
  //@@@ implement the 80-characters length of line
  if (modewrite) {
    if (rg >= gresults.size()) {
      std::cerr << "Error in ChessPGN writemoves, index result out of range" << std::endl; 
    } else {pgnstream << "\n" << moves << " " << gresults[rg] << std::endl;}
  }
}

//get a pointer to the game corresponding to the given index, return nullptr if the index is out of range
ChessPGN::PGNgame* ChessPGN::getgame(unsigned int i) {
  PGNgame* res = nullptr;
  try {
    res = allgames.at(i);
  } catch (const std::out_of_range& e) {}
  return res;
}

//virtual method empty: this will be implemented by child class
unsigned int ChessPGN::selectgame() {return 0;}


/* Methods of the nested class ChessPGN::PGNgame
 */
ChessPGN::PGNgame::~PGNgame() {}
 
//getting the value of a tag, given the symbol (key).
std::string ChessPGN::PGNgame::getfield(std::string symbol) {
  std::string res = "";
  try {
    res = pgntags.at(symbol);
  } catch (const std::out_of_range& oor) {}
  return res;
}

/* Methods and initialization of class ChessUCI
 */
//initializing static members
std::array<std::string, 11> ChessUCI::guicomms = {{"uci", "debug", "isready", "setoption", "register", "ucinewgame", "position", "go", "stop", "ponderhit", "quit"}};
std::array<std::string, 8> ChessUCI::engineansw = {{"id", "uciok", "readyok", "bestmove", "copyprotection", "registration", "info", "option"}};
std::array<std::string, 5> ChessUCI::optiontypes = {{"check", "spin", "combo", "button", "string"}};
bool ChessUCI::forcestopp = false;

ChessUCI::ChessUCI() : IPCproc() {ptosaver = nullptr;}

ChessUCI::ChessUCI(std::string cen, std::string pathcomm) : ChessUCI() {setcename(cen); setpath(pathcomm);}

ChessUCI::~ChessUCI() {
  if (isactive()) {
    for (unsigned int i = 0; i < checkvector.size(); i++) {delete checkvector[i];}
    for (unsigned int i = 0; i < spinvector.size(); i++) {delete spinvector[i];}
    for (unsigned int i = 0; i < buttonvector.size(); i++) {delete buttonvector[i];}
    for (unsigned int i = 0; i < entryvector.size(); i++) {delete entryvector[i];}
    for (unsigned int i = 0; i < combovector.size(); i++) {delete combovector[i];}
    sendcomm(10); //quit command to close the engine
  }
}

//setting name of chess engine and corresponding filename for saving options
void ChessUCI::setcename(std::string fn) {
  chengname = fn;
  filename = ".yagchess_ce_" + fn;
}

//formatting the moves for a position startpos moves command 
std::string ChessUCI::getprevmoves() {
  std::string hist, nhist;
  
  if (ptosaver == nullptr) {nhist = fixedhistory;}
  else {nhist = ptosaver->gethistory(true, true, true);} //getting the long algebraic notation of the moves up to now
  
  hist = formatprevmoves(nhist);
  while (hist.back() == ' ') {hist.erase(hist.size()-1, 1);} //erasing any white space at the end

  return hist;
}

//formatting the moves for a position startpos moves command 
std::string ChessUCI::formatprevmoves(std::string hh) {
  std::stringstream res;
  std::string strmov;
  std::size_t apos;
  std::stringstream bufhist(hh);
    
  while (! bufhist.eof()) {
    strmov.clear();
    bufhist >> strmov; //extract a move, using whitespace as separator (default implementation of operator>> )
    apos = strmov.find('-');
    if (apos != std::string::npos) {strmov.erase(apos, 1);}
    apos = strmov.find('+');
    if (apos != std::string::npos) {strmov.erase(apos, 1);}
    apos = strmov.find('x');
    if (apos != std::string::npos) {strmov.erase(apos, 1);}
    apos = strmov.find('#');
    if (apos != std::string::npos) {strmov.erase(apos, 1);}
    apos = strmov.find('=');
    if (apos != std::string::npos) {
      char promsymbol = tolower(strmov.back()); //tolower is a cctype library function
      strmov.erase(apos, 2);
      strmov.append(1, promsymbol);
    }
    
    res << strmov << " ";
  }
  
  return res.str();
}

//prepare a command and send to the engine
//@@@ not fully implemented
bool ChessUCI::sendcomm(int i) {
  bool res = true;
  
  for (unsigned int j = 0; j < 8; j++) {answok[j] = false;}
  guimess = guicomms[i];
  
  //looking at the command and set other stuff to send
  if (i == 0 || i == 2 || i == 5 || i == 8 || i == 9 || i == 10) { } //these commands do not need other parameters, nothing to be done here
  
  else if (i == 1) { } //@@@
  
  else if (i == 3) {//building setoption command
    guimess.append(" name ");
    guimess.append(optname);
    if (optvalue != "") {
      guimess.append(" value ");
      guimess.append(optvalue);
    }
    
    optname.clear(); optvalue.clear(); //clear these strings for next message
  }
  
  else if (i == 4) { } //@@@
      
  else if (i == 6) {//building position command
    if (usefen) {//using fen
      guimess.append(" fen ");
      guimess.append(curfen);
    } else {//using startpos
      std::string allmoves = getprevmoves();
      if (allmoves.size() > 1) {
        guimess.append(" startpos moves ");
        guimess.append(allmoves);
      } else {guimess.append(" startpos");}
      if (pondering) {
        guimess.append(" ");
        guimess.append(engponderon);
      }
    }
  }
  
  else if (i == 7) {//building go command
    if (gosubc.useme) {
      if (gosubc.limtime > 0) {
        guimess.append(" movetime ");
        guimess.append(std::to_string(gosubc.limtime*1000));
      } else {guimess.append(" infinite");}
      if (gosubc.limitmoves.size() > 1) {guimess.append(" searchmoves "); guimess.append(gosubc.limitmoves);}
      if (gosubc.limdepth > 0) {guimess.append(" depth "); guimess.append(std::to_string(gosubc.limdepth));}
      if (gosubc.limnodes > 0) {guimess.append(" nodes "); guimess.append(std::to_string(gosubc.limnodes*100000));}
      if (gosubc.mateinmov > 0) {guimess.append(" mate "); guimess.append(std::to_string(gosubc.mateinmov));}
      gosubc.useme = false;
    } else {
      if (thinktime <= 0) {
        if (whtimeleft > 0 && bltimeleft > 0) {
          std::stringstream tcomm;
          tcomm << " wtime " << whtimeleft*1000 << " winc 0 btime " << bltimeleft*1000 << " binc 0 movestogo 1"; //here milliseconds are required
          guimess.append(tcomm.str());
        } else {guimess.append(" infinite");}
      }
      else {guimess.append(" movetime " + std::to_string(thinktime*1000));} //here milliseconds are required
    }
    
    if (pondering) {guimess.append(" ponder");}
  }
  
  else {res = false;}
  
  guimess.append("\n");
  send_to_proc(guimess);
  
  return res;
}

//get an answer from the engine and elaborate it
//@@@not fully implemented
bool ChessUCI::geteansw(bool memo) {
  enginemess = get_from_proc();
  std::string eline, eword, message, *firstword;
  
  std::stringstream buffemess(enginemess);
  
  while (! buffemess.eof()) {
    std::getline(buffemess, eline);
    if (eline != "") {
      std::stringstream buffeline(eline);
      buffeline >> eword;
      firstword = std::find(std::begin(engineansw), std::end(engineansw), eword);
      
      if (*firstword == engineansw[0]) {//id
        message.clear();
        std::getline(buffeline, message);
        if (message[0] == ' ') {message.erase(0, 1);}
        ch_eng_communication(message);
        answok[0] = true;
      }
      
      else if (*firstword == engineansw[1]) {answok[1] = true;} //no need of further elaboration
      
      else if (*firstword == engineansw[2]) {answok[2] = true;} // no need of further elaboration
      
      else if (*firstword == engineansw[3]) {//bestmove
        engbestmove.clear();
        buffeline >> engbestmove;
        if (pondering) {
          buffeline >> engponderon; //to discard the ponder word before the move
          engponderon.clear();
          buffeline >> engponderon;
        } else {engponderon.clear();}
        answok[3] = true;
      }
      
      else if (*firstword == engineansw[4]) {answok[4] = true;} //@@@
      
      else if (*firstword == engineansw[5]) {answok[5] = true;} //@@@
      
      else if (*firstword == engineansw[6]) {//info
        message.clear();
        std::getline(buffeline, message);
        if (message[0] == ' ') {message.erase(0, 1);}
        ch_eng_communication(message);
        answok[6] = true;
      }
      
      else if (*firstword == engineansw[7]) {//option
        if (memo) {//memorizing option
          bool memok = memorizeoption(eline);
          if (! memok) {std::cerr << "Warning, failed to parse string: \"" << eline << "\" when memorizing an engine option." << std::endl;}
        }
        answok[7] = true;
      }
      else if (firstword == std::end(engineansw)) {
        std::cerr << "Error, engine returned answer: " << eword << ", which is not in the UCI protocol." << std::endl;
        return false;
      }
    }
  }
  
  return true;
}

//set time left on the clocks
void ChessUCI::get_lefttimes(double wt, double bt) {
  whtimeleft = wt;
  bltimeleft = bt;
}

//memorizing one editable option of the engine, parsing a single line sent by the engine containing an "option" command
bool ChessUCI::memorizeoption(std::string line) {
  bool parsingok = true, fn = true;
  bool readname = true, readtype = false;
  std::string token, value, *search;
  std::stringstream buffline(line);
  std::ostringstream name;
  
  //recovering option name and definition
  buffline >> token; //discarding first word, option
  while (! buffline.eof()) {
    buffline >> token;
    if (token == "name") {readname = true; readtype = false;}
    else if (token == "type") {readname = false; readtype = true;}
    else {
      if (readname) {
        if (fn) {
          name << token;
          fn = false;
        } else {name << " " << token;}
      }
      if (readtype) {
        //verifying that type is one of the 5 allowed and saving the data in the structs in the vectors
        search = std::find(std::begin(optiontypes), std::end(optiontypes), token);
        if (search == std::end(optiontypes)) {parsingok = false;}

        else if (*search == optiontypes[0]) {//type check
          CheckOption* acheopt = new CheckOption;
          acheopt->name = name.str();
          while (!buffline.eof()) {//reading the other worlds of the line in the inner loop
            buffline >> token; buffline >> value;
            if (token == "default") {
              if (value == "true") {acheopt->value = true; acheopt->defvalue = false;}
              else if (value == "false") {acheopt->value = false; acheopt->defvalue = false;}
            } else {parsingok = false;}
          }
          checkvector.push_back(acheopt);
        }
        
        else if (*search == optiontypes[1]) {//type spin
          SpinOption* aspiopt = new SpinOption;
          aspiopt->name = name.str();
          while (!buffline.eof()) {//reading the other worlds of the line in the inner loop
            buffline >> token; buffline >> value;
            if (token == "default") {aspiopt->defvalue = std::stoi(value); aspiopt->value = std::stoi(value);}
            else if (token == "min") {aspiopt->min = std::stoi(value);}
            else if (token == "max") {aspiopt->max = std::stoi(value);}
            else {parsingok = false;}
          }
          spinvector.push_back(aspiopt);
        }
        
        else if (*search == optiontypes[2]) {//type combo
          bool addtodef = false;
          bool addtovar = false;
          ComboOption* acomopt = new ComboOption;
          acomopt->name = name.str();
          while (!buffline.eof()) {//reading the other worlds of the line in the inner loop
            buffline >> token;
            if (token == "default") {addtodef = true;}
            else if (token == "var") {
              if (addtodef) {value.pop_back(); acomopt->defvalue = value; acomopt->value = value; value.clear(); addtodef = false;}
              if (addtovar) {value.pop_back(); acomopt->allowedvals.push_back(value); value.clear(); addtovar = false;}
              addtovar = true;
            } else {
              value.append(token);
              value.append(" ");
            }
          }
          if (addtovar) {value.pop_back(); acomopt->allowedvals.push_back(value); value.clear(); addtovar = false;} //to get last var
          combovector.push_back(acomopt);
        }
        
        else if (*search == optiontypes[3]) {//type button
          ButtonOption* abutopt = new ButtonOption;
          abutopt->name = name.str();
          while (!buffline.eof()) { }//reading the other worlds of the line in the inner loop, but there should not be anything else
          buttonvector.push_back(abutopt);
        }
        
        else if (*search == optiontypes[4]) {//type string (will be rendered by an entry)
          bool addtodef = false;
          EntryOption* aentopt = new EntryOption;
          aentopt->name = name.str();
          while (!buffline.eof()) {//reading the other worlds of the line in the inner loop
            buffline >> token;
            if (token == "default") {addtodef = true;}
            else {
              value.append(token);
              value.append(" ");
            }
          }
          if (addtodef) {
            if (value.size() > 0) {value.pop_back();}
            aentopt->defvalue = value;
            aentopt->value = value;
            value.clear();
            addtodef = false;
          }
          entryvector.push_back(aentopt);
        }
        
      }
    }
  }
  
  return parsingok;
}

//writing chess engine options into the dedicated file
void ChessUCI::writecepref() {
  std::ofstream outcepref;
  outcepref.open(filename, std::ios::out);
  
  //saving the check options
  CheckOption* checkopt;
  for (unsigned int i = 0; i < checkvector.size(); i++) {
    checkopt = checkvector[i];
    std::string booltxt;
    if (checkopt->value) {booltxt = "true";}
    else {booltxt = "false";}
    outcepref << optiontypes[0] << ":" << checkopt->name << "=" << booltxt << std::endl;
  }
  
  //saving the spin options
  SpinOption* spinopt;
  for (unsigned int i = 0; i < spinvector.size(); i++) {
    spinopt = spinvector[i];
    outcepref << optiontypes[1] << ":" << spinopt->name << "=" << spinopt->value << std::endl;
  }
  
  //saving the combo options
  ComboOption* comboopt;
  for (unsigned int i = 0; i < combovector.size(); i++) {
    comboopt = combovector[i];
    outcepref << optiontypes[2] << ":" << comboopt->name << "=" << comboopt->value << std::endl;
  }
  
  /* NOTE: nothing to save for the button options */
  
  //saving the entry options
  EntryOption* entryopt;
  for (unsigned int i = 0; i < entryvector.size(); i++) {
    entryopt = entryvector[i];
    outcepref << optiontypes[4] << ":" << entryopt->name << "=" << entryopt->value << std::endl;
  }
  
  outcepref.close();
}

//reading chess engine options from the dedicated file, return false only if file does not exist
bool ChessUCI::readcepref() {
  bool res = true;
  
  if (std::ifstream(filename)) {//file exhist, reading it
    std::ifstream incepref;
    incepref.open(filename, std::ios::in);
    std::string cline, optype, opname, opval;
    
    //parsing the file line by line
    while (! incepref.eof()) {
      std::getline(incepref, cline);
      std::stringstream clinebuf(cline);
      std::getline(clinebuf, optype, ':');
      std::getline(clinebuf, opname, '=');
      std::getline(clinebuf, opval);
      
      if (optype == optiontypes[0]) {//if check option
        bool boval;
        if (opval == "true") {boval = true;}
        else if (opval == "false") {boval = false;}
        for (unsigned int i = 0; i < checkvector.size(); i++) {
          if (checkvector[i]->name == opname) {
            checkvector[i]->value = boval;
            break;
          }
        }
      }
      
      if (optype == optiontypes[1]) {//if spin option
        for (unsigned int i = 0; i < spinvector.size(); i++) {
          if (spinvector[i]->name == opname) {
            spinvector[i]->value = std::stoi(opval);
            break;
          }
        }
      }
      
      if (optype == optiontypes[2]) {//if combo option
        for (unsigned int i = 0; i < combovector.size(); i++) {
          if (combovector[i]->name == opname) {
            combovector[i]->value = opval;
            break;
          }
        }
      }
      
      /* NOTE: button options are not saved (no need of doing it), so are not found in the file */
      
      if (optype == optiontypes[4]) {//if string/entry option
        for (unsigned int i = 0; i < entryvector.size(); i++) {
          if (entryvector[i]->name == opname) {
            entryvector[i]->value = opval;
            break;
          }
        }
      }
    }
  } else {res = false;}//file does not exist
  
  return res;
}

//setting the preferences, sending the proper UCI commands to the chess engine, if todef = true default options are sent
void ChessUCI::applycepref(bool todef) {
  std::string oname, ovalue;
  
  /* NOTE: type button needs only a click, the command is sent immediately by the dialog and not managed by this function */
  
  for (unsigned int i = 0; i < checkvector.size(); i++) {//edit check type value
    oname = checkvector[i]->name;
    bool oval = checkvector[i]->value;
    bool dval = checkvector[i]->defvalue;
    bool useme;
    if (todef) {useme = dval;} else {useme = oval;}
    
    if (oval != dval) {
      if (useme) {ovalue = "true";}
      else {ovalue = "false";}
      paramoptions(oname, ovalue);
      sendcomm(3);
    }
  }
  
  for (unsigned int i = 0; i < spinvector.size(); i++) {//edit spin type value
    oname = spinvector[i]->name;
    int oval = spinvector[i]->value;
    int dval = spinvector[i]->defvalue;
    if (todef) {ovalue = std::to_string(dval);} else {ovalue = std::to_string(oval);}
    
    if (oval != dval) {
      paramoptions(oname, ovalue);
      sendcomm(3);
    }
  }

  for (unsigned int i = 0; i < combovector.size(); i++) {//edit combo type value
    oname = combovector[i]->name;
    std::string oval = combovector[i]->value;
    std::string dval = combovector[i]->defvalue;
    if (todef) {ovalue = dval;} else {ovalue = oval;}
    
    if (oval != dval) {
      paramoptions(oname, ovalue);
      sendcomm(3);
    }
  }
  
  for (unsigned int i = 0; i < entryvector.size(); i++) {//edit entry/string type value
    oname = entryvector[i]->name;
    std::string oval = entryvector[i]->value;
    std::string dval = entryvector[i]->defvalue;
    if (todef) {ovalue = dval;} else {ovalue = oval;}
    
    if (oval != dval) {
      paramoptions(oname, ovalue);
      sendcomm(3);
    }
  }
}

/* Methods of class ChessConfig
 */
//initializing static members
std::array<std::string, 9> ChessConfig::confkeys = {{"chess_engines_list", "chess_engine_white", "chess_engine_black", "chess_engine_analyser", 
  "whoplayers", "white_name", "black_name", "game_time", "ponder"}};
ChessConfig::howplmap ChessConfig::sethowpl = {{"huhu", huhu}, {"huce", huce}, {"cehu", cehu}, {"cece", cece}, {"alte", alte}};
std::string ChessConfig::nulltxt = "none";

ChessConfig::ChessConfig() {}

ChessConfig::~ChessConfig() {}

//initializing the object
void ChessConfig::initcc(std::string fna, std::string fnb) {
  configfile = fna;
  altefile = fnb;
  bool checkread;
  
  //checking configfile
  if (std::ifstream(configfile)) {
    std::ifstream inconfigbuf;
    inconfigbuf.open(configfile, std::ios::in);
    checkread = readcurrent(inconfigbuf);
    inconfigbuf.close();
    
    if (! checkread) {
      std::cerr << "Error when loading the config file, a new config file with the default parameters will be created and used" << std::endl;
      writedefault();
    }
  } else {writedefault();}
  
  //checking altefile
  if (std::ifstream(altefile)) {
    std::ifstream inaltebuf;
    std::string alline;
    inaltebuf.open(altefile, std::ios::in);
    std::getline(inaltebuf, alline);
    if (alline == "white") {shumpl = white;}
    else if (alline == "black") {shumpl = black;}
    else {
      std::cerr << "Error when loading information from " << altefile << ". We use the default value" << std::endl;
      shumpl = white;
    }
  } else {shumpl = white;}
}

//add a chess engine to the list (the current path)
bool ChessConfig::addcheng(std::string name, std::string path) {
  bool res;
  if (name != "" && path != "") {
    auto empres = chessengines.emplace(name, path);
    res = empres.second;
  } else {res = false;}
  
  return res;
}

//remove chess engine from the list
bool ChessConfig::removecheng(std::string name) {
  bool res;
  int ires = chessengines.erase(name);
  if (ires == 1) {res = true;}
  else {res = false;}
  
  if (res) {
    for (unsigned int i = 0; i < chengname.size(); i++) {
      if (chengname[i] == name) {chengname[i] = nulltxt;}
    }
  }
  
  return res;
}

//set the chess engine path for a player
bool ChessConfig::setcheng(unsigned int w, std::string name) {
  bool res = true;
  if (name == nulltxt) {chengname[w] = nulltxt;}
  else {
    auto it = chessengines.find(name);
    if (it == chessengines.end()) {
      std::cerr << "Warning! Something wrong in assignation of a chess engine to a player, you use a wrong name!" << std::endl;
      res = false;
    } else {chengname[w] = it->first;}
  }
  
  return res;
}

//set who is human and who is chess engine, return false if a set engine is requested for a player but is not set 
bool ChessConfig::setifhuman(howplopt spl) {
  whoishum = spl;
  bool res = true;
  if (whoishum == huhu) {} // nothing to be done here
  else if (whoishum == huce) {
    if (chengname[0] == nulltxt) {res = false;}
  } else if (whoishum == cehu) {
    if (chengname[1] == nulltxt) {res = false;}
  } else if (whoishum == cece) {
    if (chengname[0] == nulltxt || chengname[1] == nulltxt) {res = false;}
  } else if (whoishum == alte) {
    if (chengname[0] == nulltxt || chengname[1] == nulltxt) {res = false;}
  }
  
  return res;
}

//get if players are human or not, return an array of length 2, first white second black, the c_color argument is used only in case of alternating  
std::array<bool, 2> ChessConfig::getifhumanbool(c_color whum) {
  std::array<bool, 2> res;
  if (whoishum == huhu) {res[0] = true; res[1] = true;}
  else if (whoishum == huce) {res[0] = true; res[1] = false;}
  else if (whoishum == cehu) {res[0] = false; res[1] = true;}
  else if (whoishum == cece) {res[0] = false; res[1] = false;}
  else if (whoishum == alte) {
    if (whum == white) {res[0] = true; res[1] = false;}
    else if (whum == black) {res[0] = false; res[1] = true;}
  }
  
  return res;
}

//get name of the chess engine for player
std::string ChessConfig::getchengname(unsigned int w) const {
  std::string res;
  if (chengname[w] == nulltxt) {res = "";}
  else {res = chengname[w];}
  return res;
}

//get path of the chess engine for player
std::string ChessConfig::getchengpath(unsigned int w) const {
  std::string res;
  if (chengname[w] == nulltxt) {res = "";}
  else {
    try {res = chessengines.at(chengname[w]);}
    catch (std::out_of_range e) {
      std::cerr << "Warning! " << e.what() << " Something wrong in retrieving the chess engine path associated to a name, you use a wrong name!" << std::endl;
      res = "";
    }
  }
  return res;
}

//write file with the color of the last human player, which may not be the one saved in this class as shumpl, it is update by the class managing the game
void ChessConfig::writestarthum(c_color lhc) {
  std::ofstream outaltebuf;
  outaltebuf.open(altefile, std::ios::out);
  
  std::string coltxt;
  if (lhc == white) {coltxt = "white";}
  else if (lhc == black) {coltxt = "black";}
  
  outaltebuf << coltxt << std::endl;
  outaltebuf.close();
}

//write new config file with default parameters
void ChessConfig::writedefault() {
  //set internal variables to default values (which are chosen here): no chess engine in the list, both players human, no time limit
  chessengines.clear();
  setifhuman(huhu);
  for (unsigned int i = 0; i < chengname.size(); i++) {chengname[i] = nulltxt;}
  for (unsigned int k = 0; k < playernames.size(); k++) {playernames[k] = "";}
  gametime = -1;
  ponder = false;

  saveconf();
}

//edit the config file, writing in it current values
void ChessConfig::saveconf() {
  std::ofstream outconfigbuf;
  outconfigbuf.open(configfile, std::ios::out);

  outconfigbuf << confkeys[0] << "=";
  if (chessengines.size() > 0) {
    unsigned int i = 0;
    for (auto it = chessengines.begin(); it != chessengines.end(); ++it) {
      outconfigbuf << it->first << ":" << it->second;
      if (i < chessengines.size()-1) {outconfigbuf << "|";} // to not append the | to the last couple, it will cause problems in readcurrent
      i++;
    }
  }
  outconfigbuf << std::endl;
  
  for (unsigned int i = 0; i < chengname.size(); i++) {
    outconfigbuf << confkeys[i+1] << "=" << chengname[i] << std::endl;
  }

  std::string vv;
  for (howplmap::iterator ite = sethowpl.begin(); ite != sethowpl.end(); ite++) {
    if (whoishum == ite->second) {
      vv = ite->first;
      break;
    }
  }
  outconfigbuf << confkeys[4] << "=" << vv << std::endl;

  for (unsigned int i = 0; i < playernames.size(); i++) {
    outconfigbuf << confkeys[i+5] << "=" << playernames[i] << std::endl;
  }
  
  outconfigbuf << confkeys[7] << "=" << gametime << std::endl;
  std::string strponder;
  if (ponder) {strponder = "true";} else {strponder = "false";}
  outconfigbuf << confkeys[8] << "=" << strponder << std::endl;
  
  outconfigbuf.close();
}

//read info from config file
bool ChessConfig::readcurrent(std::ifstream &confbuf) {
  bool res = true;
  bool tres;
  std::string cline, token, argum, cename, cepath;
  while (! confbuf.eof()) {
    std::getline(confbuf, cline);
    std::stringstream clinebuf(cline);
    std::getline(clinebuf, token, '=');
    std::getline(clinebuf, argum);
    
    if (token == "") {} //to avoid the always present blank token not being recognized
    else if (token == confkeys[0]) {
      if (argum.size() > 0) {
        std::stringstream argumbuff(argum);
        while (! argumbuff.eof()) {
          std::getline(argumbuff, cename, ':');
          std::getline(argumbuff, cepath, '|'); //works because the last couple is not terminated by | and getline goes until the newline
          tres = addcheng(cename, cepath);
          if (! tres) {res = false;}
        }
      }
    }
    
    else if (token == confkeys[1]) {
      tres = setcheng(0, argum);
      if (! tres) {res = false;}
    }

    else if (token == confkeys[2]) {
      tres = setcheng(1, argum);
      if (! tres) {res = false;}
    }
  
    else if (token == confkeys[3]) {
      tres = setcheng(2, argum);
      if (! tres) {res = false;}
    }
    
    else if (token == confkeys[4]) {whoishum = sethowpl.at(argum);}
    
    else if (token == confkeys[5]) {setplname(0, argum);}
    else if (token == confkeys[6]) {setplname(1, argum);}
    
    else if (token == confkeys[7]) {
      int gt = std::stoi(argum);
      setgametime(gt);
    }
    
    else if (token == confkeys[8]) {
      if (argum == "true") {setponder(true);}
      else if (argum == "false") {setponder(false);}
    }
    
    else {res = false;}
  }
  
  return res;
}
