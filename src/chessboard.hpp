/*
 * chessboard.hpp
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


#ifndef CHESSBOARD_H_DEF
#define CHESSBOARD_H_DEF 1

#include <array>
#include <sstream>
#include <fstream>

#include "chess_dconst.hpp"
#include "chessbase.hpp"
#include "chessutils.hpp"

/* Struct to write once filename extensions
 */
struct ChessExts {
  static std::string saveext; //extension for files to saving the game
  static std::string histext; //extension for files to save the history only
};

/* Forward declaration to resolve circular dependencies
 * The class will be defined later in this file
 */
class ChessBoard;

/* Class to manage the the saving system
 */
class ChessSaving {
  friend class ChessUCI; //now ChessUCI can have access to private member of ChessSaving
  
  private:
    std::stringstream filename;
    std::fstream savebuf;
    std::vector<std::streampos> linepos;
    std::array<std::string, 4> gresults = {{"*", "1/2-1/2", "0-1", "1-0"}};
    
    std::string dftsetline(std::string);
    std::string inifen;

  public:
    typedef std::vector<std::streampos>::iterator chsaviter;
    chsaviter lineiter;
    
    ChessSaving();
    virtual ~ChessSaving();
    
    bool turnlost = false;
    
    chsaviter getbegin(void) {return linepos.begin();}
    chsaviter getend(void) {return linepos.end();}
    
    void readinifen(std::string ifen) {inifen = ifen;}
    void initcs(std::string);
    
    bool drawforthree(void);
        
    void autosavegame(const ChessBoard&, bool);
    std::string loadstatus(ChessBoard&);
    bool clearfuture(bool = false);
    
    //first bool true to align the algebraic notation, a turn for each line, second bool true to get the long notation, third bool is to get only moves without numeration
    std::string gethistory(bool = true, bool = false, bool = false);
    
    void writefhist(std::string, bool = false); //bool = true allows long notation 
    void savegame(const ChessBoard&, std::string);
    void savegamepgn(const ChessBoard&, std::string, const ChessConfig&);
    bool loadgame(ChessBoard&, std::string);
    bool loadgamepgn(ChessBoard&, ChessPGN::pgnmoves);
};

/*Class represent a square of the board
 */
class ChessSquare : public CHVector {
  private:
    c_color color;
    
  public:
    Piece* p;
    
    ChessSquare();
    ChessSquare(c_color, int, int);
    ChessSquare(c_color, Piece*);
    virtual ~ChessSquare();

    void pieceinsquare(Piece*);

    c_color getcolor(void) {return color;}
};

//introducing typedef here: needed inclusion of standard library and ChessSquare class declaration
typedef std::array<std::array<ChessSquare, 8>, 8> cb_square;
typedef std::array<std::array<Piece*, 8>, 8> cb_pointers;

/* Abstract class represent a chess player
 */ 
class ChessPlayer {
  private:
    bool ishuman;
    cb_square* ptoboard; //pointer to the 2D array representing the chessboard
        
  protected:
    c_color color;
    
    std::string savefn = "";
    std::string histfn = "";
    
    void setsavefn(std::string);
    void sethistoryfn(std::string fn);
    
  public:
    /* Declared here for pointer-type compatibility but actually used only by the GUI child of this class */
    ChessBoard* ownboard;
    int sel_x;
    int sel_y;
    void setpointer(ChessBoard* pointer) {ownboard = pointer;}
    /* ... */

    wpiece promoteinto = generic;
    ChessUCI* ucieng = nullptr;
    ChessSquare* move_from = nullptr;
    ChessSquare* move_to = nullptr;
  
    ChessPlayer();
    ChessPlayer(c_color);
    ChessPlayer(c_color, bool);
    virtual ~ChessPlayer();

    std::string getsavefn(void) {return savefn;}
    std::string gethistoryfn(void) {return histfn;}

    void set_ptb(cb_square* bb) {ptoboard = bb;}
    
    c_color wpcolor(void) {return color;}
    std::string whoplay(void);
    bool isplayerhuman(void) {return ishuman;}
    void setifph(bool x) {ishuman = x;}
    
    ChessSquare* getsquare(int a, int b) {return &(*ptoboard)[a][b];}
    void resetpm(void) {move_from = nullptr; move_to = nullptr;}
    
    virtual bool player_act(void) =0;
};


/* Abstract class, to be inherited by a child class implementing the input / output virtual functions
 */
class ChessBoard {
  friend class ChessSaving; //now ChessSaving can have access to private member of ChessBoard

  protected:
    int turn = 1;
    int drawffcounter = 0;
    int tempdfc;
    c_color playerstart = white;
    gamefinal finalres = notfinished; //tells the conclusion of the game
    std::stringstream algebnotshort;
    std::stringstream algebnotlong;
    
    ChessSaving *saver;
    ChessUCI *uciwh;
    ChessUCI *ucibl;
    
    std::vector<Piece*> pieces;  //piece collection
    std::array<ChessPlayer*, 2> players; //the players
    ChessKing *whking, *blking; //pointers to the kings
    
    /* Main array is x, inner array is y coord. When accessing, outer array is the index in the first square brackets: [x][y]
     */
    cb_square squares; //the chessboard
    cb_square temporarysquares; //a temporary chessboard
    cb_pointers squareimage; //to save an image of the chessboard (only the pointers), used to reset the move if needed

    //buffer to store text messages. Printing delegated to virtual function printcb()
    std::stringstream cbbuf;

    void construct_pieces(std::string); //to build the pieces, filling the dedicated vector
    void construct_board(void); //building the chessboard

    void gather_players(ChessPlayer*, ChessPlayer*);
    void set_board_for_players(void);
    ChessRock* checkcastlingrock(int, int);

    bool engine_act(ChessPlayer*, ChessUCI*, bool = false);
    bool check_engine(ChessUCI*, ChessUCI*);
    bool start_engine(ChessUCI*);

  public:
    ChessPlayer* player_moving;
    CHVector lastmove;
    bool reqcastling = false;
    bool placefakepawn = false;
    
    ChessBoard(std::string);
    ChessBoard(const ChessBoard&);
    virtual ~ChessBoard();
        
    cb_square* getptosquares(void) {return &squares;}
    ChessSquare* getsquare(int a, int b);
    ChessSquare* getsquaretemp(int a, int b);
    gamefinal getfinalres(void) const {return finalres;}
    std::string wrpgethistory(bool a = true, bool b = true, bool c = true) {return saver->gethistory(a, b, c);} //used to extract information for the analyser
    
    void squareinpiece(Piece*);
    void emptysquare(int, int);
    void sethumplayers(std::array<bool, 2>);
    std::string genFEN(void);
    
    bool chessmove(ChessPlayer*); //move given by the ChessPlayer class of the player who move
    bool chessmove(c_color, int, int, int, int); //coordinates determined by integers
    bool chessmove(c_color, std::string); //read move in algebric notation
    bool chessmove(c_color, ChessSquare*, ChessSquare*, wpiece = generic); //move given by starting and arrival ChessSquare pointers
    bool check_starting(c_color, ChessSquare*);
    bool check_rule_move(c_color, ChessSquare*, ChessSquare*);
    bool process_move(Piece*, ChessSquare*);
    bool isincheck(ChessPlayer*);
    bool isincheck(ChessKing*);
    bool ischeckmate(ChessPlayer*);
    bool ischeckmate(ChessKing*);
    bool isdraw(ChessPlayer*);
    bool isstalemate(ChessPlayer*);
    
    void save_cbimage(void);
    void restore_cbimage(void);

    std::vector<ChessSquare*> exploremoves(Piece*, bool = false); //explore move possibilities
    std::vector<ChessSquare*> ismenacing(Piece*, int, bool = false); //check if a piece is menacing any opponent's piece
    std::vector<ChessSquare*> exploremoveats(Piece*); //explore all combination of moves, eatings and special moves
    std::vector<ChessSquare*> kinglegalmoves(ChessKing*); //explore king's legal moves (squares not menaced)
    std::vector<Piece*> ismenacedby(c_color, ChessSquare*, bool = false, int = -1); //check if a square (with or without a piece) is menaced by any piece of the given color

    void docastling(ChessRock*);
    Piece* promotepawn(ChessPawn*, wpiece);
    void removefakes(c_color);
    void enpassanteating(ChessFakePawn*);

    bool goback(int = 1);
    bool goforward(int = 1);
    
    bool wrploadgame(ChessPGN::pgnmoves); //public wrapper for the ChessSaving method, needed only for load and not for save
    void resignmess(void);
    
    void writealgnot(Piece*, ChessSquare*, bool);
    
    void engine_stop(void);
    void set_cetime(double);

    //pure virtual methods managing the input / output, different for command line game or graphical game, and turnation.
    virtual void printmess(bool = false) =0;
    virtual bool turnation(void) =0;
    virtual void printcb(void) =0;
    virtual void turnation_end(bool) =0;
    virtual wpiece choose_promotion(c_color) =0;
    virtual bool askdraw(int) =0;
    virtual std::string printhist(std::string, bool = false) =0;
    virtual void letgenupdate(void) =0;
};

#endif
