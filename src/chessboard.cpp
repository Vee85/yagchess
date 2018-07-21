/*
 * chessboard.cpp
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
#include <unistd.h> //for sleep function
#include <chrono> //for typedef time_t, localtime function, system_clock class

#include "chessboard.hpp"

/* ChessExts initialization
 */
//std::string ChessExts::saveext = ".gchess"; //extension for files to saving the game, native format
std::string ChessExts::saveext = ".pgn"; //extension for files to saving the game, using the standard PGN format
std::string ChessExts::histext = ".hchess"; //extension for text files to write the history of the game in algebraic notation

/* ChessSaving methods and initialization
 */
ChessSaving::ChessSaving() {}

ChessSaving::~ChessSaving() {
  savebuf.close();
  //the following two lines allows to extract a C-like string from a stringstream 
  const std::string& tmp = filename.str();
  const char* cfn = tmp.c_str();
  std::remove(cfn);
}

//initializing the object
void ChessSaving::initcs(std::string fn) {
  int n = 0;
  bool fncheck = true;
  filename << fn;
  while (fncheck) {
    if (std::ifstream(filename.str())) {
      filename.str("");
      filename << fn << '_' << n;
      n++;
    } else {fncheck = false;}
  }
  savebuf.open(filename.str(), std::ios::out | std::ios::in | std::ios::trunc); //needed ios::trunc here, if ios::in is provided without ios::trunc, the file is supposed to exist, no new file is created.
}

//chech for rule of three moves repeated for draw, better doing it here in ChessSaving rather than from the ChessBoard
bool ChessSaving::drawforthree() {
  bool res;
  std::string fullcurline, opcurline, fullreferline, opreferline;
  std::streampos lastline = linepos.back();
  std::streampos clinep;
  
  //retrieving last line for reference
  savebuf.seekg(lastline);
  std::getline(savebuf, fullreferline);
  opreferline = dftsetline(fullreferline);
    
  int eql = 0;
  for (unsigned int i = 1; i < linepos.size(); i++) {
    std::streampos clinep = linepos[i];
    savebuf.seekg(clinep);
    std::getline(savebuf, fullcurline);
    
    opcurline = dftsetline(fullcurline);
    
    if (opcurline == opreferline) {eql++;}
  }
  
  if (eql > 3) {res = true;}
  else {res = false;}
  
  return res;
}

//build a string discarding the stuff unnecessary for comparing piece disposition on the chessboard
std::string ChessSaving::dftsetline(std::string fullline) {
  std::string opline, res;
  
  std::stringstream buffull(fullline);
  
  std::getline(buffull, opline, '|'); //discarding first info on the chessboard
  std::getline(buffull, opline, '|'); //find the player who moves
  res = opline;
  res.append("|");
  std::getline(buffull, opline, '*'); //discarding last info on the chessboard
  std::getline(buffull, opline, '*'); //discarding short algebraic notation
  std::getline(buffull, opline, '*'); //discarding long algebraic notation
  std::getline(buffull, opline);
  
  res.append(opline);
  return res;
}

//saving the current status of the game (write a single line in the autosave file)
void ChessSaving::autosavegame(const ChessBoard& cbd, bool isbeginning) {
  Piece* pp;
  std::stringstream scc;
  std::streampos thispos;
  
  clearfuture();
  
  thispos = savebuf.tellg();
  linepos.push_back(thispos);
  lineiter = linepos.end();
  lineiter--; //to get the iterator pointing the last element, not the past-to-end element.
  
  if (isbeginning) {scc << cbd.players[1]->whoplay();}//is black
  else {scc << cbd.player_moving->whoplay();}
  
  savebuf << cbd.turn << '|' << scc.str() << '|' << cbd.drawffcounter << "*" << cbd.algebnotshort.str() << "*" << cbd.algebnotlong.str() << "*";
  for (unsigned int i = 0; i < cbd.pieces.size(); i++) {
    pp = cbd.pieces[i];
    savebuf << *pp;
  }
  
  savebuf << std::endl;
}

//load game status from a line from the savefile, it returns a string with the move corresponding to the line in short algebraic notation for printing purpose
std::string ChessSaving::loadstatus(ChessBoard& chb) {
  std::string cline, gpar, ppar, tcpar;
  std::ostringstream res;
  savebuf.seekg(*lineiter);

  std::getline(savebuf, cline);
  std::stringstream clinebuf(cline);

  //general parameters of the turn
  std::getline(clinebuf, gpar, '|');
  chb.turn = std::stoi(gpar);
  res << gpar;
  
  //assigning the opposite player: the savings store the player who did the move, not the player who moves after that move
  std::getline(clinebuf, gpar, '|');
  if (gpar == "White") {
    chb.player_moving = chb.players[1];
    res << ". ";
  } else if (gpar == "Black") {
    chb.player_moving = chb.players[0];
    res << "... ";
  }
  
  //correct turn for turnlost
  if (gpar == "Black" && turnlost) {chb.turn++;} 
  
  //assigning the counter for draw game
  std::getline(clinebuf, gpar, '*');
  chb.drawffcounter = std::stoi(gpar);

  //assignign the algebraic notation strings
  std::getline(clinebuf, gpar, '*');
  chb.algebnotshort.str(gpar);
  if (gpar == "---") {res.str("   ");} //to an empy line for the initial condition
  else {res << gpar;}
  std::getline(clinebuf, gpar, '*');
  chb.algebnotlong.str(gpar);

  //modifying the pieces
  Piece* ptoadd, *ptoput;
  c_color cc;
  int nid, px, py;
  bool ong, bmv;

  //moved pieces
  for (unsigned int k = 0; k < chb.pieces.size(); k++) {delete chb.pieces[k];}
  chb.pieces.clear();
  chb.whking = nullptr;
  chb.blking = nullptr;
  
  while (! clinebuf.eof()) {
    std::getline(clinebuf, ppar, '|');
    if (ppar != "") {
      nid = (int)(ppar[1] - '0');
      if (ppar[2] == '0') {cc = black;} else if (ppar[2] == '1') {cc = white;}
      ong = (bool)(ppar[3] - '0');
      bmv = (bool)(ppar[4] - '0');
      px = (int)(ppar[5] - '0');
      py = (int)(ppar[6] - '0');
      
      if (ppar[0] == '1') {ptoadd = new ChessPawn(nid, cc, ong, bmv, px, py);}
      else if (ppar[0] == '2') {ptoadd = new ChessRock(nid, cc, ong, bmv, px, py);}
      else if (ppar[0] == '3') {ptoadd = new ChessKnight(nid, cc, ong, bmv, px, py);}
      else if (ppar[0] == '4') {ptoadd = new ChessBishop(nid, cc, ong, bmv, px, py);}
      else if (ppar[0] == '5') {ptoadd = new ChessQueen(nid, cc, ong, bmv, px, py);}
      else if (ppar[0] == '6') {
        ptoadd = new ChessKing(nid, cc, ong, bmv, px, py);
        if (cc == black) {chb.blking = dynamic_cast<ChessKing*>(ptoadd);}
        else if (cc == white) {chb.whking = dynamic_cast<ChessKing*>(ptoadd);}
      }
      else if (ppar[0] == '7') {ptoadd = new ChessFakePawn(nid, cc, ong, bmv, px, py);}
      chb.pieces.push_back(ptoadd);
    }
  }
  
  //deleting all the pointers of the ChessSquare
  for (int i = CHVector::min_x; i < CHVector::max_x; i++) {
    for (int j =  CHVector::min_y; j < CHVector::max_y; j++) {
      chb.emptysquare(i, j);
    }
  }
  
  //placing the reconstructed pieces on the ChessBoard
  for (unsigned int q = 0; q < chb.pieces.size(); q++) {
    ptoput = chb.pieces[q];
    chb.squareinpiece(ptoput);
  }
  
  //fixing the reference pawn in fakepawns (for en passant eating)
  ChessPawn* refp;
  ChessFakePawn *tfkp, *nfkp;
  for (unsigned int l = 0; l < chb.pieces.size(); l++) {
    if (chb.pieces[l]->getidtype() == fakepawn) {
      tfkp = dynamic_cast<ChessFakePawn*>(chb.pieces[l]);
      if (tfkp->getcolor() == white) {
        refp = dynamic_cast<ChessPawn*>(chb.squares[tfkp->getx()][tfkp->gety()-1].p);
      }
      else if (tfkp->getcolor() == black) {
        refp = dynamic_cast<ChessPawn*>(chb.squares[tfkp->getx()][tfkp->gety()+1].p);
      }
      nfkp = new ChessFakePawn(tfkp, refp);
      chb.pieces[l] = nfkp;
      chb.squareinpiece(nfkp);
      delete tfkp;
    }
  }
  
  return res.str();
}

//delete iterator pointers of all saves of old future moves after back moves when doing a new move, return true if there is something to delete
bool ChessSaving::clearfuture(bool onlycheck) {
  bool res = false;
  //it makes use of some iterator algebra to verify that there are not later elements than lineiter in the linepos vector 
  int a = lineiter - linepos.begin();
  int b = linepos.end() - linepos.begin() - 1;
  if (a != b && b != -1) {
    res = true;
    if (! onlycheck) {linepos.erase(a + linepos.begin()+1, linepos.end());}
  }
  return res;
}

//extract history of the game in algebraic notation from the saving file
std::string ChessSaving::gethistory(bool aligned, bool getlong, bool plain) {
  std::string curline, cpar;
  std::stringstream res;
  
  std::string delimiter;
  if (aligned) {delimiter = "\n";}
  else {delimiter = " ";}
  
  for (unsigned int i = 1; i < linepos.size(); i++) {//from 1 to discard the first line correspondind to the initial disposition of the pieces
    std::streampos clinep = linepos[i];
    savebuf.seekg(clinep);
    std::getline(savebuf, curline);
    
    std::stringstream clinebuf(curline);
    
    std::getline(clinebuf, cpar, '*'); //this is to discard whatever is before the first *
    std::getline(clinebuf, cpar, '*'); //this is to get only what is up to the second *. Now gpar contains the short algebraic notation of the move
    if (getlong) {std::getline(clinebuf, cpar, '*');} //if this is done, cpar contains the long algebraic notation of the move
    
    if (plain) {res << cpar << " ";}
    else {
      if (i % 2 == 0) {res << " " << cpar << delimiter;}
      else {res << (i+1)/2 << ". " << cpar;}
    }
  }
  
  return res.str();
}

//write history of the game in a txt file
void ChessSaving::writefhist(std::string hfilename, bool longnot) {
  std::string fullh = gethistory(true, longnot);
  std::ofstream hst;
  hst.open(hfilename, std::ios::out);
  hst << fullh << std::endl;
  hst.close();
}

//save game in native format, write a file
void ChessSaving::savegame(const ChessBoard& chb, std::string sfilename) {
  std::ofstream fbuff;
  fbuff.open(sfilename, std::ios::out | std::ios::trunc | std::ios::binary);
  
  std::string curline;
  std::stringstream res;

  //write game information not written in the autosave file
  for (unsigned int i = 0; i < chb.players.size(); i++) {fbuff << chb.players[i]->isplayerhuman() << "|";}
  fbuff << "*";
  
  for (unsigned int i = 0; i < linepos.size(); i++) {//writing position
    fbuff << linepos[i] << "|";
  }
  fbuff << "\n";
  
  //copy line by line, in order to save only the lines marked by linepos and automatically discard the line unmarked (e.g. after some back command and repetition of moves)
  for (unsigned int i = 0; i < linepos.size(); i++) {
    std::streampos clinep = linepos[i];
    savebuf.seekg(clinep);
    std::getline(savebuf, curline);
    fbuff << curline << "\n";
  }
  
  fbuff.close();
}

//save game in pgn format, write a file
void ChessSaving::savegamepgn(const ChessBoard& chb, std::string pgnfilename, const ChessConfig& rconf) {
  ChessPGN pgnfw(pgnfilename, 'w');
  
  //getting current date
  char tchbuff [20];
  using std::chrono::system_clock;
  system_clock::time_point curtime = system_clock::now();  
  std::time_t tt = system_clock::to_time_t(curtime);
  struct tm* timeinfo;
  timeinfo = localtime(&tt);
  strftime(tchbuff, 20, "%Y.%m.%d", timeinfo);
  std::string strtime = tchbuff;
  
  //getting the result
  int rg;
  if (chb.getfinalres() == notfinished) {rg = 0;}
  else if (chb.getfinalres() == tie) {rg = 1;}
  else if (chb.getfinalres() == blackwins) {rg = 2;}
  else if (chb.getfinalres() == whitewins) {rg = 3;}
  
  //getting the player names
  std::string tns;
  std::array<std::string, 2> nms;
  for (unsigned int i = 0; i < 2; i++) {
    tns = rconf.getplname(i);
    if (tns == "") {nms[i] = "?";}
    else {nms[i] = tns;}
  }
  
  //writing the seven pgn format mandatory entries
  pgnfw.writefield("Event");
  pgnfw.writefield("Site");
  pgnfw.writefield("Date", strtime);
  pgnfw.writefield("Round");
  pgnfw.writefield("White", nms[0]);
  pgnfw.writefield("Black", nms[1]);
  pgnfw.writefield("Result", ChessPGN::gresults[rg]);
  
  //writing FEN entry if needed
  if (inifen.size() > 0) {
    pgnfw.writefield("FEN", inifen);
    pgnfw.writefield("SetUp", "1");
  }
  
  //writing moves in algebraic notation
  std::string devgame = gethistory(false);
  pgnfw.writemoves(devgame, rg);
}

//load game from file (native format), returns true if the game is successfully loaded.
bool ChessSaving::loadgame(ChessBoard& chb, std::string lfilename) {
  bool status = true;
  std::array<bool, 2> humplayers; 
  std::ifstream sbuff;
  
  try {
    sbuff.open(lfilename, std::ios::in | std::ios::binary);
    std::string cl, cp;
    
    //loading the first line
    std::getline(sbuff, cl);
    std::stringstream clbuf(cl);
    
    //setting if each player is human or not 
    for (unsigned int i = 0; i < humplayers.size(); i++) {
      std::getline(clbuf, cp, '|');
      if (cp == "0") {humplayers[i] = false;}
      else {humplayers[i] = true;}
    }
    
    std::getline(clbuf, cp, '*'); //getting rid of the * character
    
    //setting pointers to each line
    linepos.clear();
    while (! clbuf.eof()) {
      std::getline(clbuf, cp, '|');
      if (cp != "") {
        int pp = std::stoi(cp); //pp is an int, but it is used as a std::streampos in the next line: automatic conversion
        linepos.push_back(pp);
      }
    }
    
    //set lineiter to the last linepos
    lineiter = linepos.end();
    lineiter--;
    
    //close and reopen file to erase the content (content is erased by the flat ios::trunc)
    savebuf.close();
    savebuf.open(filename.str(), std::ios::out | std::ios::in | std::ios::trunc);

    //copy lines in the autosave file
    while (! sbuff.eof()) {
      std::getline(sbuff, cl);
      savebuf << cl << "\n";
    }
    
    //setting the game to the last linepos
    loadstatus(chb);
    
  } catch (const std::ifstream::failure e) {
    std::cerr << e.what() << std::endl;
    status = false;
  } catch (std::invalid_argument e) {
    std::cerr << e.what() << std::endl;
    status = false;
  }
  
  return status;
}

//load a game saved with the standard PGN format
bool ChessSaving::loadgamepgn(ChessBoard& chb, ChessPGN::pgnmoves allmoves) {
  bool status = true;
  std::ifstream sbuff;
  
  //do the moves
  c_color pwm;
  bool validmove;
  for (unsigned int i = 0; i < allmoves.size(); i++) {    
    if (i % 2 == 0) {pwm = white;}
    else if (i % 2 == 1) {pwm = black;}
    
    validmove = chb.chessmove(pwm, allmoves[i]);
    if (validmove) {
      //correcting turn counter, drawcounter, and player who did the move for a proper autosavegame, it works because ChessSaving is friend of ChessBoard
      chb.drawffcounter++;
      autosavegame(chb, false); //this should be done here, after the increment of drawffcounter and before that of turn

      if (pwm == white) {chb.player_moving = chb.players[1];
        turnlost = false;
      } else if (pwm == black) {
        chb.player_moving = chb.players[0];
        chb.turn++;
        turnlost = true;
      }
      
    } else {status = false; break;}
  }
  
  return status;
}


/* ChessSquare methods and initialization
 */
ChessSquare::ChessSquare() {}

ChessSquare::ChessSquare(c_color c, int a, int b) : CHVector(a, b, true) {
  color = c;
  p = nullptr; //null pointer
}

ChessSquare::ChessSquare(c_color c, Piece* pp) {
  color = c;
  p = pp;
  setvalues(p->getx(), p->gety(), true);
}

ChessSquare::~ChessSquare() {}

//place piece in this ChessSquare, and modify inner piece position to the square coordinates
void ChessSquare::pieceinsquare(Piece* np) {
  p = np;
  p->setvalues(x, y, true);
}


/*ChessPlayer methods and initialiation
 */
ChessPlayer::ChessPlayer() {}

ChessPlayer::ChessPlayer(c_color c) {
  color = c;
}

ChessPlayer::ChessPlayer(c_color c, bool hum) : ChessPlayer(c) {ishuman = hum;}

ChessPlayer::~ChessPlayer() {}

//set the name of the file where writing the game
void ChessPlayer::setsavefn(std::string fn) {
  if (fn.size() > 0) {
    std::size_t resf = fn.find(ChessExts::saveext);
    if (resf == std::string::npos) {savefn = fn + ChessExts::saveext;}
    else {savefn = fn;}
  } else {savefn = "savegame" + ChessExts::saveext;}
}

//set the name of the file where writing the history
void ChessPlayer::sethistoryfn(std::string fn) {
  if (fn.size() > 0) {
    std::size_t resf = fn.find(ChessExts::histext);
    if (resf == std::string::npos) {histfn = fn + ChessExts::histext;}
    else {histfn = fn;}
  } else {histfn = "";}
}

//set the id (text string) of the player
std::string ChessPlayer::whoplay() {
  std::string res;
  if (color == white) {res = "White";}
  else if (color == black) {res = "Black";}
  
  return res;
}


/*ChessBoard methods and initialization
 */
ChessBoard::ChessBoard(std::string fenpos) {
  saver = new ChessSaving();
  saver->readinifen(fenpos);
  construct_board();
  construct_pieces(fenpos);
  temporarysquares = squares;
  saver->initcs(".chess_saving");
  algebnotshort.str("---");
  algebnotlong.str("---");
}

ChessBoard::~ChessBoard() {
  delete saver;
  delete uciwh;
  delete ucibl;
  for (unsigned int i = 0; i < pieces.size(); i++) {delete pieces[i];}
}

//gathering the players
void ChessBoard::gather_players(ChessPlayer* plw, ChessPlayer* plb) {
  players[0] = plw;
  players[1] = plb;
  if (playerstart == white) {player_moving = players[0];}
  else if (playerstart == black) {player_moving = players[1];}
}

void ChessBoard::set_board_for_players() {
  for (unsigned int i = 0; i < players.size(); i++) {
    players[i]->setpointer(this);
    players[i]->set_ptb(this->getptosquares());
  }
}

//check if chess engine should be started and start if if needed
bool ChessBoard::check_engine(ChessUCI* wh, ChessUCI* bl) {
  uciwh = wh;
  ucibl = bl;
  bool res, ares = true, bres = true;
  
  if (! players[0]->isplayerhuman()) {
    ares = start_engine(uciwh);
    players[0]->ucieng = uciwh;
  }
  if (! players[1]->isplayerhuman()) {
    bres = start_engine(ucibl);
    players[1]->ucieng = ucibl;
  }
  
  res = ares && bres; //true only if both are true
  return res;
}

//starting the chess engine
bool ChessBoard::start_engine(ChessUCI* ucidialog) {
  ucidialog->setchspointer(saver);
  ucidialog->init();
  ucidialog->sendcomm(0); //sending uci
  
  bool res = true;
  int cc = 0;
  while ((! ucidialog->checkanswok(1)) && cc < 100) {//checking for uciok
    sleep(0.1);
    ucidialog->geteansw(true);
    cc++;
  }
  
  if (cc == 100) {res = false;}
  if (res) {
    bool readpr = ucidialog->readcepref(); //reading the saved options in the file
    if (! readpr) {
      cbbuf << "No custom preferences for Chess Engine " << ucidialog->getcename() << ", default values used";
      printmess();
    } else {
      ucidialog->applycepref(false); //apply the options (sending the UCI commands to the chess engine)
    }
  }
  return res;
}

//building the pieces
void ChessBoard::construct_pieces(std::string tfenpos) {
  std::string fenpos;
  if (tfenpos.size() > 0) {fenpos = tfenpos;}
  else {fenpos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";} //FEN notation of initial position

  //reading FEN 
  std::istringstream buffenpos(fenpos);
  std::string fenchb, fenrow, wmstr, castlstr, enpasstr, fenturn, fendrawc;
  buffenpos >> fenchb; //extracting chessboard representation from FEN notation
  std::istringstream buffenchb(fenchb);

  //creating pieces 
  int i = 0;
  std::vector<ChessRock*> rockvec;
   
  while (! buffenchb.eof()) {
    std::getline(buffenchb, fenrow, '/');
    int j = 0;
    for (unsigned int k = 0; k < fenrow.size(); k++) {
      std::string elem = fenrow.substr(k, 1);
      
      if (isdigit(elem[0])) {j += elem[0] - '0';} //correct coordinate in case of number indicating empty squares 
      else {//building the piece
        Piece* cpiec;
        ChessFEN::IdPair ptype = ChessFEN::fenidtable.at(elem);
        if (ptype.pi == pawn) {
          ChessPawn* pppawn = new ChessPawn(ptype.col, j, i);
          cpiec = pppawn;
        }
          
        else if (ptype.pi == rock) {
          ChessRock* pprock = new ChessRock(ptype.col, j, i);
          pprock->beenmoved = true; //it is easier to revert it to false when checking the castling possibilities
          cpiec = pprock;
          rockvec.push_back(pprock);
        }
          
        else if (ptype.pi == knight) {
          ChessKnight* ppknight = new ChessKnight(ptype.col, j, i);
          cpiec = ppknight;
        }
          
        else if (ptype.pi == bishop) {
          ChessBishop* ppbishop = new ChessBishop(ptype.col, j, i);
          cpiec = ppbishop;
        }
        
        else if (ptype.pi == queen) {
          ChessQueen* ppqueen = new ChessQueen(ptype.col, j, i);
          cpiec = ppqueen;
        }
        
        else if (ptype.pi == king) {
          ChessKing* ppking = new ChessKing(ptype.col, j, i);
          cpiec = ppking;
          
          //assigning pointers to kings
          if (ptype.col == black) {blking = ppking;}
          else if (ptype.col == white) {whking = ppking;}
        }
        pieces.push_back(cpiec);
        squareinpiece(cpiec);
        j++; //should be incremented only when there is a piece, otherwise is already incremented by the digit in FEN notation by the right quantity
      }
    }
    i++;
  }

  //selecting player who moves
  buffenpos >> wmstr; //extracting moving player from FEN notation
  if (wmstr == "w") {playerstart = white;}
  else if (wmstr == "b") {playerstart = black;}
  else {
    std::cerr << "Error, \"" << wmstr << "\" does not mean anything. This should be \"w\" or \"b\" to indicate who has the move. We will give it to white" << std::endl;
    playerstart = white;
    }
    
  //selecting castling possibilities
  buffenpos >> castlstr; //extracting castling possibilities from FEN notation
  for (unsigned int r = 0; r < rockvec.size(); r++) {
    ChessRock* cro = rockvec[r];
    if (cro->gety() == blking->gety() || cro->getx() > blking->getx()) {
      if (castlstr.find(ChessFEN::kb) != std::string::npos) {cro->beenmoved = false;}
    }
    if (cro->gety() == blking->gety() || cro->getx() < blking->getx()) {
      if (castlstr.find(ChessFEN::qb) != std::string::npos) {cro->beenmoved = false;}
    }
    if (cro->gety() == whking->gety() || cro->getx() > whking->getx()) {
      if (castlstr.find(ChessFEN::kw) != std::string::npos) {cro->beenmoved = false;}
    }
    if (cro->gety() == whking->gety() || cro->getx() < whking->getx()) {
      if (castlstr.find(ChessFEN::qw) != std::string::npos) {cro->beenmoved = false;}
    }
  }
  
  //adding en passant move possibility if present
  buffenpos >> enpasstr; //extracing en passant position
  if (enpasstr != "-") {
    Piece* refpi; ChessPawn* refpawn;
    int epx = ChessCoordinates::xin(enpasstr.substr(0, 1));
    int epy = ChessCoordinates::yin(enpasstr.substr(1, 1));
    
    if (epy == 2) {refpi = getsquare(epx, epy+1)->p;}
    else if (epy == 5) {refpi = getsquare(epx, epy-1)->p;}
    else {std::cerr << "Error, en passant not possible in the indicated square!" << std::endl;}
    refpawn = dynamic_cast<ChessPawn*>(refpi);
    if (refpawn == nullptr) {std::cerr << "Error, the related pawn is absent!" << std::endl;}
    else {
      ChessFakePawn* ppfkpawn = new ChessFakePawn(refpawn->getcolor(), epx, epy, refpawn);
      Piece* ppfpp = ppfkpawn;
      pieces.push_back(ppfpp);
      squareinpiece(ppfpp);
    }
  }
    
  //adjusting turnation counter and 50 moves rule draw counter
  buffenpos >> fendrawc; //extracting draw counter from FEN notation
  buffenpos >> fenturn; //extracting turn from FEN notation
  drawffcounter = std::stoi(fendrawc);  
  turn = std::stoi(fenturn);
}

//building the chessboard (replacing ChessSquares in the matrix with the proper constructor)
void ChessBoard::construct_board() {
  c_color tc;
  for (int i = CHVector::min_x; i < CHVector::max_x; i++) {
    for (int j = CHVector::min_y; j < CHVector::max_y; j++) {
      if ((i+j) % 2 == 0) {tc = white;}
      else {tc = black;}
      squares[i][j] = ChessSquare(tc, i, j);
    }
  }
}

//get a pointer to a ChessSquare from the chessboard
ChessSquare* ChessBoard::getsquare(int a, int b) {
  ChessSquare* res;

  if (a >= CHVector::min_x && a < CHVector::max_x && b >= CHVector::min_y && b < CHVector::max_y) {res = &squares[a][b];}
  else {res = nullptr;}

  return res;
}

//get a pointer to a ChessSquare from the temporary chessboard
ChessSquare* ChessBoard::getsquaretemp(int a, int b) {
  ChessSquare* res;

  if (a >= CHVector::min_x && a < CHVector::max_x && b >= CHVector::min_y && b < CHVector::max_y) {res = &temporarysquares[a][b];}
  else {res = nullptr;}

  return res;
}

//put the piece in the square corresponding to the internal coordinates of the piece
void ChessBoard::squareinpiece(Piece* pp) {
  if (pp->ongame) {
    ChessSquare* sq;
    
    int a = pp->getx();
    int b = pp->gety();
    
    sq = &squares[a][b];
    sq->p = pp;
  }
}

//set the pointer in a ChessSquare to null
void ChessBoard::emptysquare(int dx, int dy) {
  ChessSquare* sqd;
  
  sqd = &squares[dx][dy];
  sqd->p = nullptr;
}

//set if players are human or not, public wrapper to handle the protected array hosting players
void ChessBoard::sethumplayers(std::array<bool, 2> harr) {
  for (unsigned int i = 0; i < harr.size(); i++) {
    players[i]->setifph(harr[i]);
  }
}

//generate FEN representation of the chessboard
std::string ChessBoard::genFEN() {
  std::ostringstream res, epstr, castlstr;
  Piece* ps;
  ChessRock* rockcastl;

  //writing pieces on the chessboard
  for (int i = 0; i < CHVector::max_y; i++) {
    int c = 0;
    for (int j = 0; j < CHVector::max_x; j++) {
      ps = getsquare(j, i)->p;
      
      if (ps == nullptr) {c++;}
      else {
        ChessFEN::IdPair piecetype = ChessFEN::IdPair(ps->getidtype(), ps->getcolor());
        std::string symb = ChessFEN::getkeyofval(piecetype);
        if (symb == "") {
          if (ps->getidtype() == fakepawn) {
            epstr << ChessCoordinates::xout(j) << ChessCoordinates::yout(i); //saving en passant reference
            c++;
          } else {std::cerr << "Error, something wrong when constructing FEN notation" << std::endl;}
        } else {
          if (c > 0) {res << c; c = 0;}
          res << symb;
        }
      }
    }
    if (c > 0) {res << c; c = 0;}
    if (i != CHVector::max_y -1) {res << "/";}
  }
  res << " ";
  
  //adding player who moves
  if (player_moving->wpcolor() == white) {res << "w" << " ";}
  else if (player_moving->wpcolor() == black) {res << "b" << " ";}
  
  //adding castling possibilities
  if (! whking->beenmoved) {
    rockcastl = checkcastlingrock(7, 7);
    if (rockcastl != nullptr) {castlstr << "K";}
    rockcastl = checkcastlingrock(0, 7);
    if (rockcastl != nullptr) {castlstr << "Q";}
  }
  if (! blking->beenmoved) {
    rockcastl = checkcastlingrock(7, 0);
    if (rockcastl != nullptr) {castlstr << "k";}
    rockcastl = checkcastlingrock(0, 0);
    if (rockcastl != nullptr) {castlstr << "q";}
  }
  if (castlstr.str().size() > 0) {res << castlstr.str() << " ";}
  else {res << "- ";}
  
  //adding en passant eating square reference
  if (epstr.str().size() > 0) {res << epstr.str() << " ";}
  else {res << "- ";}
  
  //adding halfmove number from last eating / pawn movement
  res << drawffcounter << " ";
  
  //adding turn number
  res << turn;
  
  return res.str();
}

//move a piece, from-to are inside the ChessPlayer who do the move, wrapper for another chessmove method
bool ChessBoard::chessmove(ChessPlayer* whoismoving) {
  bool rr = chessmove(whoismoving->wpcolor(), whoismoving->move_from, whoismoving->move_to, whoismoving->promoteinto);
  whoismoving->promoteinto = generic;
  return rr;
}

//move a piece, from-to given as coordinates (no need of players), wrapper for another chessmove method
bool ChessBoard::chessmove(c_color cc, int xa, int ya, int xb, int yb) {
  ChessSquare* a = getsquare(xa, ya);
  ChessSquare* b = getsquare(xb, yb);
  if (a == nullptr || b == nullptr) {return false;}
  bool rr = chessmove(cc, a, b);
  return rr;
}

//move a piece, the move is described by a string in short algebraic notation (needed also the color of the moving player), it is a wrapper for another chessmove method
bool ChessBoard::chessmove(c_color cc, std::string strmove) {
  ChessSquare *sqfr, *sqto;
  wpiece mpc;
  wpiece prompiece = generic;
  std::string coordstring, promstr;
  bool res;
  bool iseat = false;
  int oxc = -1;
  int oyc = -1;
  std::size_t apos;
  
  //writing the move
  if (cc == white) {cbbuf << turn << ". " << strmove;}
  else if (cc == black) {cbbuf << turn << "... " << strmove;}
  printmess();
  
  //verify special notation for castling
  if (strmove[0] == 'O') {
    ChessKing* ptok;
    if (cc == white) {ptok = whking;}
    else if (cc == black) {ptok = blking;}
    sqfr = getsquare(ptok->getx(), ptok->gety());
    
    if (strmove == "O-O") {sqto = getsquare(ptok->getx() +2, ptok->gety());}
    else if (strmove == "O-O-O") {sqto = getsquare(ptok->getx() -2, ptok->gety());}
    
    res = chessmove(cc, sqfr, sqto);
    return res;
  }
  
  //finding the type of the moving piece
  try {
    mpc = ChessIdentifiers::arrpid.at(strmove.substr(0, 1));
    coordstring = strmove.substr(1, strmove.size()-1);
  } catch (const std::out_of_range oor) {//because pawn is not in arrpid
    mpc = pawn;
    coordstring = strmove;
  }
  
  //removing special notations of eating piece (x), check (+), checkmate (#) and promotion (= , this is saved in the dedicated string)
  promstr = "";
  apos = coordstring.find('x');
  if (apos != std::string::npos) {iseat = true; coordstring.erase(apos, 1);}

  apos = coordstring.find('+');
  if (apos != std::string::npos) {coordstring.erase(apos, 1);}

  apos = coordstring.find('#');
  if (apos != std::string::npos) {coordstring.erase(apos, 1);}
    
  apos = coordstring.find('=');
  if (apos != std::string::npos) {
    promstr = coordstring.substr(apos+1, 1);
    coordstring.erase(apos, 2);
    prompiece = ChessIdentifiers::arrpid[promstr];
  }

  //also suffix annotations (! !! ? ?? ?! !?), if present, are removed
  apos = coordstring.find("!!");
  if (apos != std::string::npos) {coordstring.erase(apos, 2);}
  
  apos = coordstring.find("!?");
  if (apos != std::string::npos) {coordstring.erase(apos, 2);}
  
  apos = coordstring.find("?!");
  if (apos != std::string::npos) {coordstring.erase(apos, 2);}
  
  apos = coordstring.find("??");
  if (apos != std::string::npos) {coordstring.erase(apos, 2);}
  
  apos = coordstring.find('!');
  if (apos != std::string::npos) {coordstring.erase(apos, 1);}
  
  apos = coordstring.find('?');
  if (apos != std::string::npos) {coordstring.erase(apos, 1);}
  
  if (coordstring.size() == 4) {//both x and y of starting position are indicated
    oxc = ChessCoordinates::xin(coordstring.substr(0, 1));
    oyc = ChessCoordinates::yin(coordstring.substr(1, 1));
    coordstring = coordstring.substr(2, 2);
  }
  
  if (coordstring.size() == 3) {//only x or y of starting position is indicated
    oxc = ChessCoordinates::xin(coordstring.substr(0, 1));
    oyc = ChessCoordinates::yin(coordstring.substr(0, 1));
    coordstring = coordstring.substr(1, 2);
  }
    
  //finally getting coordinates of arrival square
  int xc = ChessCoordinates::xin(coordstring.substr(0, 1));
  int yc = ChessCoordinates::yin(coordstring.substr(1, 1));
  if (xc != -1 && yc != -1) {sqto = getsquare(xc, yc);}
  else {std::cerr << "Error, problem in coordinate conversion of " << coordstring << std::endl; std::exit(EXIT_FAILURE);}

  //retrieving moving piece
  std::vector<Piece*> movpcs, movpcsspec, validpcs;
  if (iseat) {movpcs = ismenacedby(cc, sqto, false, 2);}
  else {
    movpcs = ismenacedby(cc, sqto, false, 1);
    movpcsspec = ismenacedby(cc, sqto, false, 3);
    for (unsigned int i = 0; i < movpcsspec.size(); i++) {
      if (std::find(movpcs.begin(), movpcs.end(), movpcsspec[i]) == movpcs.end()) {//verify if element in movpcsspec is not already present
        movpcs.push_back(movpcsspec[i]);
      }
    }
  }

  Piece* pp;
  temporarysquares = squares;
  for (unsigned int i = 0; i < movpcs.size(); i++) {
    pp = movpcs[i];
    if (pp->getidtype() == mpc) {
      //excluding pieces that cannot legally move (for example, because they leave their king in check)
      sqfr = getsquare(pp->getx(), pp->gety());
      sqfr->p = nullptr; sqto->p = pp;
      bool ucheck;
      if (pp->getcolor() == white) {ucheck = isincheck(whking);}
      else if (pp->getcolor() == black) {ucheck = isincheck(blking);}
      squares = temporarysquares; //restoring the square after the test
      if (!ucheck) {validpcs.push_back(pp);}
    }
  }
  
  if (validpcs.size() == 0) { }
  else if (validpcs.size() == 1) {sqfr = getsquare(validpcs[0]->getx(), validpcs[0]->gety());}
  else if (validpcs.size() > 1) {
    for (unsigned int i = 0; i < validpcs.size(); i++) {
      if (oxc == -1 && oyc == -1) {std::cerr << "Error in chessmove with algebraic notation: cannot be more than one piece that can legally move in the arrival square without info on coordinates of moving piece!" << std::endl;}
      if (validpcs[i]->getx() == oxc || validpcs[i]->gety() == oyc) {sqfr = getsquare(validpcs[i]->getx(), validpcs[i]->gety()); break;}
    }
  }
  
  //std::cout << strmove << " from: " << ChessCoordinates::xout(sqfr->getx()) << ChessCoordinates::yout(sqfr->gety()) << " to: " << ChessCoordinates::xout(sqto->getx()) << ChessCoordinates::yout(sqto->gety()) << std::endl;
  res = chessmove(cc, sqfr, sqto, prompiece);
  return res;
}

//move a piece, from-to gives as pointers to ChessSquares objects (do not care from where these pointers came)
//the method perform also castling and pawn promotion, and check if the king is in check after the move (invalidating the move in this case)
bool ChessBoard::chessmove(c_color cc, ChessSquare* froms, ChessSquare* tos, wpiece prompiece) {
  Piece* movingpiece = froms->p;
  bool okmove, movedone, iseating;
  ChessKing *ptoking, *potherking;
  std::stringstream storepos;
  
  iseating = false;
  okmove = check_rule_move(cc, froms, tos);
  
  if (okmove) {
    save_cbimage();
    
    if (movingpiece->getidtype() == pawn) {drawffcounter = -1;} //resetting counter for drawing after a pawn is moved.
    
    if (tos->p != nullptr) {
      iseating = true;
      tos->p->ongame = false; //select flag: the piece previously in the arrival square is eated
      drawffcounter = -1; //resetting counter for drawing after an eating (to -1 in order to compensate for the automatic increment)
      if (tos->p->getidtype() == fakepawn && movingpiece->getidtype() == pawn) {//handling en passant eating
        ChessFakePawn* fakepeated = dynamic_cast<ChessFakePawn*>(tos->p);
        enpassanteating(fakepeated);
      }
    }
    writealgnot(movingpiece, tos, iseating); //the algebraic notation
    
    tos->pieceinsquare(movingpiece); //moving the piece: setting the pointer in the arrival square to the piece and modifying internal coordinates
    froms->p = nullptr; //setting the pointer in the starting square to null
    
    c_color adv = !cc;
    if (cc == white) {
      ptoking = whking;
      potherking = blking;
    } else if (cc == black) {
      ptoking = blking;
      potherking = whking;
    }
        
    bool kic = isincheck(ptoking); //verify if the king of the player who has just done the move is in check
    
    //check castling condition if requested. If the move was the castling, further checks are needed to validate and concluded it
    bool okcastling = true;
    ChessRock* rockcastling;
    
    if (reqcastling) {
      if (ptoking != movingpiece) {
        std::cerr << "Error! Castling requested whitout moving the King? Something wrong!" << std::endl;
        std::exit(EXIT_FAILURE);
      }
      
      //checking if king has been moved
      if (ptoking->beenmoved) {
        cbbuf << "You already moved the king.\n"; 
        okcastling = false;
      }
      
      //getting the king position before the move, the square crossed by the king, the squares between the king and the correct rock
      std::array<ChessSquare*, 2> movingking;
      std::array<ChessSquare*, 3> sqkingtorock;
      int ia, ja, jb, jc, jd;
      unsigned int limktr;
      
      ia = froms->getx();
      ja = froms->gety();
      
      if (lastmove.getx() == +2) {jb = +1; jc = 7, jd = 5, limktr = 2;}
      else if (lastmove.getx() == -2) {jb = -1; jc = 0, jd = 1; limktr = 3;}
        
      movingking[0] = &squares[ia][ja];  //starting position of the king
      movingking[1] = &squares[ia+jb][ja]; //middle square of the moving
      for (unsigned int l = 0; l < 3; l++) {sqkingtorock[l] = &squares[jd+l][ja];}
      rockcastling = checkcastlingrock(jc, ja);
      if (rockcastling == nullptr) {okcastling = false;}
      
      //checking if king is in check in the whole move
      std::vector<Piece*> whomka, whomkb;
      whomka = ismenacedby(adv, movingking[0], false, 2);
      whomkb = ismenacedby(adv, movingking[1], false, 2);
      if ((whomka.size() > 0 || whomkb.size() > 0) || kic) {
        cbbuf << "The King will be in check during or at the end of the move.\n";
        okcastling = false;
      }
      
      //checking if squares between the rock and the king are empty
      for (unsigned int m = 0; m < limktr; m++) {
        if (sqkingtorock[m]->p != nullptr && sqkingtorock[m]->p != ptoking) {
          cbbuf << "The squares between the King and the Rock are not all empty.\n";
          okcastling = false;
          break;
        }
      }
      
      //do the castling
      if (okcastling) {docastling(rockcastling);} //perform castling (move the rock)
      else {
        cbbuf << "Castling not possible!";
        printmess(true);
      }
    }
    
    //now checking if the move is valid (some checks are easy to do after the move)
    bool aftermoveok;
        
    if (reqcastling) {aftermoveok = okcastling;} //check if castling condition are satisfied
    else {aftermoveok = !kic;} //check only if the player's king is in check at the end of the move
        
    if (aftermoveok) {
      movedone = true;
      tos->p->beenmoved = true; //set the flag for special move: all special moves are possible only during the first move of the piece.
      
      //further checks for the pawn
      if (movingpiece->getidtype() == pawn) {
        ChessPawn* ppawn = dynamic_cast<ChessPawn*> (movingpiece);
        bool dopromo = ppawn->promoteme(); //check pawn promotion
        
        if (dopromo) {
          wpiece selpiece;
          if (prompiece == generic) {selpiece = choose_promotion(ppawn->getcolor());}
          else {selpiece = prompiece;}
          Piece* ppiece = promotepawn(ppawn, selpiece);
          algebnotshort << "=" << ppiece->getnalg(); //adding promotion symbol in short algebraic notation
          algebnotlong << "=" << ppiece->getnalg(); //adding promotion symbol in long algebraic notation
        }
        
        if (placefakepawn) {//place the fake pawn for the en passant eating
          ChessFakePawn* ffpp = new ChessFakePawn(ppawn);
          ChessSquare* ffppsquare = getsquare(ffpp->getx(), ffpp->gety());
          ffppsquare->pieceinsquare(ffpp);
          pieces.push_back(ffpp);
        }
      }
      
      //verifying if is checkmate for other king
      bool chma = ischeckmate(potherking);
      if (chma) {
        //assign a winner in the dedicated variable
        if (ptoking->getcolor() == white) {finalres = whitewins;}
        else if (ptoking->getcolor() == black) {finalres = blackwins;}
        algebnotshort << "#"; //add symbol for checkmate to the move in the short algebraic notation
        algebnotlong << "#"; //add symbol for checkmate to the move in the long algebraic notation
        
      } else {
        finalres = notfinished;
        
        //verifying if other king is in check
        bool otkic = isincheck(potherking);
        if (otkic) {
          if (potherking->getcolor() == white) {cbbuf << "White";}
          else if (potherking->getcolor() == black) {cbbuf << "Black";}
          cbbuf << " King is in check!";
          printmess();
          algebnotshort << "+"; //add symbol for check to the move in the short algebraic notation
          algebnotlong << "+"; //add symbol for check to the move in the long algebraic notation
        }
      }
      
      //removing fake pawns if present
      if (ChessFakePawn::getcounter() > 0) {removefakes(movingpiece->getcolor());}
      
    } else {
      restore_cbimage();
      movedone = false;
      
      if (!reqcastling) {
        ChessSquare* sqwk = getsquare(ptoking->getx(), ptoking->gety());
        std::vector<Piece*> mm = ismenacedby(adv, sqwk);
        
        for (unsigned int i = 0; i < mm.size(); i++) { 
          storepos << mm[i]->getidtxt() << " (" << ChessCoordinates::xout(mm[i]->getx()) << " " << ChessCoordinates::yout(mm[i]->gety()) << ")";
          if (i != mm.size()-1) {storepos << "\n";}
        }
        cbbuf << "You cannot do that, your King is or will be in check by " << storepos.str();
        printmess(true);
      }
    }
    
  } else {
    cbbuf << "Move not possible.";
    printmess();
    movedone = false;
  }

  reqcastling = false;
  placefakepawn = false;
  return movedone;
}

//checking if the rock is available for castling, in input coordinates of the rock, get a valid pointer only if castling condition for the rock are satisfied, othervise is null 
ChessRock* ChessBoard::checkcastlingrock(int x, int y) {
  Piece* pcorner = squares[x][y].p;
  ChessRock* res = nullptr;
  bool writemess = false;
  
  //checking if rock has been moved
  if (pcorner == nullptr) {writemess = true;}
  else {
    if (pcorner->getidtype() == rock && (! pcorner->beenmoved)) {res = dynamic_cast<ChessRock*>(pcorner);}
    else {writemess = true;}
  }

  if (writemess) {cbbuf << "The Rock has been moved.\n";}
  return res;
}

//verifying if the requested move is valid
bool ChessBoard::check_rule_move(c_color currc, ChessSquare* fs, ChessSquare* ps) {
  bool precheck = check_starting(currc, fs);
  if (!precheck) {return false;}
   
  Piece* mp = fs->p;
  
  bool res;
  int whatact;

  //checking if another piece is in the arrival square
  Piece* arrp = ps->p;
  if (arrp == nullptr) {
    whatact = 1; //is a move
  } else {
    whatact = 2; //is an eating
    if (mp->getcolor() == arrp->getcolor()) {
      cbbuf << "Error, you cannot eat your own piece!";
      printmess();
      return false;
    }
  }

  mp->assign_methods(whatact); //assigning methods of piece for the proper action (move, eating)
  res = process_move(mp, ps);

  //checking for special moves is no move / eating has been performed and special moves are still possible.
  if ((!res) && (!mp->beenmoved)) {
    mp->assign_methods(3);
    res = process_move(mp, ps);
    
    if (mp->getidtype() == king) {
      //set if a castling has been requested
      reqcastling = true;
    } else if (mp->getidtype() == pawn) {
      //set for fakepawn for en passant eating
      placefakepawn = true;
    }
  }
  mp->assign_methods(0); //deassigning methods of piece
  
  return res;
}

//verifying if the requested move is valid, first part
bool ChessBoard::check_starting(c_color currc, ChessSquare* stsq) {
  Piece* mp = stsq->p;
  
  //verifying that there is a piece in the starting square
  if (mp == nullptr) {
    cbbuf << "Error, no piece to move in " << ChessCoordinates::xout(stsq->getx()) << ChessCoordinates::yout(stsq->gety()) << " square.";
    printmess();
    return false;
  }

  if (mp->getcolor() != currc) {
    cbbuf << "Error, moving a wrong piece.";
    printmess();
    return false;
  }
  
  if (! (*mp == *stsq)) {//verifying that the coordinates of ChessSquare and the pointed piece are consistent
    std::cerr << "ERROR, something wrong: piece position not corresponding to square position!" << std::endl;
    std::exit(EXIT_FAILURE);
  }
  
  return true;
}

//verifying that the piece can reach che arrival square
bool ChessBoard::process_move(Piece* pp, ChessSquare* ts) {
  CHVector comp;
  ChessSquare* pcs;

  for (unsigned int i = 0; i < pp->wrdirection(); i++) {
    for (unsigned int j = 0; j < pp->wrsteps(i); j++) {
      comp = *pp + pp->getaction(i, j);
      
      if (comp.isinvalid()) {break;} // break the inner loop, we are outside the chessboard
      else {
        pcs = getsquare(comp.getx(), comp.gety());
        
        if (pcs->p == nullptr && *pcs == *ts) {
          lastmove = pp->getaction(i, j);
          return true;
        }
      
        if (pcs->p != nullptr) {
          if (*pcs == *ts) {
            lastmove = pp->getaction(i, j);
            return true;
          }
          
          if (pcs->p->getidtype() != fakepawn) {break;} //break inner loop because a piece is on the path of the move
        }
      }
    }
  }
  
  return false;
}

//check if the Player is in check, wrapper for another isincheck function
bool ChessBoard::isincheck(ChessPlayer* cpl) {
  ChessKing* ck;
  //selecting the king
  if (cpl->wpcolor() == white) {ck = whking;}
  else if (cpl->wpcolor() == black) {ck = blking;}

  bool res = isincheck(ck);
  return res;
}

//check if a ChessKing piece is in check
bool ChessBoard::isincheck(ChessKing* ck) {
  bool res = false;
  c_color kcol = ck->getcolor();
  ChessSquare* kingpos = getsquare(ck->getx(), ck->gety());
  std::vector<Piece*> ucheck = ismenacedby(!kcol, kingpos);
  if (ucheck.size() > 0) {res = true;}
  return res;
}

//check if it is checkmate for a player, wrapper for the other ischeckmate function
bool ChessBoard::ischeckmate(ChessPlayer* cpl) {
  ChessKing* ck;
  c_color kcol;
  
  //selecting the king
  kcol = cpl->wpcolor();
  if (kcol == white) {ck = whking;}
  else if (kcol == black) {ck = blking;}
  
  bool res = ischeckmate(ck);
  return res;
}

//check if it is checkmate for a ChessKing piece
bool ChessBoard::ischeckmate(ChessKing* ck) {
  c_color kcol, adv;
    
  //selecting the king
  kcol = ck->getcolor();
  adv = !kcol;
    
  //check if the king is at least in check and return false if not (we do not use the dedicated function because we need to keep the vector with the pieces menacing the king)
  ChessSquare* kingpos = getsquare(ck->getx(), ck->gety());
  std::vector<Piece*> menk = ismenacedby(adv, kingpos);
  if (menk.size() == 0) {return false;}

  //check if squares where the king can move are menaced
  std::vector<ChessSquare*> kingsm = kinglegalmoves(ck);
  if (kingsm.size() > 0) {return false;}
  
  //check if the piece menacing the king can be eated or blocked.
  bool trajfound = false;
  CHVector comp;
  ChessSquare *pcs, *squareck;
  Piece *menacingk;
  std::vector<ChessSquare*> trajectory;
  std::vector<Piece*> menacingset, menacingsetbis;

  if (menk.size() == 1) { //if it is > 1, is checkmate: with a single move, two different pieces cannot be eated or blocked
    temporarysquares = squares; //copying the chessboard in the temporary chessboard
    menacingk = menk[0];
    squareck = getsquare(menacingk->getx(), menacingk->gety());
    
    //to check if the piece menacing the king can be eated
    menacingset = ismenacedby(kcol, squareck);
    for (unsigned int i = 0; i < menacingset.size(); i++) {
      if (*(menacingset[i]) == *ck) {
        menacingsetbis = ismenacedby(adv, squareck);
        if (menacingsetbis.size() == 0) {return false;}
      } else {
        //forcing the eating
        menacingk->ongame = false;
        pcs = getsquare(menacingset[i]->getx(), menacingset[i]->gety());
        squareck->p = menacingset[i]; pcs->p = nullptr;        
        bool stillcheck = isincheck(ck); //king is not forcefully moved, it is safe to check using this function
        squares = temporarysquares; //restoring the chessboard
        menacingk->ongame = true; //replacing the eated piece on the chessboard
        if (! stillcheck) {return false;}
      }
    }
    
    //extracting the trajectory: the squares crossed by the menacing piece
    menacingk->assign_methods(2);
    for (unsigned int i = 0; i < menacingk->wrdirection(); i++) {
      trajectory.clear();
      for (unsigned int j = 0; j < menacingk->wrsteps(i); j++) {
        comp = *menacingk + menacingk->getaction(i, j);
        
        if (comp.isinvalid()) {
          break;
        } else {
          pcs = getsquare(comp.getx(), comp.gety());
          if (*pcs != *ck) {trajectory.push_back(pcs);}
          else {
            trajfound = true;
            break;
          }
        }
      }
      if (trajfound) {break;}
    }
    menacingk->assign_methods(0);
    
    //exploring if the trajectory is menaced by other pieces of the player (if so, the menacing piece can be blocked);
    //trajectory should be menaced by moves and not eatings
    for (unsigned int i = 0; i < trajectory.size(); i++) {
      menacingset.clear();
      menacingset = ismenacedby(kcol, trajectory[i], true, 1);

      for (unsigned int j = 0; j < menacingset.size(); j++) {
        if (*(menacingset[j]) != *ck) {
          //checking if the piece which should be moved can be moved
          pcs = getsquare(menacingset[j]->getx(), menacingset[j]->gety());
          trajectory[i]->p = menacingset[j]; pcs->p = nullptr;
          bool stillcheck = isincheck(ck);
          squares = temporarysquares; //restoring the chessboard
          if (! stillcheck) {return false;}
        }
      }
    }
  }

  //if the method arrives here, nothing can be done: it is checkmate
  return true;
}

//check if draw conditions are matched and propose draw
bool ChessBoard::isdraw(ChessPlayer* plmov) {
  bool drawdone, drawfifty, drawthree, drawstale, onlykings;
  
  //rule of 50 moves without eating (just verify the value of the counter)
  if (drawffcounter == 50) {drawfifty = true;}
  else {drawfifty = false;}
  
  //rule of the 3 repeated moves
  drawthree = saver->drawforthree();
  
  //stalemate
  drawstale = isstalemate(plmov);

  //not a rule, but only the kings on the chessboard is also a draw
  onlykings = true;
  for (unsigned int i = 0; i < pieces.size(); i++) {
    if (pieces[i]->getidtype() != king && pieces[i]->ongame == true) {
      onlykings = false;
      break;
    }
  }
  
  if (drawfifty) {drawdone = askdraw(0);}
  else if (drawthree) {drawdone = askdraw(1);}
  else if (drawstale) {drawdone = askdraw(2);}
  
  if (onlykings) {drawdone = askdraw(3);}

  if (drawdone) {finalres = tie;} //saving final status of the game for writing
  else if (drawfifty) {drawffcounter = -1;} //reset drawing counter
  
  return drawdone;
}

//check if is stalemate for a player
bool ChessBoard::isstalemate(ChessPlayer* plm) {
  c_color kcol, adv;
  
  //selecting the king
  ChessKing* pk;
  kcol = plm->wpcolor();
  adv = !kcol;
  if (kcol == white) {pk = whking;}
  else if (kcol == black) {pk = blking;}
  
  //checking if king is in check
  bool kinc = isincheck(pk);
  if (kinc) {return false;} //if king is in check, it cannot be stalemate
  
  //checking if king has legal moves
  std::vector<ChessSquare*> kvm = kinglegalmoves(pk);
  if (kvm.size() > 0) {return false;} //there are legal moves for the king, cannot be stalemate
  
  //checking other pieces on the board
  for (unsigned int i = 0; i < pieces.size(); i++) {
    Piece* pp = pieces[i];
    if (pp->getcolor() == kcol && pp->ongame) {
      if (pp->getidtype() == pawn) {
        //checking if the pawn can move forward
        pp->assign_methods(1);
        std::vector<ChessSquare*> pawmov = exploremoves(pp);
        pp->assign_methods(0);
        for (unsigned int j = 0; j < pawmov.size(); j++) {
          if (pawmov[j]->p == nullptr) {return false;} //at least a pawn can move
        }
        
        //checking if the pawn can eat
        pp->assign_methods(2);
        std::vector<ChessSquare*> paweat = exploremoves(pp);
        pp->assign_methods(0);
        for (unsigned int j = 0; j < paweat.size(); j++) {
          if (paweat[j]->p != nullptr) {
            if (paweat[j]->p->getcolor() == adv) {return false;} //at least a pawn can eat
          }
        }
        
      } else if (pp->getidtype() != king) {return false;} //we have pieces that are not pawns other than the king
    }
  }
  
  //if the method arrives here, it is stalemate
  return true;
}

//save an "image" of the chessboard (only the pointers to pieces)
void ChessBoard::save_cbimage() {
  for (int i = CHVector::min_x; i < CHVector::max_x; i++) {
    for (int j = CHVector::min_y; j < CHVector::max_y; j++) {
      squareimage[i][j] = squares[i][j].p;
    }
  }
  tempdfc = drawffcounter;
}

//restore the chessboard to the "image"
void ChessBoard::restore_cbimage() {
  for (int i = CHVector::min_x; i < CHVector::max_x; i++) {
    for (int j = CHVector::min_y; j < CHVector::max_y; j++) {
      squares[i][j].p = squareimage[i][j];
      if (squares[i][j].p != nullptr) {
        squares[i][j].pieceinsquare(squares[i][j].p);
        squares[i][j].p->ongame = true; //replacing piece on the chessboard (the appropriate flag)
      }
    }
  }
  drawffcounter = tempdfc; //set back the drawing counter
}

//explore the possible moves
std::vector<ChessSquare*> ChessBoard::exploremoves(Piece* pp, bool uset) {  
  std::vector<ChessSquare*> res;
  CHVector comp;
  ChessSquare* pcs;
  
  for (unsigned int i = 0; i < pp->wrdirection(); i++) {
    for (unsigned int j = 0; j < pp->wrsteps(i); j++) {
      comp = *pp + pp->getaction(i, j);
      
      if (comp.isinvalid()) {
        break;
      } else {
        if (uset) {pcs = getsquaretemp(comp.getx(), comp.gety());}
        else {pcs = getsquare(comp.getx(), comp.gety());}
        res.push_back(pcs);
        if (pcs->p != nullptr) {
          if (pcs->p->getidtype() != fakepawn) {break;}
        }
      }
    }
  }
  return res;
}

//check who a piece is menacing
std::vector<ChessSquare*> ChessBoard::ismenacing(Piece* tp, int am, bool uset) {
  if (am != 1 && am != 2 && am != 3) {
    std::cerr << "Error! Something wrong with the selection method for moving a piece! The integer is " << am << std::endl;
    std::exit(EXIT_FAILURE);
  }

  tp->assign_methods(am);
  std::vector<ChessSquare*> menaced = exploremoves(tp, uset);
  tp->assign_methods(0);
  return menaced;
}

//explore all combination of moves, eatings and special moves
std::vector<ChessSquare*> ChessBoard::exploremoveats(Piece* movp) {
  std::vector<ChessSquare*> sqtbsgen, sqtbsm, sqtbse, sqtbss;
  
  sqtbsm = ismenacing(movp, 1, false);
  for (unsigned int i = 0; i < sqtbsm.size(); i++) {
    if (sqtbsm[i]->p == nullptr) {sqtbsgen.push_back(sqtbsm[i]);}
  }

  sqtbse = ismenacing(movp, 2, false);
  for (unsigned int i = 0; i < sqtbse.size(); i++) {
    if (sqtbse[i]->p != nullptr) {
      if (sqtbse[i]->p->getcolor() != player_moving->wpcolor()) {sqtbsgen.push_back(sqtbse[i]);}
    }
  }
  
  if (! movp->beenmoved) {
    sqtbss = ismenacing(movp, 3, false);
    for (unsigned int i = 0; i < sqtbss.size(); i++) {
      if (sqtbss[i]->p == nullptr) {sqtbsgen.push_back(sqtbss[i]);}
    }
  }
  
  return sqtbsgen;
}

//explore king's legal moves (squares not menaced)
std::vector<ChessSquare*> ChessBoard::kinglegalmoves(ChessKing* pki) {
  //selecting colors
  c_color kcol, adv;
  kcol = pki->getcolor();
  adv = !kcol;
    
  //copying the chessboard in the temporary chessboard
  temporarysquares = squares;
  
  //check if the king is at least menaced and return false if not (we do not use the dedicated function because we need to keep the vector with the pieces menacing the king)
  ChessSquare* tempkingpos = getsquaretemp(pki->getx(), pki->gety());
  
  //check if squares where the king can move are menaced ** from here working on the temporary copy of the chessboard
  //extracting the squares where the king can move
  std::vector<ChessSquare*> kingtogo, kingcango, kingsurego;
  pki->assign_methods(1); //move is used, but is not relevant here if it is a move or an eating, because king moves and eats in the same way
  kingtogo = exploremoves(pki, true);
  pki->assign_methods(0);
  
  //removing from the squares where the king can move those occupied by other pieces of the same player (cannot be eated)
  ChessSquare* squareck;
  for (unsigned int i = 0; i < kingtogo.size(); i++) {
    squareck = kingtogo[i];
    if (squareck->p == nullptr) {kingcango.push_back(squareck);}
    else {
      if (squareck->p->getcolor() == adv) {kingcango.push_back(squareck);}
    }
  }
  
  //check if the valid squares are menaced
  tempkingpos->p = nullptr; //removing the king from the temporary chessboard
  std::vector<Piece*> menacingset;
  for (unsigned int i = 0; i < kingcango.size(); i++) {
    squareck = kingcango[i];
    
    menacingset = ismenacedby(adv, squareck, true, 2);
    if (menacingset.size() == 0) {kingsurego.push_back(squareck);}
    menacingset.clear(); 
  }
  
  tempkingpos->p = pki; //putting back the king in the temporary chessboard
  return kingsurego;
}

/* check by who a square is menaced (provided the color of the menacing player)
 * the integer selmet sets if should be considered menaced by moving (for empty squares) or eating (occupied squares) or special moving, 
 * following the same notation of assing_method of piece
 * no automatic assignment is done for special move, is not needed. To explore for special moves, it should be selected by assigning selmet = 3 
 */
std::vector<Piece*> ChessBoard::ismenacedby(c_color adv, ChessSquare* tsq, bool usetemp, int selmet) {
  int imet;
  if (selmet == -1) {
    if (tsq->p != nullptr) {
      if (tsq->p->getidtype() != fakepawn) {imet = 2;} //occupied square
      else {imet = 1;} //empty square
    }
    else {imet = 1;} //empty square
  } else {
    imet = selmet;
  }
    
  Piece* evalpiece;
  std::vector<ChessSquare*> menacedset;
  std::vector<Piece*> res;
  
  for (unsigned int i = 0; i < pieces.size(); i++) {
    evalpiece = pieces[i];
    if (evalpiece->getcolor() == adv && evalpiece->ongame && evalpiece->getidtype() != fakepawn) {menacedset = ismenacing(evalpiece, imet, usetemp);}

    for (unsigned int j = 0; j < menacedset.size(); j++) {
      if (*(menacedset[j]) == *tsq) {res.push_back(evalpiece);}
    }
    menacedset.clear();
  }
  return res;
}

//move the rock in the castling
void ChessBoard::docastling(ChessRock* rockm) {
  ChessSquare* start;
  ChessSquare* arrival;

  int x = rockm->getx();
  int y = rockm->gety();

  start = getsquare(x, y);
  
  if (x == 0 && y == 0) {arrival = &squares[3][0];}
  else if (x == 0 && y == 7) {arrival = &squares[3][7];}
  else if (x == 7 && y == 0) {arrival = &squares[5][0];}
  else if (x == 7 && y == 7) {arrival = &squares[5][7];}
  else {std::cerr << "Castling rock not in the supposed position? Something wrong!" << std::endl; std::exit(EXIT_FAILURE);}
  
  arrival->pieceinsquare(rockm);
  start->p = nullptr;
  
  cbbuf << "Castling!";
  printmess();
}

//promote a pawn, substituting it with a new piece
Piece* ChessBoard::promotepawn(ChessPawn* pp, wpiece wp) {
  int cx = pp->getx();
  int cy = pp->gety();
  ChessSquare* promsquare = getsquare(cx, cy);
  
  c_color tcol = pp->getcolor();
  
  pp->ongame = false;
  Piece* newpiece;
  
  if (wp == rock) {newpiece = new ChessRock(tcol, cx, cy);}
  else if (wp == knight) {newpiece = new ChessKnight(tcol, cx, cy);}
  else if (wp == bishop) {newpiece = new ChessBishop(tcol, cx, cy);}
  else if (wp == queen) {newpiece = new ChessQueen(tcol, cx, cy);}
  else {
    std::cerr << "Error, something wrong with the pawn promotion system!" << std::endl;
    std::exit(EXIT_FAILURE);
  }
  
  //adding the new piece to the stack of pieces
  pieces.push_back(newpiece);
  
  //place the new piece in the square of the pawn
  promsquare->pieceinsquare(newpiece);
  
  cbbuf << "Pawn promoted!";
  printmess();
  
  return newpiece;
}

//removing fakepawns of the opposite color
void ChessBoard::removefakes(c_color cc) {
  c_color rmc = !cc;
  Piece *atp, *btp;
  
  //removing fakepawns from the chessboard (pointer to 0)
  for (int i = CHVector::min_x; i < CHVector::max_x; i++) {
    for (int j = CHVector::min_y; j < CHVector::max_y; j++) {

      if (squares[i][j].p != nullptr) {
        atp = squares[i][j].p;
        
        if (atp->getidtype() == fakepawn && atp->getcolor() == rmc) {
          squares[i][j].p = nullptr;
        }
      }
    }
  }
  
  //removing fakepawns from the vector of pieces and delete them from the computer memory 
  for (unsigned int k = 0; k < pieces.size(); k++) {
    btp = pieces[k];
    if (btp->getidtype() == fakepawn && btp->getcolor() == rmc) {
      pieces.erase(pieces.begin() + k);
      delete btp;
    }
  }
}

//perform en passant eating (removing eated pawn)
void ChessBoard::enpassanteating(ChessFakePawn* fkp) {
  ChessPawn* epeated = fkp->getrefp();
  epeated->ongame = false;
  ChessSquare* holdeated = getsquare(epeated->getx(), epeated->gety());
  holdeated->p = nullptr;
}

//going back n moves, return false if no further back action is possible; if n is negative, it goes back until no further back is possible
bool ChessBoard::goback(int n) {
  bool res = true;
  int e;
  if (n > 0) {e = n;} else {e = -1 * n;}
  
  for (int i = 0; i < e; i++) {
    if (saver->lineiter != saver->getbegin()) {
      saver->lineiter--;
    } else {
      res = false;
      break;
    }
    if (n < 0) {e++;}
  }
  std::string mess = saver->loadstatus(*this);
  cbbuf << mess;
  printmess();
  ChessUCI::forcestopp = true; //if we go back even one move, pondering must be prevented 
  return res;
}

//going forward n moves, return false if no further forward action is possible; if n is negative, it goes forward until no further back is possible
bool ChessBoard::goforward(int n) {
  bool res = true;
  int e;
  if (n > 0) {e = n;} else {e = -1 * n;}

  for (int i = 0; i < e; i++) {
    saver->lineiter++;
    if (saver->lineiter == saver->getend()) {
      saver->lineiter--;
      res = false;
      ChessUCI::forcestopp = false; //if we come back at the last move after going back
      break;
    }
    if (n < 0) {e++;}
  }
  std::string mess = saver->loadstatus(*this);
  cbbuf << mess;
  printmess();
  return res;
}

//public wrapper for ChessSaving loadgame function, plus check on the filename
bool ChessBoard::wrploadgame(ChessPGN::pgnmoves themoves) {
  //just verify that the file has the correct extension
  //~ std::size_t resf = lfn.find(ChessExts::saveext);
  //~ if (resf == std::string::npos) {return false;}
  
  bool status = saver->loadgamepgn(*this, themoves);
  return status;
}

//print resign message
void ChessBoard::resignmess(void) {
  std::string winner;
  if (player_moving->wpcolor() == white) {winner = "Black";}
  else if (player_moving->wpcolor() == black) {winner = "White";}
  cbbuf << player_moving->whoplay() << " has resigned!\n" << winner << " wins!";
  printmess(true);
}

//write the algebraic notation of the move and store it in the dedicated attribute
void ChessBoard::writealgnot(Piece* mvp, ChessSquare* arrsq, bool iseating) {
  algebnotshort.str("");
  algebnotlong.str("");
  algebnotshort << mvp->getnalg(); //add symbol of moving piece only in short notation
  //algebnotlong << mvp->getnalg();
    
  //for short notation, search if a coordinate of the moving piece should be added to avoid ambiguity
  Piece* altp;
  std::vector<Piece*> menset;
  if (iseating) {menset = ismenacedby(mvp->getcolor(), arrsq, false, 2);}
  else {menset = ismenacedby(mvp->getcolor(), arrsq, false, 1);}
  
  temporarysquares = squares;
  for (unsigned int i = 0; i < menset.size(); i++) {
    altp = menset[i];
    if (altp->getcolor() == mvp->getcolor() && altp->getidtype() == mvp->getidtype() && altp->getidtype() != pawn && altp->getnumid() != mvp->getnumid()) {
      //checking that the alternative piece can legally move (not leaving the king in check)
      ChessSquare* stasq = getsquare(altp->getx(), altp->gety());
      stasq->p = nullptr; arrsq->p = altp; //moving the piece
      bool ucheck;
      if (altp->getcolor() == white) {ucheck = isincheck(whking);}
      else if (altp->getcolor() == black) {ucheck = isincheck(blking);}
      squares = temporarysquares; //restoring the square after the test
      if (!ucheck) {
        if (altp->getx() != mvp->getx()) {algebnotshort << ChessCoordinates::xout(mvp->getx());}
        else if (altp->gety() != mvp->gety()) {algebnotshort << ChessCoordinates::yout(mvp->gety());}
        break; //needed to consider unrealistic cases where the ambiguity should be resolved between 3 or more pieces, when we found 2, we are satisfied (otherwise, the disambiguity letter is repeated)
      }
    }
  }

  //for long notation, add the coordinate of the moving piece and add -
  algebnotlong << ChessCoordinates::xout(mvp->getx()) << ChessCoordinates::yout(mvp->gety());

  if (iseating) {
    if (mvp->getidtype() == pawn) {algebnotshort << ChessCoordinates::xout(mvp->getx());}
    algebnotshort << "x";
    algebnotlong << "x";
  } else {
    algebnotlong << "-"; //separator between the squares for the long notation (substituted by 'x' if the piece is eating
  }

  algebnotshort << ChessCoordinates::xout(arrsq->getx()) << ChessCoordinates::yout(arrsq->gety());
  algebnotlong << ChessCoordinates::xout(arrsq->getx()) << ChessCoordinates::yout(arrsq->gety());

  /* NOTE: symbol for opposite king in check and promoted pawn if needed is not added here but later in the body of chess_move
   * algebnotshort and algebnotlong strings will be written in the saving file by saver->autosavegame
   */
}

//interacting with the chess engine to set here the move (starting and final chessquare of ChessPlayer) according to the response of the engine
bool ChessBoard::engine_act(ChessPlayer* plm, ChessUCI* ucidialog, bool ponderphase) {
  if (! ponderphase) {
    if (ucidialog->getusefen()) {
      std::string fenrep = genFEN();
      ucidialog->setfenchb(fenrep);
    }
    if (ucidialog->getpondering() && (! ucidialog->getusefen())) {//if FEN is used in position command, it is impossible to ponder 
      std::string hh = ucidialog->getprevmoves(); //get formatted history
      std::stringstream bufhh(hh);
      std::string mm;
      while (! bufhh.eof()) {bufhh >> mm;} //take last move done
             
      if (ucidialog->getpondermove() == mm && (turn != 1) && (! ChessUCI::forcestopp)) {//excluding the first move, nothing to ponder on the first move and check for a force stop
        ucidialog->sendcomm(9);//sending ponderhit
      } else {
        if (turn != 1) {
          ucidialog->sendcomm(8); //stop the current search in pondering mode
          while (! ucidialog->checkanswok(3)) {sleep(0.1); ucidialog->geteansw();}
        }
        ucidialog->setpondering(false); //as the user played a different move from the pondered one, in this phase the engine should not ponder but search from zero a new move
        ucidialog->sendcomm(6); //sending position
        ucidialog->sendcomm(7); //sending go
        ucidialog->setpondering(true); //after the engine will answer with the bestmove, the pondering flag should be set back to true      
      }
    } else {//if pondering is set false, should stay always false, no need of touching it
      ucidialog->sendcomm(6); //sending position
      ucidialog->sendcomm(7); //sending go
    }

    //listening to the chess engine
    while (! ucidialog->checkanswok(3)) {
      sleep(0.1);
      ucidialog->geteansw();
      letgenupdate();
    }
    
    std::string mmove = ucidialog->getmovetodo();
    
    cbbuf << "Chess engine for " << plm->whoplay() << " moves " << mmove;
    printmess();
    
    int coordset[4];
    
    try {
      coordset[0] = ChessCoordinates::xin(mmove.substr(0, 1));
      coordset[1] = ChessCoordinates::yin(mmove.substr(1, 1));
      coordset[2] = ChessCoordinates::xin(mmove.substr(2, 1));
      coordset[3] = ChessCoordinates::yin(mmove.substr(3, 1));
    } catch (std::exception e) {
      std::cerr << "Something wrong with collecting the coordinates of the move indicated by the engine!" << std::endl;
      return false;
    }
    
    plm->move_from = getsquare(coordset[0], coordset[1]);
    plm->move_to = getsquare(coordset[2], coordset[3]);
    
    if (mmove.size() > 4) {
      char promsymbol = toupper(mmove.at(4));
      std::string promstr = std::string(1, promsymbol);
      plm->promoteinto = ChessIdentifiers::arrpid.at(promstr);
    }
    
    if (plm->move_from == nullptr || plm->move_to == nullptr) {
      std::cerr << "Something wrong with getting one or both squares corresponding to the coordinates indicated by the engine." << std::endl;
      return false;
    }
  } else {//now the call in ponder phase  
    //sending now command for pondering (position ang go ponder)
    if (ucidialog->getpondering()) {
      ucidialog->sendcomm(6); //sending position
      ucidialog->sendcomm(7); //sending go
    }
    ChessUCI::forcestopp = false; //resetting to false after each move
  }
  
  return true;
}

//stop the chess engine
void ChessBoard::engine_stop() {
  ChessUCI* currce;
  if (player_moving->wpcolor() == white) {currce = uciwh;}
  else if (player_moving->wpcolor() == black) {currce = ucibl;}
  
  currce->sendcomm(8); //sending stop
}

//set the thinking time for chess engine
void ChessBoard::set_cetime(double t) {
  uciwh->set_thinktime(t);
  ucibl->set_thinktime(t);
}
