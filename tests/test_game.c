#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "test.h"

#include "../list.h"
#include "../game.h"

// gcc -Wall -o test_game -I.. ../list.c ../tools.c ../game.c test_game.c

void write_sample(char *path) {
    int i=0, timer=60;
    char players[][9] = { "ludo", "cedric" };
    FILE *fd;

    fd=fopen(path, "wb+");
    assert(fd);

    fprintf(fd, "HX%-8s %-8s %04d0020\r\n", players[0], players[1], timer);

    while(++i<10)
        fprintf(fd, "%04u%s %u\r\n", timer-=5, players[i&1], T_OK);
    fprintf(fd, "%04u%s %u\r\n", timer-=5, players[i&1], T_INVALID);
    fprintf(fd, "%04u%s %u\r\n", timer-=5, players[i&1], T_OK);

    fprintf(fd, "FIN\r\n");

    fclose(fd);
}

int main() {
    sGame game;
    LIST *l, *ltmp;
    int found, count;

    write_sample("/tmp/test_game.histo");

    test(game_new(&game, "test_game")==&game);

    test(!strcmp(game_get_filepath(&game), "/tmp/test_game.histo"));

    game_destroy(&game);

    test((l=ltmp=game_histo_getlist())!=NULL);
    found=0;
    while(ltmp) {
        if(!strcmp((char *)ltmp->data, "test_game"))
            found=1;
        
        ltmp=ltmp->next;
    }
    test(found==1);

    game_histo_destroylist(l);

    test(!game_histo_load(&game, "test_game"));

    test(!strcmp(game.conf.playername[0], "ludo"));
    test(!strcmp(game.conf.playername[1], "cedric"));
    test(game.conf.t_total==60);
    test(game.conf.t_turn==20);

    test(game.st.pcurr==P_2);
    test(game.st.t_remaining==5);
    test(game.turns!=NULL)

    count=0;
    for(ltmp=game.turns; ltmp; ltmp=ltmp->next) {
        sGameTurn *turn = (sGameTurn *)ltmp->data;

        count++;

        test(turn->t_remaining==60-5*count);
        if(count<10) {
            test(turn->player==(count&1));
            test(turn->type==T_OK);
        }
        else if(count==10) {
            test(turn->player==0);
            test(turn->type==T_INVALID);
        }
        else if(count==11) {
            test(turn->player==0);
            test(turn->type==T_OK);
        }
    }
    test(count==11);

    game_destroy(&game);

    unlink("/tmp/test_game.histo");

    printf("All tests passed !\n");

    return 0;
}

