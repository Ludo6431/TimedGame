#include <stdio.h>  // f*
#include <sys/types.h>  // DIR
#include <dirent.h> // opendir, readdir, struct dirent, closedir
#include <string.h> // strcasecmp, memset
#include <errno.h>  // errno

#include "list.h"   // LIST
#include "tools.h"  // exitOnErrSyst
#include "alloc.h"  // xstrdup

#include "game.h"

// creates a new game from scratch
sGame *game_new(sGame *g, const char *fname) {
    if(!g)
        g=xmalloc(sizeof(*g));

    memset((void *)g, '\0', sizeof(*g));

    strncpy(g->conf.gamename, fname, sizeof(g->conf.gamename));

    return g;
}

char *game_get_filepath(const sGame *g) {
    static char filepath[256];

    snprintf(filepath, sizeof(filepath), "/tmp/%s.histo", g->conf.gamename);

    return filepath;
}

int game_histo_check(char *name) {
    sGame game;

    if(game_histo_load(&game, name))
        return 1;

    game_destroy(&game);

    return 0;
}

// get list of valid histo files in /tmp
LIST *game_histo_getlist() {
    DIR *dir;
    struct dirent *dire;
    LIST *files=NULL;
    char *p;

    if(!(dir=opendir("/tmp")))
        exitOnErrSyst("opendir", "/tmp");

    errno=0;    // to check readdir errors
    while((dire=readdir(dir))) {
        p=strrchr(dire->d_name, '.'); // get the index of the last '.' character in the filename
        if(!p || strcmp(".histo", p))   // if there isn't a dot or the file extension isn't .histo, bye bye
            continue;

        *p='\0';    // remove the extension from the filename

        if(game_histo_check(dire->d_name))  // check this is a valid .histo file
            continue;

        files=list_append(files, (void *)xstrdup(dire->d_name));    // append game name without extension
    }
    if(errno)
        exitOnErrSyst("readdir", "/tmp");

    if(closedir(dir))
        exitOnErrSyst("closedir", "/tmp");

    return files;
}

// load a histo file in memory
int game_histo_load(sGame *g, const char *name) {
    FILE *f;
    char line[256];
    int ret, ffull=0;
    sGameConf *conf=&g->conf;
    sGameState *state=&g->st;

    memset(g, '\0', sizeof(*g));

    strncpy(conf->gamename, name, sizeof(conf->gamename));
    g->turns=NULL;

    if(!(f=fopen(game_get_filepath(g), "rb")))  // the filepath is calculated with the gamename
        return 1;

    while(fgets(line, sizeof(line), f)) {
        if(line[0]=='H' && line[1]=='X') {  // get game configuration
            sscanf(line+2, "%s", conf->playername[P_1]);
            sscanf(line+11, "%s", conf->playername[P_2]);
            ret=sscanf(line+20, "%04d%04d", (int *)&conf->t_total, (int *)&conf->t_turn);
            if(ret!=2) {
                fclose(f);
                return 1;
            }
        }
        else if(line[0]=='F' && line[1]=='I' && line[2]=='N' && line[3]=='H' && line[4]=='X') {
            // OK, TODO check magic/CRC? and return 1 if fail
            ffull=1;
            break;
        }
        else {  // get turn
            sGameTurn *turn=xcalloc(1, sizeof(sGameTurn));

            char player[9];
            ret=sscanf(line, "%04d%s %u", (int *)&turn->t_remaining, player, &turn->type);
            if(ret!=3) {
                fclose(f);
                return 1;
            }

            if(!strcmp(player, conf->playername[P_1]))
                turn->player=P_1;
            else if(!strcmp(player, conf->playername[P_2]))
                turn->player=P_2;
            else {
                fclose(f);
                return 1;
            }

            g->turns=list_append(g->turns, turn);
        }
    }

    fclose(f);

    if(!ffull)
        return 1;

    // set initial state based on the last played turn
    if(g->turns) {
        LIST *tmp=g->turns;
        sGameTurn *turn;

        while(tmp->next)    // get last turn
            tmp=tmp->next;

        // get curr player (next player to play)
        turn=(sGameTurn *)tmp->data;
        switch(turn->type) {
        case T_OK:  // le dernier tour a été bien fait par turn->player, c'est donc à l'autre
            state->pcurr=!turn->player;
            break;
        case T_INVALID: // le dernier tour a été mal fait par turn->player, c'est donc toujours à lui
            state->pcurr=turn->player;
            break;
        case T_WIN: // wtf, ended games shouldn't be there...
            g->st.state=GS_WIN;
            state->pcurr=turn->player;
            break;
        default:
            break;
        }

        // get remaining time
        state->t_remaining=turn->t_remaining;
    }

    return 0;
}

// save a game to disc
int game_histo_save(const sGame *g) {
    FILE *f;
    LIST *turns=g->turns;

    if(!(f=fopen(game_get_filepath(g), "wb+")))
        return 1;

    fprintf(f, "HX%-8s %-8s %04u%04u\r\n", g->conf.playername[0], g->conf.playername[1], (unsigned int)g->conf.t_total, (unsigned int)g->conf.t_turn);

    while(turns) {
        fprintf(f, "%04u%s %u\r\n", (unsigned int)((sGameTurn *)turns->data)->t_remaining, g->conf.playername[((sGameTurn *)turns->data)->player], ((sGameTurn *)turns->data)->type);

        turns=turns->next;
    }

    fprintf(f, "FINHX\r\n");

    fclose(f);

    return 0;
}

int game_playturn(sGame *g, const sGameTurn *t) {
    if(g->st.state!=GS_INIT && g->st.state!=GS_TURN)
        return 1;

    g->st.state = GS_TURN;
    g->st.t_remaining = t->t_remaining;

    g->turns = list_append(g->turns, xmemdup((void *)t, sizeof(*t)));    // push the turn on the stack

    if(t->type==T_WIN)
        g->st.state=GS_WIN;

    if(t->type==T_OK)
        g->st.pcurr^=1;   // change current player

    return 0;
}

