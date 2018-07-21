/*
 * chessutils.hpp
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


#ifndef CHESSUCI_H_DEF
#define CHESSUCI_H_DEF 1

#include <array>
#include <vector>
#include <fstream>
#include <unordered_map>

#include "chess_dconst.hpp"
#include "ipcproc.hpp"


/* Structure holding the go subcommand info to pass 
 */
struct CEGoSubcomm {
  std::string limitmoves;
  double limtime; //in seconds
  int limdepth;
  int limnodes; //in units of 100.000
  int mateinmov;
  bool useme = false;
};

/* Class to deal with the Portable Game Notation (PGN)
 * The class works in reading mode (the file must already exist) and writing mode (if the file exhist, it is overwritten)
 * An instance of this class manage a single PGN file in writing or reading mode
 * A full description of the PGN format can be found here: http://www.saremba.de/chessgml/standards/pgn/pgn-complete.htm
 * Follwing the description given in the above link, this class can understand the export PGN format.
 */
class ChessPGN {
  protected:
    std::string pgnfile;
    bool modewrite;
    std::ofstream pgnstream;
    
  public:
    typedef std::vector<std::string> pgnmoves;
    typedef std::unordered_map<std::string, std::string> typetags;
    
    static std::array<std::string, 4> gresults;
    
    ChessPGN(std::string, char);
    ~ChessPGN();

    std::string getfilename(void) {return pgnfile;}

    std::string readfield(unsigned int, std::string);
    void writefield(std::string, std::string = "?");

    pgnmoves readmoves(unsigned int);
    void writemoves(std::string, unsigned int);
    
    /* Nested class to store a single instance game in PGN format
     * PGN files with multiple games are handled by storing each game in an instance of this class
     */
    class PGNgame {
      private:
        typetags pgntags;
        pgnmoves pgnmovetext;
        
      public:
        PGNgame(typetags a, pgnmoves b) : pgntags(a), pgnmovetext(b) {}
        ~PGNgame();
        
        std::string getfield(std::string);
        pgnmoves getmovetext(void) {return pgnmovetext;}
    };
    
    std::vector<PGNgame*> allgames;

    unsigned int numgames(void) {return allgames.size();}
    PGNgame* getgame(unsigned int i);
    
    virtual unsigned int selectgame(void); //virtual method, we can not make it pure virtual because this class is not abstract, it is instantiated by itself
};

/* Forward declaration to resolve circular dependencies
 * The class is defined in file chessboard.hpp
 */
class ChessSaving;

/* Class implementing the UCI protocol to allow GUI to communicate with the chess engine
 * The UCI protocol documentation can be found here: http://wbec-ridderkerk.nl/html/UCIProtocol.html
 */
class ChessUCI : public IPCproc {
  private:
    ChessSaving* ptosaver;
    std::string fixedhistory;
    std::string curfen;
    std::string chengname;
    std::string filename;
    std::string guimess;
    std::string enginemess;
    std::string engbestmove;
    std::string engponderon;
    std::string optname;
    std::string optvalue;
    double thinktime = 0; //in seconds
    double whtimeleft = -1; //in seconds
    double bltimeleft = -1; //in seconds
    CEGoSubcomm gosubc;
    bool usefen = false;
    
    //array for received answers;
    bool answok[8] = {false, false, false, false, false, false, false, false};

  protected:
    //full list of GUI-to-engine commands 
    static std::array<std::string, 11> guicomms;
    
    //full list of engine-to-GUI answers
    static std::array<std::string, 8> engineansw;

    //full list of option types (send by engine when describing an option)
    static std::array<std::string, 5> optiontypes;
    
    /* Nested data structures to store the parameters of the chess engine editable options */
    struct CheckOption {
      std::string name;
      bool defvalue;
      bool value;
    };
    
    struct SpinOption {
      std::string name;
      int defvalue;
      int value;
      int min;
      int max;
    };
    
    struct ComboOption {
      std::string name;
      std::string defvalue;
      std::string value;
      std::vector<std::string> allowedvals;
    };
    
    struct ButtonOption {
      std::string name;
    };
    
    struct EntryOption {
      std::string name;
      std::string defvalue;
      std::string value;
    };
    
    std::vector<CheckOption*> checkvector;
    std::vector<SpinOption*> spinvector;
    std::vector<ButtonOption*> buttonvector;
    std::vector<EntryOption*> entryvector;
    std::vector<ComboOption*> combovector;
    
    bool pondering = false; //controls pondering
    bool memorizeoption(std::string);
    
  public:
    /* can be used outside the class as a flag, never checked by methods of this class,
     * is used to check if a pondering search must be stopped and a new search shoud be started
     */
    static bool forcestopp;
    
    ChessUCI();
    ChessUCI(std::string, std::string);
    virtual ~ChessUCI();
    void setpondering(bool x) {pondering = x;}
    bool getpondering(void) {return pondering;}
    
    void setusefen(bool df) {usefen = df;}
    bool getusefen(void) {return usefen;}
    void setcename(std::string);
    std::string getcename(void) const {return chengname;} 
    
    void setchspointer(ChessSaving* pp) {ptosaver = pp;}
    void setfixedhist(std::string hh) {fixedhistory = hh;}
    void setfenchb(std::string ff) {curfen = ff;}
    std::string getpondermove(void) {return engponderon;}
    std::string getprevmoves(void);
    std::string formatprevmoves(std::string);
    std::string getmovetodo(void) {return engbestmove;}
    bool checkanswok(int i) {return answok[i];}
    void paramoptions(std::string name, std::string value) {optname = name; optvalue = value;} //set internal variables for a setoption command
    void setgosubcomm(CEGoSubcomm sc) {gosubc = sc;} //set internal variables for adding subcommands to a go command
    
    bool sendcomm(int);
    bool geteansw(bool = false);
    
    std::string gui_said(void) {return guimess;}
    std::string engine_said(void) {return enginemess;}
    
    void set_thinktime(double t) {thinktime = t;}
    void get_lefttimes(double, double);

    void writecepref(void);
    bool readcepref(void);
    void applycepref(bool);
    
    //pure virtual methods
    virtual void set_options(std::string) =0;
    virtual void ch_eng_communication(std::string) =0;
};

/* Class handling a config file, should save and read some general parameters,
 * like the path of the chess engine, if a player is usually a human or a chess engine, and other stuffs  
 */
class ChessConfig {
  public:
    typedef std::unordered_map<std::string, std::string> umapstrstr;
    typedef std::unordered_map<std::string, howplopt> howplmap;

  private:
    static std::array<std::string, 9> confkeys;
    static std::string nulltxt;
    static howplmap sethowpl;
    
    std::string configfile;
    std::string altefile;
    c_color shumpl;
    umapstrstr chessengines;
    howplopt whoishum;
    
    //in these arrays index 0 is for white, index 1 is for black, 2 is for the analyser tool (when appropriated)
    std::array<std::string, 3> chengname;
    std::array<std::string, 2> playernames;
    int gametime; //in seconds
    bool ponder;
        
  public:  
    ChessConfig();
    virtual ~ChessConfig();
    
    void initcc(std::string, std::string);
    
    bool addcheng(std::string, std::string);
    bool removecheng(std::string);
    bool setcheng(unsigned int, std::string);
    bool setifhuman(howplopt);
    void setplname(unsigned int w, std::string n) {playernames[w] = n;} //set player name
    void setgametime(int t) {gametime = t;} //set time game
    void setponder(bool p) {ponder = p;} //set ponder boolean value
    std::string getchengname(unsigned int w) const;
    std::string getchengpath(unsigned int) const;
    howplopt getifhuman(void) const {return whoishum;} //get the value of current players option
    std::array<bool, 2> getifhumanbool(c_color);
    std::string getplname(unsigned int w) const {return playernames[w];} //get player name
    const umapstrstr& getcemap(void) const {return chessengines;} //return the map
    int getgametime(void) const {return gametime;} //get time game
    bool getponder(void) {return ponder;} //get ponder boolean value
    c_color getstarthumpl(void) const {return shumpl;} //get the color of the last human player
    
    void writestarthum(c_color);
    void writedefault(void);
    void saveconf(void);
    bool readcurrent(std::ifstream&);
};

#endif
