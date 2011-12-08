#include <stdio.h>  // f*
#include <sys/types.h>  // DIR
#include <dirent.h> // opendir, readdir, struct dirent, closedir
#include <string.h> // strcasecmp

#include "list.h"   // LIST
#include "tools.h"  // exitOnErrSyst
#include "alloc.h"  // xstrdup

#include "game.h"

sGame *game_new(sGame *g, const char *fname){
	int cle,id; 
	sShm* adresse;

	printf("Entrez votre nom :\n")
	fgets(g.playername[1][],50,stdin);
	
    /*creation de la clee*/

    cle=ftok(fname,0));

    id=ouverture_memoire_partage(cle);

    adresse=attachement_memoire_partage(id);
	
	shmlock(adresse);
	
    printf("Entrez la durée de la partie (en secondes) : \n")
	scanf("%d", g->t_total);
	g.t_remaining=g.t_total;
    printf("Entrez le temps alloué à un coup : \n")
	scanf("%d", g->t_turn);
	
	g.state=GS_INIT;
	
	g.player=P_1;
	printf("Vous etes le joueur n°1 \n")
	shmunlock(adresse);
 //remarque : demander tout au debut, puis  lock,  creation, attachement, copie unlock
}

int check_histo(char *fname) {
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
LIST *game_histo_getlist() {   // get list of histo files in /tmp
    DIR *dir;
    struct dirent *dire;
    LIST *files=NULL;
    char fpath[256];

    if(!(dir=opendir("/tmp")))
        exitOnErrSyst("opendir", "/tmp");

    errno=0;    // to check readdir errors
    while((dire=readdir(dir)) {
        int len=strlen(dire->d_name);
        if(!strcasecmp(".histo", dire->d_name[len-6])) {
            fnprintf(fpath, sizeof(fpath), "/tmp/%s", dire->d_name);

            if(!check_histo(fpath))
                files=list_append(files, xstrdup(dire->d_name));
        }
    }
    if(errno)
        exitOnErrSyst("readdir", "/tmp");

    if(closedir(dir))
        exitOnErrSyst("closedir", "/tmp");

    return files;
}

int game_histo_load(sGame *g, const char *name) {
    FILE *f;
    char fpath[256];
    char line[256];
    int i;

    fnprintf(fpath, 256, "/tmp/%s", name);

    if(!(f=fopen(fpath, "rb")))
        return 1;

    g->turns=NULL;

    while(i++, fgets(line, sizeof(line), f)) {
        if(line[0]=='H' && line[1]=='X') {
            sscanf(line+2, "%s %s %04d%04d", &g->playername[0], &g->playername[1], &g->t_total, &g->t_remaining);
            // TODO check error and return 1
        }
        else if(line[0]=='F' && line[1]=='I' && line[2]=='N') {
            // OK, TODO check magic/CRC? and return 1
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

int game_histo_save(const sGame *g) {
    FILE *f;
    char fpath[256];
    int i;
    LIST *turns=g->turns;

    fnprintf(fpath, sizeof(fpath), "/tmp/%s", name);

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

