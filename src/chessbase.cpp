/*
 * chessbase.cpp
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


#include <algorithm>

#include "chessbase.hpp"
using namespace std::placeholders;

/* static member initializations of ChessIdentifiers class
 */
std::string ChessIdentifiers::pgen = "0";
std::string ChessIdentifiers::ppaw = "";
std::string ChessIdentifiers::pro = "R";
std::string ChessIdentifiers::pkn = "N";
std::string ChessIdentifiers::pbi = "B";
std::string ChessIdentifiers::pqu = "Q";
std::string ChessIdentifiers::pki = "K";
//generic and pawn not included in the array because they are never used in algebric notation
ChessIdentifiers::hashidtype ChessIdentifiers::arrpid = {{ChessIdentifiers::pro, rock}, {ChessIdentifiers::pkn, knight}, 
  {ChessIdentifiers::pbi, bishop}, {ChessIdentifiers::pqu, queen}, {ChessIdentifiers::pki, king}};


/* static member initialization of ChessFEN
 */
std::string ChessFEN::pw = "P";
std::string ChessFEN::pb = "p";
std::string ChessFEN::rw = "R";
std::string ChessFEN::rb = "r";
std::string ChessFEN::nw = "N";
std::string ChessFEN::nb = "n";
std::string ChessFEN::bw = "B";
std::string ChessFEN::bb = "b";
std::string ChessFEN::qw = "Q";
std::string ChessFEN::qb = "q";
std::string ChessFEN::kw = "K";
std::string ChessFEN::kb = "k";

ChessFEN::hashfenid ChessFEN::fenidtable = {{ChessFEN::pw, ChessFEN::IdPair(pawn, white)}, {ChessFEN::pb, ChessFEN::IdPair(pawn, black)},
  {ChessFEN::rw, ChessFEN::IdPair(rock, white)}, {ChessFEN::rb, ChessFEN::IdPair(rock, black)},
  {ChessFEN::nw, ChessFEN::IdPair(knight, white)}, {ChessFEN::nb, ChessFEN::IdPair(knight, black)},
  {ChessFEN::bw, ChessFEN::IdPair(bishop, white)}, {ChessFEN::bb, ChessFEN::IdPair(bishop, black)},
  {ChessFEN::qw, ChessFEN::IdPair(queen, white)}, {ChessFEN::qb, ChessFEN::IdPair(queen, black)},
  {ChessFEN::kw, ChessFEN::IdPair(king, white)}, {ChessFEN::kb, ChessFEN::IdPair(king, black)}};

//overloading operator ==
bool ChessFEN::IdPair::operator == (const ChessFEN::IdPair& oth) {
  if (pi == oth.pi && col == oth.col) {return true;}
  else {return false;}
}

//static member method initialization
std::string ChessFEN::getkeyofval(ChessFEN::IdPair pid) {
  std::string res = "";
  for (hashfenid::iterator it = fenidtable.begin(); it != fenidtable.end(); it++) {
    if (pid == it->second) {
      res = it->first;
      break;
    }
  }
  return res;
}


/* non member overload operator for get opposite color
 */
c_color operator ! (const c_color cc) {
  c_color res;
  if (cc == white) {res = black;}
  else if (cc == black) {res = white;}
  return res;  
}


/* methods and initializations of CHVector class
 */
//static member initialized here, not directly in the class.
int CHVector::min_x = MINX;
int CHVector::max_x = MAXX;
int CHVector::min_y = MINY;
int CHVector::max_y = MAXY;

int ChessPawn::whcounter = 0;
int ChessPawn::blcounter = 0;

int ChessRock::whcounter = 0;
int ChessRock::blcounter = 0;

int ChessKnight::whcounter = 0;
int ChessKnight::blcounter = 0;

int ChessBishop::whcounter = 0;
int ChessBishop::blcounter = 0;

int ChessQueen::whcounter = 0;
int ChessQueen::blcounter = 0;

int ChessKing::whcounter = 0;
int ChessKing::blcounter = 0;

int ChessFakePawn::fakecounter = 0;


//constructors and destructor
CHVector::CHVector() {}

CHVector::CHVector(int a, int b, bool confine) {
  setvalues(a, b, confine);
}

CHVector::~CHVector() {}

//overloading operator + for vectorial sum
CHVector CHVector::operator + (const CHVector& par) {
  int rx = x + par.x;
  int ry = y + par.y;
  CHVector res(rx, ry, true);
  return res;
}

//overloading operator == for vectorial equality
bool CHVector::operator == (const CHVector& par) {
  if (x == par.x && y == par.y) {return true;}
  else {return false;}
}

//overloading operator != for vectorial inequality
bool CHVector::operator != (const CHVector& par) {
  return !((*this) == par);
}

//printing the position
void CHVector::print() {
  std::cout << "position(x,y) = " << x << ", " << y << std::endl;
}

//check if the vector is valid (inside the square)
bool CHVector::isinvalid() {
  bool res;
  if ((getx() == -1) && (gety() == -1)) {res = true;}
  else {res = false;}
  return res;
}

//set the coordinates of the vector
void CHVector::setvalues(int a, int b, bool check) {
  bool initi = true;
  if (check) {
    try {
      if (a < min_x) {throw "x too low";}
      else if (a >= max_x) {throw "x too high";}
      else if (b < min_y) {throw "y too low";}
      else if (b >= max_y) {throw "y too high";}
    }
    catch (char const* e) {
      //std::cout << "Outside boundary: " << e << std::endl;
      initi = false;
    }
  }
  
  if (initi) {
    x = a;
    y = b;
  } else {
    x = -1;
    y = -1;
  }
}

//methods and initializations of Piece class
Piece::Piece() {
  idpi = generic;
  nalg = ChessIdentifiers::pgen;
  icon = "Generic icon";
}

Piece::Piece(c_color c) : Piece() {
  color = c;
}

Piece::Piece(c_color c, int x, int y) : CHVector(x, y, true) {
  color = c;
}

Piece::~Piece() {}

//quick methods to get some values
c_color Piece::getoppcolor() {
  c_color res;
  if (color == white) {res = black;}
  else if (color == black) {res = white;}
  return res;
}

//extract a move from the array
CHVector Piece::getmoves(int i, int j) {
  std::vector<CHVector> vret = moves->at(i);
  CHVector ret = vret.at(j);
  if (color == white) {return ret.reverse();}
  else {return ret;}
}

//extract an eating move from the array
CHVector Piece::geteating(int i, int j) {
  std::vector<CHVector> vret = eating->at(i);
  CHVector ret = vret.at(j);  
  if (color == white) {return ret.reverse();}
  else {return ret;}
}

//extract a special move from the array
CHVector Piece::getspecials(int i, int j) {
  std::vector<CHVector> vret = specials->at(i);
  CHVector ret = vret.at(j);
  if (color == white) {return ret.reverse();}
  else {return ret;}
}

//return string corresponding to the type of piece
std::string Piece::getidtxt() {
  std::string res;
  if (idpi == pawn) {res = "Pawn";}
  else if (idpi == rock) {res = "Rock";}
  else if (idpi == bishop) {res = "Bishop";}
  else if (idpi == knight) {res = "Knight";}
  else if (idpi == queen) {res = "Queen";}
  else if (idpi == king) {res = "King";}
  else if (idpi == fakepawn) {res = " ";}
  else if (idpi == generic) {res = "I really should not be here!";}
  
  return res;
}

/* link properly the methods to extract a move, eating or special move
 * assing a method before using ChessBoard::exploremoves method on a Piece,
 * it is suggested to use again this method to assign the null target after the exploremoves method is used
 */
void Piece::assign_methods(int a) {
  if (a == 0) {//null target
    wrdirection = nullptr;
    wrsteps = nullptr;
    getaction = nullptr;
  } else if (a == 1) {//move
    wrdirection = std::bind(&Piece::moves_directions, this);
    wrsteps = std::bind(&Piece::moves_steps, this, _1);
    getaction = std::bind(&Piece::getmoves, this, _1, _2);
  } else if (a == 2) {//eating
    wrdirection = std::bind(&Piece::eating_directions, this);
    wrsteps = std::bind(&Piece::eating_steps, this, _1);
    getaction = std::bind(&Piece::geteating, this, _1, _2);
  } else if (a == 3) {//special move
    wrdirection = std::bind(&Piece::specials_directions, this);
    wrsteps = std::bind(&Piece::specials_steps, this, _1);
    getaction = std::bind(&Piece::getspecials, this, _1, _2);
  } else {
    std::cout << "ERROR, wrong linking argument!" << std::endl;
    std::exit(EXIT_FAILURE);
  }
} 

//set the icon for the gui implementation
void Piece::seticon() {
  if (getidtype() == generic) {
    icon = "errorimage.png";
  } else if (getidtype() == fakepawn) {
    icon = "noicon";
  } else {
    std::string cstr;
    if (color == black) {cstr = "Black";}
    else if (color == white) {cstr = "White";}
    
    icon = getidtxt() + cstr + ".png";
  }
}

/* A null vector for pieces with no special moves
 */
CHVector nullmove = CHVector(0, 0, false);
std::vector<CHVector> nv = {nullmove};
std::vector<std::vector<CHVector>> nospecials = {nv};

/* The various pieces of chess game, methods and implementations
 */
 
/* Pawn
 */
 
//building functions for static member initialization. The vector to copy in the static member must be initialized inside a function.
std::vector<std::vector<CHVector>> make_pawn_moves(void) {
  std::vector<std::vector<CHVector>> ov;
  std::vector<CHVector> v;
  
  v.push_back(CHVector(0, 1, false));
  ov.push_back(v);
  return ov;
} 

std::vector<std::vector<CHVector>> make_pawn_eating(void) {
  std::vector<std::vector<CHVector>> ov;
  std::vector<CHVector> va;
  std::vector<CHVector> vb;
  
  va.push_back(CHVector(1, 1, false));
  vb.push_back(CHVector(-1, 1, false));

  ov.push_back(va);
  ov.push_back(vb);
  return ov;
}

std::vector<std::vector<CHVector>> make_pawn_specials(void) {
  std::vector<std::vector<CHVector>> ov;
  std::vector<CHVector> v;
  
  v.push_back(CHVector(0, 1, false));
  v.push_back(CHVector(0, 2, false));
  ov.push_back(v);
  return ov;
}

//static member initialization.
std::vector<std::vector<CHVector>> ChessPawn::smoves = make_pawn_moves();
std::vector<std::vector<CHVector>> ChessPawn::seating = make_pawn_eating();
std::vector<std::vector<CHVector>> ChessPawn::sspecials = make_pawn_specials();

//Constructor and other methods.
ChessPawn::ChessPawn(c_color c, int x, int y) : Piece(c, x, y) {
  idpi = pawn;
  nalg = ChessIdentifiers::ppaw;
  seticon();
  link_moveat();
  if (color == white) {num_id = whcounter++;}
  else if (color == black) {num_id = blcounter++;}
}

ChessPawn::ChessPawn(int nid, c_color c, bool ong, bool bmv, int x, int y) : Piece(c, x, y) {
  idpi = pawn;
  nalg = ChessIdentifiers::ppaw;
  seticon();
  link_moveat();
  num_id = nid;
  ongame = ong;
  beenmoved = bmv;
  
  if (color == white) {whcounter++;}
  else if (color == black) {blcounter++;}
}

//destructor
ChessPawn::~ChessPawn() {
  if (color == white) {num_id = whcounter--;}
  else if (color == black) {num_id = blcounter--;}
}

bool ChessPawn::promoteme() {
  if (color == white && gety() == 0) {return true;}
  else if (color == black && gety() == 7) {return true;}
  else {return false;}
}

void ChessPawn::link_moveat() {  
  moves = &smoves;
  eating = &seating;
  specials = &sspecials;
}


/* Rock
 */

//building functions for static member initialization. The vector to copy in the static member must be initialized inside a function.
std::vector<std::vector<CHVector>> make_rock_moves(void) {
  std::vector<std::vector<CHVector>> ov;
  std::vector<CHVector> va;
  std::vector<CHVector> vb;
  
  for (int i = CHVector::min_x +1; i < CHVector::max_x; i++) {
    va.push_back(CHVector(i, 0, false));
    vb.push_back(CHVector(-i, 0, false));
  }
  ov.push_back(va);
  ov.push_back(vb);

  va.clear();
  vb.clear();
  
  for (int j = CHVector::min_y +1; j < CHVector::max_y; j++) {
    va.push_back(CHVector(0, j, false));
    vb.push_back(CHVector(0, -j, false));
  }
  ov.push_back(va);
  ov.push_back(vb);

  return ov;
}

//static member initialization.
std::vector<std::vector<CHVector>> ChessRock::smoves = make_rock_moves();

//Constructor and other methods.
ChessRock::ChessRock(c_color c, int x, int y) : Piece(c, x, y) {
  idpi = rock;
  nalg = ChessIdentifiers::pro;
  seticon();
  link_moveat();
  if (color == white) {num_id = whcounter++;}
  else if (color == black) {num_id = blcounter++;}
}

ChessRock::ChessRock(int nid, c_color c, bool ong, bool bmv, int x, int y) : Piece(c, x, y) {
  idpi = rock;
  seticon();
  nalg = ChessIdentifiers::pro;
  link_moveat();
  num_id = nid;
  ongame = ong;
  beenmoved = bmv;
  
  if (color == white) {whcounter++;}
  else if (color == black) {blcounter++;}
}

//destructor
ChessRock::~ChessRock() {
  if (color == white) {num_id = whcounter--;}
  else if (color == black) {num_id = blcounter--;}
}

int ChessRock::getcounter(c_color cc) {
  int res;
  if (cc == white) {res = whcounter;}
  else if (cc == black) {res = blcounter;}
  return res;
}

void ChessRock::link_moveat() {
  moves = &smoves;
  eating = &smoves;
  specials = &nospecials;
}

/* Knight
 */

//building functions for static member initialization. The vector to copy in the static member must be initialized inside a function.
std::vector<std::vector<CHVector>> make_knight_moves(void) {
  std::vector<std::vector<CHVector>> ov;
  std::vector<CHVector> v;
  
  v.push_back(CHVector(1, 2, false));
  ov.push_back(v);
  v.clear();
    
  v.push_back(CHVector(2, 1, false));
  ov.push_back(v);
  v.clear();
  
  v.push_back(CHVector(2, -1, false));
  ov.push_back(v);
  v.clear();
  
  v.push_back(CHVector(1, -2, false));
  ov.push_back(v);
  v.clear();
  
  v.push_back(CHVector(-1, -2, false));
  ov.push_back(v);
  v.clear();
  
  v.push_back(CHVector(-2, -1, false));
  ov.push_back(v);
  v.clear();
  
  v.push_back(CHVector(-2, 1, false));
  ov.push_back(v);
  v.clear();
  
  v.push_back(CHVector(-1, 2, false));
  ov.push_back(v);
  v.clear();
  
  return ov;
} 

//static member initialization.
std::vector<std::vector<CHVector>> ChessKnight::smoves = make_knight_moves();

//Constructor and other methods.
ChessKnight::ChessKnight(c_color c, int x, int y) : Piece(c, x, y) {
  idpi = knight;
  nalg = ChessIdentifiers::pkn;
  seticon();
  link_moveat();
  if (color == white) {num_id = whcounter++;}
  else if (color == black) {num_id = blcounter++;}
}

ChessKnight::ChessKnight(int nid, c_color c, bool ong, bool bmv, int x, int y) : Piece(c, x, y) {
  idpi = knight;
  nalg = ChessIdentifiers::pkn;
  seticon();
  link_moveat();
  num_id = nid;
  ongame = ong;
  beenmoved = bmv;
  
  if (color == white) {whcounter++;}
  else if (color == black) {blcounter++;}
}

//destructor
ChessKnight::~ChessKnight() {
  if (color == white) {num_id = whcounter--;}
  else if (color == black) {num_id = blcounter--;}
}

int ChessKnight::getcounter(c_color cc) {
  int res;
  if (cc == white) {res = whcounter;}
  else if (cc == black) {res = blcounter;}
  return res;
}
  
void ChessKnight::link_moveat() {
  moves = &smoves;
  eating = &smoves;
  specials = &nospecials;
}

/* Bishop
 */

//building functions for static member initialization. The vector to copy in the static member must be initialized inside a function.
std::vector<std::vector<CHVector>> make_bishop_moves(void) {
  std::vector<std::vector<CHVector>> v;
  std::vector<CHVector> va;
  std::vector<CHVector> vb;
  std::vector<CHVector> vc;
  std::vector<CHVector> vd;
  
  for (int i = CHVector::min_x +1, j = CHVector::min_y +1; (i < CHVector::max_x) && (j < CHVector::max_y); i++, j++) {
    va.push_back(CHVector(i, j, false));
    vb.push_back(CHVector(i, -j, false));
    vc.push_back(CHVector(-i, -j, false));
    vd.push_back(CHVector(-i, j, false));
  }
  
  v.push_back(va);
  v.push_back(vb);
  v.push_back(vc);
  v.push_back(vd);
  
  return v;
} 

//static member initialization.
std::vector<std::vector<CHVector>> ChessBishop::smoves = make_bishop_moves();

//Constructor and other methods.
ChessBishop::ChessBishop(c_color c, int x, int y) : Piece(c, x, y) {
  idpi = bishop;
  nalg = ChessIdentifiers::pbi;
  seticon();
  link_moveat();
  if (color == white) {num_id = whcounter++;}
  else if (color == black) {num_id = blcounter++;}
}

ChessBishop::ChessBishop(int nid, c_color c, bool ong, bool bmv, int x, int y) : Piece(c, x, y) {
  idpi = bishop;
  nalg = ChessIdentifiers::pbi;
  seticon();
  link_moveat();
  num_id = nid;
  ongame = ong;
  beenmoved = bmv;
  
  if (color == white) {whcounter++;}
  else if (color == black) {blcounter++;}
}

//destructor
ChessBishop::~ChessBishop() {
  if (color == white) {num_id = whcounter--;}
  else if (color == black) {num_id = blcounter--;}
}

int ChessBishop::getcounter(c_color cc) {
  int res;
  if (cc == white) {res = whcounter;}
  else if (cc == black) {res = blcounter;}
  return res;
}
  
void ChessBishop::link_moveat() {
  moves = &smoves;
  eating = &smoves;
  specials = &nospecials;
}

/* Queen
 */

//building functions for static member initialization. The vector to copy in the static member must be initialized inside a function.
std::vector<std::vector<CHVector>> make_queen_moves(void) {
  std::vector<std::vector<CHVector>> ov;

  std::vector<CHVector> va;
  std::vector<CHVector> vb;
  std::vector<CHVector> vc;
  std::vector<CHVector> vd;
  
  for (int i = CHVector::min_x +1; i < CHVector::max_x; i++) {
    va.push_back(CHVector(i, 0, false));
    vb.push_back(CHVector(-i, 0, false));
  }
  ov.push_back(va);
  ov.push_back(vb);
  
  for (int j = CHVector::min_y +1; j < CHVector::max_y; j++) {
    vc.push_back(CHVector(0, j, false));
    vd.push_back(CHVector(0, -j, false));
  }
  ov.push_back(vc);
  ov.push_back(vd);
  
  va.clear(); vb.clear(); vc.clear(); vd.clear();
  
  for (int k = CHVector::min_x +1, l = CHVector::min_y +1; (k < CHVector::max_x) && (l < CHVector::max_y); k++, l++) {
    va.push_back(CHVector(k, l, false));
    vb.push_back(CHVector(k, -l, false));
    vc.push_back(CHVector(-k, l, false));
    vd.push_back(CHVector(-k, -l, false));
  }
  
  ov.push_back(va);
  ov.push_back(vb);
  ov.push_back(vc);
  ov.push_back(vd);
    
  return ov;
} 

//static member initialization.
std::vector<std::vector<CHVector>> ChessQueen::smoves = make_queen_moves();

//Constructor and other methods.
ChessQueen::ChessQueen(c_color c, int x, int y) : Piece(c, x, y) {
  idpi = queen;
  nalg = ChessIdentifiers::pqu;
  seticon();
  link_moveat();
  if (color == white) {num_id = whcounter++;}
  else if (color == black) {num_id = blcounter++;}
}

ChessQueen::ChessQueen(int nid, c_color c, bool ong, bool bmv, int x, int y) : Piece(c, x, y) {
  idpi = queen;
  nalg = ChessIdentifiers::pqu;
  seticon();
  link_moveat();
  num_id = nid;
  ongame = ong;
  beenmoved = bmv;
  
  if (color == white) {whcounter++;}
  else if (color == black) {blcounter++;}
}

//destructor
ChessQueen::~ChessQueen() {
  if (color == white) {num_id = whcounter--;}
  else if (color == black) {num_id = blcounter--;}
}

int ChessQueen::getcounter(c_color cc) {
  int res;
  if (cc == white) {res = whcounter;}
  else if (cc == black) {res = blcounter;}
  return res;
}

void ChessQueen::link_moveat() {
  moves = &smoves;
  eating = &smoves;
  specials = &nospecials;
}

/* King
 */

//building functions for static member initialization. The vector to copy in the static member must be initialized inside a function.
std::vector<std::vector<CHVector>> make_king_moves(void) {
  std::vector<std::vector<CHVector>> ov;
  std::vector<CHVector> iv;
  
  iv.push_back(CHVector(0, 1, false));
  ov.push_back(iv);
  iv.clear();
  
  iv.push_back(CHVector(1, 1, false));
  ov.push_back(iv);
  iv.clear();
  
  iv.push_back(CHVector(1, 0, false));
  ov.push_back(iv);
  iv.clear();
  
  iv.push_back(CHVector(1, -1, false));
  ov.push_back(iv);
  iv.clear();
  
  iv.push_back(CHVector(0, -1, false));
  ov.push_back(iv);
  iv.clear();
  
  iv.push_back(CHVector(-1, -1, false));
  ov.push_back(iv);
  iv.clear();
  
  iv.push_back(CHVector(-1, 0, false));
  ov.push_back(iv);
  iv.clear();
  
  iv.push_back(CHVector(-1, 1, false));
  ov.push_back(iv);
  iv.clear();
  
  return ov;
}

std::vector<std::vector<CHVector>> make_king_specials(void) {
  std::vector<std::vector<CHVector>> ov;
  std::vector<CHVector> va;
  std::vector<CHVector> vb;
  
  va.push_back(CHVector(1, 0, false));
  va.push_back(CHVector(2, 0, false));
  
  vb.push_back(CHVector(-1, 0, false));
  vb.push_back(CHVector(-2, 0, false));
  
  ov.push_back(va);
  ov.push_back(vb);
  
  return ov;
}

//static member initialization.
std::vector<std::vector<CHVector>> ChessKing::smoves = make_king_moves();
std::vector<std::vector<CHVector>> ChessKing::sspecials = make_king_specials();

//Constructor and other methods.
ChessKing::ChessKing(c_color c, int x, int y) : Piece(c, x, y) {
  idpi = king;
  nalg = ChessIdentifiers::pki;
  seticon();
  link_moveat();
  if (color == white) {num_id = whcounter++;}
  else if (color == black) {num_id = blcounter++;}
}

ChessKing::ChessKing(int nid, c_color c, bool ong, bool bmv, int x, int y) : Piece(c, x, y) {
  idpi = king;
  nalg = ChessIdentifiers::pki;
  seticon();
  link_moveat();
  num_id = nid;
  ongame = ong;
  beenmoved = bmv;
  
  if (color == white) {whcounter++;}
  else if (color == black) {blcounter++;}
}

//destructor
ChessKing::~ChessKing() {
  if (color == white) {num_id = whcounter--;}
  else if (color == black) {num_id = blcounter--;}
}

int ChessKing::getcounter(c_color cc) {
  int res;
  if (cc == white) {res = whcounter;}
  else if (cc == black) {res = blcounter;}
  return res;
}

void ChessKing::link_moveat() {
  moves = &smoves;
  eating = &smoves;
  specials = &sspecials;
}

/* FakePawn, placeholder to allow pawn eating en passant
 */

//no need of building functions for static member initialization, not used.

//Constructor and other methods.
ChessFakePawn::ChessFakePawn(c_color c, int x, int y, ChessPawn* rfp) : Piece(c, x, y) {
  idpi = fakepawn;
  nalg = ChessIdentifiers::pgen;
  seticon();
  link_moveat();
  num_id = 0;
  fakecounter++;
  refpawn = rfp;
}

//constructor to be used after the move of the reference pawn: it allow to create a fake pawn with the correct coordinate automatically after the move of a pawn
ChessFakePawn::ChessFakePawn(ChessPawn* rfp) : ChessFakePawn(rfp->getcolor(), rfp->getx(), rfp->gety(), rfp) {
  if (color == white) {y++;}
  else if (color == black) {y--;}
}

//never use it for real fakepawn, only for placeholder in loading a saving
ChessFakePawn::ChessFakePawn(int nid, c_color c, bool ong, bool bmv, int x, int y) : Piece(c, x, y) {
  idpi = fakepawn;
  nalg = ChessIdentifiers::pgen;
  seticon();
  link_moveat();
  num_id = nid;
  ongame = ong;
  beenmoved = bmv;
  
  fakecounter++;
}

ChessFakePawn::ChessFakePawn(ChessFakePawn* placer, ChessPawn* rfp) : ChessFakePawn(placer->getcolor(), placer->getx(), placer->gety(), rfp) {
  num_id = placer->getnumid();
  ongame = placer->ongame;
  beenmoved = placer->beenmoved;
}

//destructor
ChessFakePawn::~ChessFakePawn() {
  fakecounter--;
}

void ChessFakePawn::link_moveat() {
  moves = &nospecials;
  eating = &nospecials;
  specials = &nospecials;
}

/* Non member functions */
//overloading non member operator<< for Piece. We can not overload operator>> because piece is an abstract class, we should verify before that the piece is the correct subclass 
std::ostream& operator<<(std::ostream& os, const Piece& pst) {
  os << pst.idpi << pst.num_id << pst.color << pst.ongame << pst.beenmoved << pst.x << pst.y << '|';
  return os;
}

//overloading non member >> operator for Piece
//~ std::istream& operator>>(std::istream &is, Piece &pst) {
  //~ int i, j;
  //~ is >> i >> j >> pst.num_id;
  //~ pst.id = static_cast<wpiece>(i);
  //~ pst.color = static_cast<c_color>(j);
  //~ return is;
//~ }


/* Methods of class ChessCoordinates
 */
ChessCoordinates::xlarray buildxletters(void) {
  ChessCoordinates::xlarray res;
  for (int i = 0; i < CHVector::max_x; i++) {res[i] = 'a' + i;}
  return res;
}

ChessCoordinates::ylarray buildynumbers(void) {
  ChessCoordinates::ylarray res;
  for (int i = 0; i < CHVector::max_y; i++) {res[i] = '0' + CHVector::max_y - i;} //reverse order because y coordinates in the square matrix are inversed.
  return res;
}

//static member initialization
ChessCoordinates::xlarray ChessCoordinates::xletters = buildxletters();
ChessCoordinates::ylarray ChessCoordinates::ynumbers = buildynumbers();

ChessCoordinates::ChessCoordinates() {}

ChessCoordinates::~ChessCoordinates() {}

//teplate function for reading input coordinates, template T is used in place of xlarray or ylarray. If conversion fails, the return integer is -1 
template <typename T> int ChessCoordinates::genin(std::string ist, T arrt) {
  int res;
  char cist;
  typename T::iterator search;
  if (ist.size() == 1) {
    cist = ist.c_str()[0];
    search = std::find(std::begin(arrt), std::end(arrt), cist);
    
    if (search == std::end(arrt)) {res = -1;}
    else {res = search - arrt.begin();}
  } else {res = -1;}

  return res;
}

//selecting the x char (letter) for output
char ChessCoordinates::xout(int n) {
  char res;
  if (n >= CHVector::min_x && n < CHVector::max_x) {res = xletters[n];}
  else {std::cerr << "Error, something wrong in x coordinates selection!" << std::endl; std::exit(EXIT_FAILURE);}
  
  return res;
}

//selecting the y char (number) for output
char ChessCoordinates::yout(int n) {
  char res;
  if (n >= CHVector::min_y && n < CHVector::max_y) {res = ynumbers[n];}
  else {std::cerr << "Error, something wrong in y coordinates selection!" << std::endl; std::exit(EXIT_FAILURE);}
  
  return res;
}

//selecting the x int (corresponding to a letter) from input
int ChessCoordinates::xin(std::string ss) {
  return genin<xlarray>(ss, xletters);
}  

//selecting the y int (corresponding to a number) from input
int ChessCoordinates::yin(std::string ss) {
  return genin<ylarray>(ss, ynumbers);
}
