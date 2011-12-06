#ifndef _GAME_H
#define _GAME_H

// this is where we handle the game

#include "list.h"   // LIST

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
} sGameTurn;

typedef struct {
    eGameState state;   // current state of the game
    ePlayer player;    // current player

    LIST *turns;    // list of game turns

    time_t t_remaining;
    time_t t_total;
} sGame;

// new game from scratch
sGame *     game_new        (sGame *g, const char *name);

// game from/to .histo file
LIST *      game_histo_getlist    ();   // get list of histo files in /tmp
int         game_histo_load       (sGame *g, const char *name);
int         game_histo_save       (const sGame *g);

// manage a game
eGameState  game_state      (const sGame *g);
int         game_playturn   (sGame *g, const sGameTurn *t);

// destroy game
void        game_destroy    (sGame *g);

#endif

