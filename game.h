#ifndef _GAME_H
#define _GAME_H

// this is where we handle the game

#include <string.h> // strcpy
#include <stdlib.h> // free

#include "list.h"   // LIST, list_*

typedef enum {
    P_1=0,
    P_2=1
} ePlayer;

typedef enum {
    GS_INIT,    // initial state
    GS_TURN,    // turn in progress (see ePlayer to know which player turn it is)
    GS_WIN      // a player won (see ePlayer to know which one won)
} eGameState;

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

typedef struct {
    eGameState state;   // current state of the game
    ePlayer player;    // current player
    char playername[2][9];  // players names
    char gamename[64];

    LIST *turns;    // list of game turns

    time_t t_remaining;
    time_t t_total;
    time_t t_turn;
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

static inline
char *          game_get_name           (sGame *g) { return g->gamename; }
static inline
char *          game_get_playername     (sGame *g, ePlayer p) { return g->playername[p]; }
static inline
void            game_set_playername     (sGame *g, ePlayer p, char *name) { strcpy(g->playername[p], name); }
static inline
eGameState      game_get_state          (sGame *g) { return g->state; }
static inline
ePlayer         game_get_player         (sGame *g) { return g->player; }
static inline
time_t          game_get_remainingtime  (sGame *g) { return g->t_remaining; }
static inline
void            game_set_remainingtime  (sGame *g, time_t t) { g->t_remaining=t; }
static inline
time_t          game_get_totaltime      (sGame *g) { return g->t_total; }
static inline
void            game_set_totaltime      (sGame *g, time_t t) { g->t_total=t; }
static inline
time_t          game_get_turntime       (sGame *g) { return g->t_turn; }
static inline
void            game_set_turntime       (sGame *g, time_t t) { g->t_turn=t; }

static inline
LIST *          game_histo_destroylist  (LIST *l) {
    return list_destroy_full(l, (free_handler)free);
}
static inline
void            game_destroy            (sGame *g) {
    g->turns=list_destroy_full(g->turns, (free_handler)free);   // free the turns stack

    g->state=GS_INIT;
}

#endif

