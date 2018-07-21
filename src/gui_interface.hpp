/*
 * gui_interface.hpp
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


#ifndef GUI_INTERFACE_H_DEF
#define GUI_INTERFACE_H_DEF 1

#include <gtkmm.h>

#include "chessboard.hpp"

//defined signals, declared here as global variables
typedef sigc::signal<void, bool> type_sm;
typedef sigc::signal<bool, c_color, ChessSquare*> sig_firstsel;
typedef sigc::signal<void, c_color, bool, bool> sig_gameend;
typedef sigc::signal<void, std::string> sig_displaymessage;


/* Class Child of ChessPGN, its purpose is to interact with the user in case a PGN file loaded has more than one game
 * so that the user can choose (through the selectgame method) which game should be loaded.
 */
class ChessPGNGui : public ChessPGN {
  private:
    bool gameok;
    
    /* Nested class to implement the graphical interface.
     * This builds the dialog where the user can choose the game to be shown. 
     */
    class PGNselgame : public Gtk::Dialog {
      private:
        ChessPGNGui* ptopgn;
        unsigned int chogame;
        bool chmade;
                
        Gtk::Box* mainbox = get_content_area();
        Gtk::ScrolledWindow container;
        Gtk::TreeView insidetv;
        Glib::RefPtr<Gtk::ListStore> insidels;
        Gtk::ButtonBox bbox;
        Gtk::Button bclose, bopen;
      
        //nested class of nested class defining the model for the game list
        class PGNgamemodel : public Gtk::TreeModelColumnRecord {
          public:
            //instances of the class, which are the columns of the model
            Gtk::TreeModelColumn<unsigned int> col_index;
            Gtk::TreeModelColumn<Glib::ustring> col_event;
            Gtk::TreeModelColumn<Glib::ustring> col_site;
            Gtk::TreeModelColumn<Glib::ustring> col_date;
            Gtk::TreeModelColumn<Glib::ustring> col_round;
            Gtk::TreeModelColumn<Glib::ustring> col_white;
            Gtk::TreeModelColumn<Glib::ustring> col_black;
            Gtk::TreeModelColumn<Glib::ustring> col_result;
      
            PGNgamemodel() {//in this way the constructor add its instances (columns) as childs 
              add(col_index); add(col_event); add(col_site); add(col_date);
              add(col_round); add(col_white); add(col_black); add(col_result);
            }
        };
        
        PGNgamemodel modelgamelist;
         
        void buildlist(void);
      
      public:
        PGNselgame();
        PGNselgame(ChessPGNGui*);
        ~PGNselgame();

        unsigned int chosen(void) {return chogame;}
        bool choicemade(void) {return chmade;}
        
        //signal handlers
        void on_clicked_button(bool);
    };

  public:
    ChessPGNGui(std::string, char);
    ~ChessPGNGui();
    
    unsigned int selectgame(void) override;
    bool isgameok(void) {return gameok;}
};


/* Class Child of ChessUCI
 */
class ChessUCIGui : public ChessUCI {
  private:
    sig_displaymessage displaymess;
    
    /* Nested class to implement the graphical interface.
     * As the class should work as the base class for most of its purposes,
     * it is better to place the GUI implementation for setting the chess engine options in a nested class
     */
    class ChessEngOptGui : public Gtk::Dialog {
      private:
        ChessUCIGui* ptoucigui;
        
        Gtk::Box* mainbox = get_content_area();
        Gtk::Box lastbox;
        Gtk::Button applybut, godefbut, savebut;
        /* As we cannot know the number of options during compilation,
         * the child widgets relative to the options will be implemented
         * with the use of Gtk::manage in the costructor
         * Other instances (not gtkmm widget) have to be managed with new and delete, from here the pointer vectors
         */
         
        //nested class of nested class defining a simple model for the combo options
        class ComboModel : public Gtk::TreeModelColumnRecord {
          public:
            Gtk::TreeModelColumn<Glib::ustring> optval;
            
            ComboModel() {//in this way the constructor add its instances (columns) as childs 
              add(optval);
            }
        };

        std::vector<ComboModel*> storecombomodels;
        
      public:
        ChessEngOptGui();
        ChessEngOptGui(const ChessEngOptGui&);
        ChessEngOptGui(ChessUCIGui*, std::string);
        ~ChessEngOptGui();
        
        //option signal handlers
        void on_type_check_toggled(CheckOption*, Gtk::CheckButton*);
        void on_type_spin_changed(SpinOption*, Gtk::SpinButton*);
        void on_type_combo_changed(ComboOption*, ComboModel*, Gtk::ComboBox*);
        void on_type_button_clicked(ButtonOption*);
        void on_type_entry_changed(EntryOption*, Gtk::Entry*);
        
        //other signal handlers
        void on_abutton_clicked(bool);
        void on_save_pref_clicked(void);
    };

  public:
    ChessUCIGui();
    ChessUCIGui(std::string, std::string);
    ~ChessUCIGui();
        
    //public methods wrapping the private signals
    sig_displaymessage send_cemessage(void) {return displaymess;}
    
    //overriding pure virtual methods
    void set_options(std::string) override;
    void ch_eng_communication(std::string) override;
};


/* Empty declaration to resolve circular dependencies
 * The class will be defined later in this file
 */
class ChessBoardGui;

/* Class Child of ChessPlayer
 */
class ChessPlayerGui : public ChessPlayer {
  private:
    type_sm status_move;
    sig_firstsel status_first_sel;    
    
  public:
    ChessPlayerGui();
    ChessPlayerGui(c_color);
    ChessPlayerGui(c_color, bool);
    ~ChessPlayerGui();
    
    //public methods wrapping the private signals
    type_sm send_status_signal(void) {return status_move;}
    sig_firstsel send_firstsel_signal(void) {return status_first_sel;}

    void binding_signals(void); //connecting the signals

    //signal handlers
    void on_status_event_sent(bool);
    bool on_firstsel_event_sent(c_color, ChessSquare*);

    //public wrapper for inherited protected method (which should stay protected)
    void wrappersethistoryfn(std::string fn) {sethistoryfn(fn);}
    void wrappersetsavefn(std::string fn) {setsavefn(fn);}

    bool player_act(void) override;
};


/* Class to build a square of the chessboard
 * the class store only the graphical aspects and the event driven method, to manipulate pieces and squares
 */
class ChessSquareGui : public Gtk::Frame {
  private:
    c_color col;
    int x;
    int y;
    
    Glib::RefPtr<Gtk::CssProvider> square_cssprov;
    Gtk::EventBox eventsq;
    Gtk::Image insidesq;
    Gtk::Menu popm;
    
    ChessBoardGui* ownboard = nullptr;

  public:
    bool selected = false;
    static bool allowclick;

    ChessSquareGui();
    ChessSquareGui(c_color, ChessBoardGui*, int, int);
    ~ChessSquareGui();
    
    //methods
    void setcolor(c_color);
    void setcoord(int, int);
    void setpointer(ChessBoardGui* pointer) {ownboard = pointer;}
    int getx(void) {return x;}
    int gety(void) {return y;}
    ChessBoardGui* getpboard(void) {return ownboard;}

    void showpiece(Piece*);
    bool applyselect(bool);
    void deassociate_from_board(void) {ownboard = nullptr;}
    
    //signal handler
    bool onclick(GdkEventButton*);
    void onpopmenacing(bool);
    void onpopmenacedby(bool);
};


/* Empty declaration to resolve circular dependencies
 * The class will be defined later in this file
 */
class ChessWindowGui;

/* Class to show a timer for time game
 */
class ChessTimer : public Gtk::Frame {
  private:
    c_color color;
    int chesstimer; //timer in seconds
    bool running;
    ChessWindowGui* ptowin = nullptr;
    
    Gtk::Label tdisplay;
    Glib::RefPtr<Gtk::CssProvider> t_cssprov;
    
    sigc::connection tconn; //connection to the callback method called on timeout

    sig_gameend endontime;
    
    std::string formattimer(int);
    void updatetimerlab(void);
    
    //callback method called on timeout
    bool updatetimerval(void);
    
  public:
    ChessTimer();
    ~ChessTimer();
    
    c_color getcolor(void) {return color;}
    void init(ChessWindowGui*, c_color, int);
    int gettime(void) {return chesstimer;}
    void deltimer(void);
    void stopgotimer(void);
};


/* Class implementing the graphical interface of the game
 */
class ChessWindowGui : public Gtk::Window {
  private:
    typedef std::array<Gtk::Label, 8> labelarr;
    ChessConfig* cnfgf;
    ChessUCIGui* ucianalys;
    c_color starthumpl;
    
    Gtk::Box mainbox = Gtk::Box(Gtk::ORIENTATION_VERTICAL);
    Gtk::Box upperbar = Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
    ChessTimer whtimer, bltimer;
    
    Gtk::Box centralgroup = Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
    Gtk::Box rightpanel = Gtk::Box(Gtk::ORIENTATION_VERTICAL);
    
    Gtk::Box upxlabelbox = Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
    Gtk::Box leylabelbox = Gtk::Box(Gtk::ORIENTATION_VERTICAL);
    Gtk::Box loxlabelbox = Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
    Gtk::Box riylabelbox = Gtk::Box(Gtk::ORIENTATION_VERTICAL);
        
    Gtk::ScrolledWindow sidecontainer, downcontainer;
    Gtk::TextView textsidearea, textdownarea;
    Glib::RefPtr<Gtk::TextBuffer> sbuffer = textsidearea.get_buffer(); //linking the pointer to buffer to the textarea Gtk::TextBuffer 
    Gtk::TextIter txtspos = sbuffer->end(); //setting the text iterator to the end of the empty buffer
    Glib::RefPtr<Gtk::TextBuffer> dbuffer = textdownarea.get_buffer(); //linking the pointer to buffer to the textarea Gtk::TextBuffer 
    Gtk::TextIter txtdpos = dbuffer->end(); //setting the text iterator to the end of the empty buffer
    
    Gtk::Grid wideboard; //is the chessboard with the coordinate labels
    Gtk::Grid board; //the chessboard without the labels (only the squares)
    labelarr xlaba, ylaba, xlabb, ylabb;
    
    std::array<std::array<ChessSquareGui, 8>, 8> graphsq;  //the frames constituting the chessboard
    ChessBoardGui* pgameboard = nullptr;
    
    Gtk::Box sidebara = Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
    Gtk::Box sidebarb = Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
    Gtk::Box sidebarc = Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
            
    Gtk::ButtonBox actionbuttons;
    Gtk::Button butnew, butstart, butback, butforward, butresign, butstopcheng, butanalyse;
    Gtk::Label labttime;
    Gtk::SpinButton spbttime;
    Gtk::CheckButton limittime;
    
    ChessSquareGui whopl;
    
    Glib::RefPtr<Gtk::Builder> menubuilder;
    Glib::RefPtr<Gio::SimpleActionGroup> menuactiongroup;
    
    //menu signal handlers
    void on_action_game_new(std::string = "");
    void on_action_game_load(void);
    void on_action_game_readfen(void);
    void on_action_game_save(void);
    void on_action_game_close(void);
    void on_action_game_quit(void);
    void on_action_game_preferences(void);

    void on_action_game_start(void);    
    void on_action_game_back(int = 1);
    void on_action_game_forward(int = 1);
    void on_action_game_resign(void);
        
    void on_action_setceoptions(c_color);
    void on_toggled_cetime_button(void);
    void on_set_cetime(void);
    void on_stop_chess_engine(void);
    void on_chess_engine_suggest(void);
    void on_chess_engine_analyse(void);

    void on_action_game_history_save(bool);
    void on_action_game_history_show(bool);
    void on_action_gen_fen(void);
    
    void on_action_printabout(void);
    void on_action_printhelp(void);
    
  public:
    ChessWindowGui();
    ChessWindowGui(const ChessWindowGui&); //will not be defined
    ChessWindowGui(const Glib::RefPtr<Gtk::Application>& app);
    ~ChessWindowGui();
        
    ChessSquareGui* getsquaregui(int a, int b) {return &graphsq[a][b];}
    ChessConfig getconfclass(void) {return *cnfgf;}
    Gtk::Grid* getptoboard(void) {return &board;}
    void doprintmess(std::string, bool);
    void displaycemess(std::string);
    void clearboard(void);
    
    void showwhom(c_color);
    void presstimer(void);
    std::array<int, 2> gettimers(void);
    void setbaftermove(void);

    void on_game_end(c_color, bool, bool);
};


/* Class implementing the virtual functions of the chessboard for the graphical game
 */
class ChessBoardGui : public ChessBoard {
  private:
    ChessWindowGui* pwindow;
    sig_gameend gameend;
    bool isawinner;

  public:  
    ChessBoardGui(std::string);
    ChessBoardGui(ChessWindowGui*, ChessPlayerGui*, ChessPlayerGui*, ChessUCIGui*, ChessUCIGui*, std::string);
    ~ChessBoardGui();
    
    bool check_startinggui(c_color, ChessSquare*);
    void gather_players(ChessPlayerGui*, ChessPlayerGui*);
    void guisavegame(std::string); //wrapper for load game not needed, is inherited by ChessBoard class
    void selectguisquares(std::vector<ChessSquare*>);
    void unselectallsq(void);
    void setceoptions(c_color);
    
    //public methods wrapping the private signals
    sig_gameend send_gameend_signal(void) {return gameend;}
    
    //overriding virtual methods
    void printmess(bool = false) override;
    bool turnation(void) override;
    void printcb(void) override;
    void turnation_end(bool) override;
    wpiece choose_promotion(c_color) override;
    bool askdraw(int) override;
    std::string printhist(std::string, bool = false) override;
    void letgenupdate(void) override;
};

/* Class implementing the popup window to be shown for pawn promotion (choose the piece)
 */
class ChessPromotionGui : public Gtk::Dialog {
  private:
    Gtk::Box* mainbox = get_content_area();
    Gtk::Label explain;
    Gtk::ButtonBox selpiece = Gtk::ButtonBox(Gtk::ORIENTATION_HORIZONTAL);
    
    Gtk::Image rockim, knightim, bishopim, queenim;
    Gtk::Button rockb, knightb, bishopb, queenb;
    
  public:
    wpiece pselected = generic;
    
    ChessPromotionGui(ChessWindowGui*, c_color);
    ChessPromotionGui(const ChessPromotionGui&);
    ~ChessPromotionGui();
        
    //signal handler
    void on_choice_clicked(wpiece);
};

/*Class implementing the popup window to ask if the draw is accepted
 */
class ChessAskyn : public Gtk::Dialog {
  private:
    bool answer;

    Gtk::Box* mainbox = get_content_area();
    Gtk::Label explain;
    Gtk::ButtonBox getansw = Gtk::ButtonBox(Gtk::ORIENTATION_HORIZONTAL);
    
    Gtk::Button yesbutton, nobutton;
    
  public:    
    ChessAskyn(Gtk::Window*, std::string, std::string);
    ChessAskyn(const ChessAskyn&);
    ~ChessAskyn();
    
    bool getanswer(void) {return answer;}

    //signal handler
    void on_yesno_clicked(bool);
};

/*Class implementing a dialog to get and pass the FEN notation
 */
class ChessgetFEN : public Gtk::Dialog {
  private:
    bool answer = false;
    std::string fenrep;
    
    Gtk::Box* mainbox = get_content_area();
    Gtk::Label descr;
    Gtk::Entry fenent;
    Gtk::Button cancel, confirm;
    Gtk::ButtonBox buttons;
    
  public:
    ChessgetFEN(Gtk::Window*);
    ChessgetFEN(const ChessgetFEN&);
    ~ChessgetFEN();
    
    bool getanswer(void) {return answer;}
    std::string getfenrep(void) {return fenrep;}
    void on_button_clicked(bool); //signal handler
};

/*Class implementing the popup window which set the preferences (config file)
 */
class ChessPreferGui : public Gtk::Dialog {
  private:
    ChessConfig* confmanager;
    
    Gtk::Box* mainbox = get_content_area();
    Gtk::Grid gridcontainer;

    Gtk::Label topl, timel, whoplayl, namel_w, namel_b, tlab_a, tlab_b;
    Gtk::Entry cenamee, cepathe, whname, blname;
    Gtk::Button ceaddb, fipathb, sedefb, savecb;
    Gtk::ScrolledWindow scwinfortable;
    Gtk::TreeView cetabletv;
    Glib::RefPtr<Gtk::ListStore> cetablels;
    Gtk::ComboBox gttablecb, whoplaytablecb;
    Glib::RefPtr<Gtk::ListStore> gttablels;
    Glib::RefPtr<Gtk::ListStore> whoplaytablels;
    Gtk::CheckButton checkb_ponder;

    Gtk::Menu popmenu;

    //Nested class for the model columns storing the chess engines:
    class ModelEngColumns : public Gtk::TreeModelColumnRecord {
      public:
        //instances of the class, which are the columns of the model
        Gtk::TreeModelColumn<Glib::ustring> col_name;
        Gtk::TreeModelColumn<Glib::ustring> col_path;
        Gtk::TreeModelColumn<Glib::ustring> col_usedby;
      
        ModelEngColumns() {//in this way the constructor add its instances (columns) as childs 
          add(col_name); add(col_path); add(col_usedby);
        }
    };
    
    ModelEngColumns cetablemodel;
    
    //Nested class for the model columns storing the options for game time
    class ModelTimeColumns : public Gtk::TreeModelColumnRecord {
      public:
        //instances of the class, which are the columns of the model
        Gtk::TreeModelColumn<Glib::ustring> col_name;
        Gtk::TreeModelColumn<int> col_value;

        ModelTimeColumns() {
          add(col_name); add(col_value);
        }
    };
    
    ModelTimeColumns gttablemodel;
    
    //Nested class for the model colums storing the human / chess engine player options
    class ModelWhoplayColumns : public Gtk::TreeModelColumnRecord {
      public:
        //instances of the class, which are the columns of the model
        Gtk::TreeModelColumn<Glib::ustring> col_name;
        Gtk::TreeModelColumn<howplopt> col_value;
        
        ModelWhoplayColumns() {
          add(col_name); add(col_value);
        }
    };
    
    ModelWhoplayColumns whoplaymodel;
    
    void populatetreeview(void);
    void populatecomboboxtime(void);
    void populatecomboboxwhoplay(void);
    bool setcbtimeval(void);
    bool setcbwhopval(void);
    void setnamesasconf(Gtk::Entry&, unsigned int);
    void setponderasconf(void);

  public:
    ChessPreferGui(ChessWindowGui*, ChessConfig*);
    ChessPreferGui(const ChessPreferGui&); //will not be defined
    ~ChessPreferGui();
    
    //signal handlers for the buttons in the dialog
    void on_add_button_clicked(void);
    void on_save_custom_clicked(void);
    void on_set_default_clicked(void);
    void on_find_path_clicked(Gtk::Entry*);
    
    //signal handlers for the popup menu, to show it and act when an entry of the menu is selected
    void on_click_in_treeview(GdkEventButton*);
    void on_popup_menu_setfor(unsigned int);
    void on_popup_menu_remove(void);
    void on_popup_menu_options(void);
    
    //signal handler for the comboboxes
    void on_timecombobox_changed(void);
    void on_whoplaycombobox_changed(void);
    
    //signal handler for the ponder checkbutton
    void on_ponder_checkbutton_toggled(void);
    
    //signal handler for the entry holding the player name
    void on_entry_changed(unsigned int);
};

/*Class implementing the popup window which allow to analyse the game
 */
class ChessAnalysisGui : public Gtk::Dialog {
  private:
    ChessUCIGui* analys;
    Gtk::Box* mainbox = get_content_area();
    Gtk::Button suggm, ceopt, stopce;
    Gtk::ButtonBox lbuttons = Gtk::ButtonBox(Gtk::ORIENTATION_HORIZONTAL);
    Gtk::Grid rows;
    Gtk::Label searchmovlab;
    Gtk::CheckButton timecbu, depthcbu, nodecbu, matecbu;
    Gtk::Entry searchmovent;
    Gtk::SpinButton timespi, depthspi, nodespi, matespi;
    Gtk::ScrolledWindow container;
    Gtk::TextView infoarea;
    Glib::RefPtr<Gtk::TextBuffer> ibuffer = infoarea.get_buffer(); //linking the pointer to buffer to the textarea Gtk::TextBuffer 
    Gtk::TextIter txtipos = ibuffer->end(); //setting the text iterator to the end of the empty buffer
    
    //signal handlers
    void on_setoptions_clicked(void);
    void on_suggest_move_clicked(void);
    void on_stop_chess_engine(void);

    void displaycemess(std::string);
    
  public:
    ChessAnalysisGui(ChessWindowGui*, ChessUCIGui*);
    ChessAnalysisGui(const ChessAnalysisGui&); //will not be defined
    ~ChessAnalysisGui();
};

#endif
