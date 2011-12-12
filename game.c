#include <stdio.h>  // f*
#include <sys/types.h>  // DIR
#include <dirent.h> // opendir, readdir, struct dirent, closedir
#include <string.h> // strcasecmp

#include "list.h"   // LIST
#include "tools.h"  // exitOnErrSyst
#include "alloc.h"  // xstrdup

#include "game.h"

// creates a new game from scratch
sGame *game_new(sGame *g, const char *fname, const char *pn1, const char *pn2) {
    if(!g)
        g=xcalloc(1, sizeof(*g));

    strncpy(g->gamename, sizeof(g->gamename), fname);
    strncpy(g->playername[P_1], sizeof(*g->playername), pn1);
    strncpy(g->playername[P_2], sizeof(*g->playername), pn2);

    return g;
}

static int check_histo(char *fname) {
    FILE *f;
    long size;
    char *buf;

    if(!(f=fopen(fname, "rb")))
        return 1;

    if(fseek(f, 0, SEEK_END)) {
        fclose(f);
        return 1;
    }

    size=ftell(f);
    if(size<0) {
        fclose(f);
        return 1;
    }

    if(fseek(f, 0, SEEK_START)) {
        fclose(f);
        return 1;
    }

    buf=xmalloc(size);

// FIXME don't load the entire file in memory
    if(fread(buf, 1, size, f)!=size)
        fclose(f);
        return 1;
    }
    fclose(f);

    if(buf[0]!='H' || buf[1]!='X')
        return 1;

    xfree(buf);

    // all tests passed
    return 0;
}

// get list of valid histo files in /tmp
LIST *game_histo_getlist() {
    DIR *dir;
    struct dirent *dire;
    LIST *files=NULL;
    char *p, fpath[256];

    if(!(dir=opendir("/tmp")))
        exitOnErrSyst("opendir", "/tmp");

    errno=0;    // to check readdir errors
    while((dire=readdir(dir)) {
        p=strrchr(dire->d_name, '.'); // get the index of the last '.' character in the filename
        if(!p || strcmp(".histo", p))   // if there isn't a dot or the file extension isn't .histo, bye bye
            continue;

        fnprintf(fpath, sizeof(fpath), "/tmp/%s", dire->d_name);    // build the temporary absolute path to the histo file

        if(check_histo(fpath))  // check this is a valid .histo file
            continue;

        *p='\0';    // remove the extension from the filename

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
    char fpath[256];
    char line[256];

    fnprintf(fpath, sizeof(fpath), "/tmp/%s.histo", name);

    if(!(f=fopen(fpath, "rb")))
        return 1;

    strncpy(g->gamename, sizeof(g->gamename), name);
    g->turns=NULL;

    while(fgets(line, sizeof(line), f)) {
        if(line[0]=='H' && line[1]=='X') {
            sscanf(line+2, "%s %s %04d%04d", &g->playername[0], &g->playername[1], &g->t_total, &g->t_remaining);
            // TODO check error and return 1
        }
        else if(line[0]=='F' && line[1]=='I' && line[2]=='N') {
            // OK, TODO check magic/CRC? and return 1
            break;
        }
        else {
            sGameTurn *turn=xcalloc(1, sizeof(sGameTurn));

            char player[9];
            sscanf(line, "%04d%s %u", &turn->t_remaining, player, &turn->type); // TODO check error
            if(!strcmp(player, g->playername[0]))
                turn->player=P_1;
            else if(!strcmp(player, g->playername[1])
                turn->player=P_2;
            else {
                fclose(f);
                return 1;
            }

            g->turns=list_append(g->turns, turn);
        }
    }

    fclose(f);

    return 0;
}

// save a game to disc
int game_histo_save(const sGame *g) {
    FILE *f;
    char fpath[256];
    int i;
    LIST *turns=g->turns;

    fnprintf(fpath, sizeof(fpath), "/tmp/%s.histo", g->gamename);

    if(!(f=fopen(fpath, "wb+")))
        return 1;

    fprintf(f, "HX%s %s %04u%04u\r\n", g->playername[0], g->playername[1], g->t_total, g->t_remaining);

    while(turns) {
        fprintf(f, "%04u%s %u\r\n", ((sGameTurn *)turns->data)->t_remaining, g->playername[((sGameTurn *)turns->data)->player], ((sGameTurn *)turns->data)->type);

        turns=turns->next;
    }

    fprintf(f, "FIN\r\n");

    fclose(f);

    return 0;
}

int game_playturn(sGame *g, const sGameTurn *t) {
    g->turns = list_append(g->turns, t);    // push the turn on the stack

    g->player^=1;   // change player
}

