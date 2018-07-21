/*
 * chessbase.hpp
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


#ifndef CHESSBASE_H_DEF
#define CHESSBASE_H_DEF 1

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

#include "chess_dconst.hpp"

/* Association with identifiers and pieces 
 */
struct ChessIdentifiers {
  static std::string pgen;
  static std::string ppaw;
  static std::string pro;
  static std::string pkn;
  static std::string pbi;
  static std::string pqu;
  static std::string pki;
  
  typedef std::unordered_map<std::string, wpiece> hashidtype;
  static hashidtype arrpid;
};

/* Class holding as static members the symbols used in FEN notation
 */
class ChessFEN {
  public:
    static std::string pw;
    static std::string pb;
    static std::string rw;
    static std::string rb;
    static std::string nw;
    static std::string nb;
    static std::string bw;
    static std::string bb;
    static std::string qw;
    static std::string qb;
    static std::string kw;
    static std::string kb;

    struct IdPair {
      public:
        wpiece pi;
        c_color col;
        
        IdPair(wpiece a, c_color b) : pi{a}, col{b} {}
        
        bool operator== (const IdPair&);
    };
    
    typedef std::unordered_map<std::string, IdPair> hashfenid;
    static hashfenid fenidtable;
    
    static std::string getkeyofval(IdPair);
};

/* Non member overload operator for get opposite color
 */
c_color operator ! (const c_color);

/* 2D vector arithmetic with limits
 */
class CHVector {
  protected:
    int x;
    int y;
        
  public:
    static int min_x;
    static int max_x;
    static int min_y;
    static int max_y;
    
    CHVector();
    CHVector(int, int, bool);
    virtual ~CHVector();
        
    CHVector operator+ (const CHVector&);
    bool operator== (const CHVector&);
    bool operator!= (const CHVector&);
    
    void print(void);
    int getx(void) {return x;}
    int gety(void) {return y;}
    
    bool isinvalid(void);
    
    CHVector reverse(void) {return CHVector(x, -y, false);}
    void setvalues(int, int, bool);
};

/* A Generic chess piece
 */
class Piece : public CHVector {
  protected:
    wpiece idpi;
    std::string nalg;
    int num_id; //numeric id of the piece, used to discriminate pieces of same type and color
    c_color color;
        
    std::string icon; //will store the address of the image for the gui implementation
    std::vector<std::vector<CHVector>>* moves;
    std::vector<std::vector<CHVector>>* eating;
    std::vector<std::vector<CHVector>>* specials;
     
  public:
    bool ongame = true;
    bool beenmoved = false;
    
    Piece();
    Piece(c_color); 
    Piece(c_color, int, int);
    virtual ~Piece();

    wpiece getidtype(void) {return idpi;}
    std::string getidtxt(void);
    std::string getnalg(void) {return nalg;}
    int getnumid(void) {return num_id;}
    c_color getcolor(void) {return color;}
    c_color getoppcolor(void);
    CHVector getmoves(int, int);
    CHVector geteating(int, int);
    CHVector getspecials(int, int);
    
    unsigned int moves_directions(void) {return moves->size();}
    unsigned int moves_steps(int i) {std::vector<CHVector> v = moves->at(i); return v.size();}
    
    unsigned int eating_directions(void) {return eating->size();}
    unsigned int eating_steps(int i) {std::vector<CHVector> v = eating->at(i); return v.size();}
    
    unsigned int specials_directions(void) {return specials->size();}
    unsigned int specials_steps(int i) {std::vector<CHVector> v = specials->at(i); return v.size();}
    
    void seticon(void);
    std::string geticon(void) {return icon;}
    
    friend std::ostream& operator<<(std::ostream&, const Piece&); //this is a function, non a method of this class, but declared here as friend allows it to access the private member of this class
    //~ friend std::istream& operator>>(std::istream&, Piece&);
    
    //function wrappers
    std::function<unsigned int (void)> wrdirection; //Will contains moves_direction / eating_direction / specials_direction according to the required action.
    std::function<unsigned int (int)> wrsteps; //Will contains moves_steps / eating_steps / specials_steps according to the required action.
    std::function<CHVector (int, int)> getaction; //Will contains getmoves / geteating / getspecials according to the required action.
    
    void assign_methods(int); //0: null target - 1: move - 2: eating - 3: special move 
    
    virtual void link_moveat(void) =0;

};

/* The various pieces of chess game, class declarations
 */

//pawn
class ChessPawn : public Piece {
  private: 
    static std::vector<std::vector<CHVector>> smoves;
    static std::vector<std::vector<CHVector>> seating;
    static std::vector<std::vector<CHVector>> sspecials;
    
    static int whcounter;
    static int blcounter;
    
  public:
    ChessPawn(c_color, int, int);
    ChessPawn(int, c_color, bool, bool, int, int);
    ~ChessPawn();

    static int getcounter(c_color);
    bool promoteme(void);

    void link_moveat(void) override;
};

//rock
class ChessRock : public Piece {
  private:
    static std::vector<std::vector<CHVector>> smoves;
    
    static int whcounter;
    static int blcounter;

  public:
    ChessRock(c_color, int, int);
    ChessRock(int, c_color, bool, bool, int, int);
    ~ChessRock();

    static int getcounter(c_color);
    void link_moveat(void) override;
};

//knight
class ChessKnight : public Piece {
  private:
    static std::vector<std::vector<CHVector>> smoves;
    
    static int whcounter;
    static int blcounter;
  
  public:
    ChessKnight(c_color, int, int);
    ChessKnight(int, c_color, bool, bool, int, int);
    ~ChessKnight();

    static int getcounter(c_color);
    void link_moveat(void) override;
};

//bishop
class ChessBishop : public Piece {
  private:
    static std::vector<std::vector<CHVector>> smoves;
    
    static int whcounter;
    static int blcounter;
  
  public:
    ChessBishop(c_color, int, int);
    ChessBishop(int, c_color, bool, bool, int, int);
    ~ChessBishop();
    
    static int getcounter(c_color);
    void link_moveat(void) override;
};

//queen
class ChessQueen : public Piece {
  private:
    static std::vector<std::vector<CHVector>> smoves;
    
    static int whcounter;
    static int blcounter;
  
  public:
    ChessQueen(c_color, int, int);
    ChessQueen(int, c_color, bool, bool, int, int);
    ~ChessQueen();
    
    static int getcounter(c_color);
    void link_moveat(void) override;
};

//king
class ChessKing : public Piece {
  private:
    static std::vector<std::vector<CHVector>> smoves;
    static std::vector<std::vector<CHVector>> sspecials;
    
    static int whcounter;
    static int blcounter;
  
  public:
    ChessKing(c_color, int, int);
    ChessKing(int, c_color, bool, bool, int, int);
    ~ChessKing();
    
    static int getcounter(c_color);
    void link_moveat(void) override;
};

//fakepawn, just a placeholder to allow pawn eating en passant 
class ChessFakePawn : public Piece {
  private:
    static std::vector<std::vector<CHVector>> smoves;
    static int fakecounter;
    
    ChessPawn* refpawn;
    
  public:
    ChessFakePawn(c_color, int, int, ChessPawn*);
    ChessFakePawn(int, c_color, bool, bool, int, int);
    ChessFakePawn(ChessFakePawn*, ChessPawn*);
    ChessFakePawn(ChessPawn*);
    ~ChessFakePawn();
    void link_moveat(void) override;
    
    static int getcounter(void) {return fakecounter;}
    ChessPawn* getrefp(void) {return refpawn;}
};

/* Class for chess input/output coordinates, made of static attributes and methods only, no need to create object of this class, as its attributes do not change
 */
class ChessCoordinates {
  public: typedef std::array<char, MAXX> xlarray;
  public: typedef std::array<char, MAXY> ylarray;

  private:
    static xlarray xletters;
    static ylarray ynumbers;
    
    template <typename T> static int genin(std::string, T);
    
  public:
    ChessCoordinates();
    ~ChessCoordinates();
    
    static char xout(int);
    static char yout(int);
    static int xin(std::string);
    static int yin(std::string);
};

#endif
