#ifndef _GAME_H
#define _GAME_H

// this is where we handle the game

#include <string.h> // strcpy
#include <stdlib.h> // free
#include <time.h>   // time_t

#include "list.h"   // LIST, list_*

typedef enum {
    P_1=0,  // de sorte que !P_2 == P_1 et !P_1 == P_2
    P_2=1
} ePlayer;

typedef enum {
    T_OK,
    T_INVALID,
    T_WIN
} eTurnTypes;

typedef struct {    // define a turn of the game
    ePlayer player;

    eTurnTypes type;

    time_t t_remaining;
} sGameTurn;

// stocke la configuration de la partie => statique
typedef struct {
    char gamename[64];  // game name
    char playername[2][9];  // players names
    time_t t_total;
    time_t t_turn;

    ePlayer phost, pjoin;
} sGameConf;

typedef enum {
    GS_INIT,    // initial state
    GS_TURN,    // turn in progress (see ePlayer to know which player turn it is)
    GS_WIN      // a player won (see ePlayer to know which one won)
} eGameState;

// stocke le statut du jeu => évolue au cours de la partie
typedef struct {
    eGameState state;
    ePlayer pcurr;
    time_t t_remaining;
} sGameState;

typedef struct {    // initial state of the game, sent to the new user
    sGameConf conf;
    sGameState st;
} sGameInit;

typedef struct {
    // shared data
    sGameConf conf; // configuration of the game
    sGameState st; // real-time status of the game

    // local data
    ePlayer pme;
    LIST *turns;    // list of game turns
} sGame;

// new game from scratch
sGame *     game_new                (sGame *g, const char *name);
char *      game_get_filepath       (const sGame *g);

// game from/to .histo file
LIST *      game_histo_getlist      ();   // get list of histo files in /tmp
// LIST *   game_histo_destroylist  (LIST *l);

int         game_histo_check        (char *filepath);
int         game_histo_load         (sGame *g, const char *name);
int         game_histo_save         (const sGame *g);

// manage a game
int         game_playturn           (sGame *g, const sGameTurn *t);

// destroy game
// void     game_destroy            (sGame *g)


// static functions declarations:
// game configuration
static inline
sGameConf *     game_get_conf           (sGame *g, sGameConf *c) { if(c) { memcpy(c, &g->conf, sizeof(*c)); return c; } else return &g->conf; }
static inline
void            game_set_conf           (sGame *g, sGameConf *c) { memcpy(&g->conf, c, sizeof(*c)); }

// game state
static inline
sGameState *    game_get_state          (sGame *g, sGameState *s) { if(s) { memcpy(s, &g->st, sizeof(*s)); return s; } else return &g->st; }
static inline
void            game_set_state          (sGame *g, sGameState *s) { memcpy(&g->st, s, sizeof(*s)); }

static inline   // Is it my turn?
int             game_isit_myturn        (sGame *g) { return g->pme==g->st.pcurr; }
static inline   // Am I host?
int             game_ami_host           (sGame *g) { return g->pme==g->conf.phost; }

static inline
LIST *          game_histo_destroylist  (LIST *l) {
    return list_destroy_full(l, (free_handler)free);
}
static inline
void            game_destroy            (sGame *g) {
    g->turns=list_destroy_full(g->turns, (free_handler)free);   // free the turns stack

    g->st.state=GS_INIT;
}

#endif

