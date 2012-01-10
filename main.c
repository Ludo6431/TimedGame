/************************** main.c ************************/
#include <stdio.h>

#include "menu.h"   // menu_run
#include "game.h"
#include "menu_functions.h" // ...
#include "timer.h"  // timer_stop
#include "msgs.h"   // to get the last received message
#include "longjump.h"   // long jump stuff

void _timer_conn(int sig, int t, void *data) {
    printf("\x1b[s\x1b[%d;0H", (int)data);

    printf("Il te reste %02d secondes", t);

    printf("\x1b[u");
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    eMenuState MenuState=M_MAIN;
    sGame game; // partie en cours
    sGameConf *conf = game_get_conf(&game, NULL);
    sMsg msg;
    char buf[256];
    char *choix;

    sTimer timer_glob={ 0, _timer_conn, (void *)1, &jumpenv, 10 /* FIXME */ };
    sTimer timer_conn={ 0, _timer_conn, (void *)2, &jumpenv, LJUMP_TIMER };
    sTimer timer_turn={ 0, _timer_conn, (void *)2, &jumpenv, LJUMP_TIMER };

// TODO: l'état pause doit être passé à l'autre processus pour mettre en pause son timer global

    while(1) {
        printf("\x1b[2J\x1b[0;0H");  // clear screen & go to the upper left hand corner

        // TODO: flush the input buffer before printing the new menu

        // TODO: prévoir une zone à l'écran (avant ou après le menu) dans laquelle on pourra afficher des messages
        printf("\n\n");

        if(!(choix=menu_run(MenuState, buf, sizeof(buf))) || !strlen(choix))
            continue;   // loop

        switch(choix[0]) {
        case '1':   // M_MAIN, "Nouvelle partie"
            nouvelle_partie(&game);

            timer_start(&timer_conn, 30);   // start the 30s connexion timer
            MenuState=M_WAITCON;
            break;
        case '2':   // M_MAIN, "Connexion à une partie"
            connexion(&game);
            // FIXME: si on se connecte a une partie sauvegardée qui vient d'être re-ouverte, il faut faire en sorte d'imposer au joueur qui se connecte son role ( J1 ou J2), car celui qui a re-ouvert la partie a choisi qui il était.
            MenuState=((game_get_player(&game)==game_get_me(&game))?M_MYTURN:M_HISTURN);

            timer_start(&timer_glob, conf->t_total);
            if(MenuState==M_MYTURN)
                timer_start(&timer_turn, conf->t_turn);
            break;
        case '3':   // M_MAIN, "Charger une partie sauvegardée"
            reprise_partie_sauvegarde(&game);
            timer_start(&timer_conn, 30);
            MenuState=M_WAITCON;
            break;
        case '4':   // M_MYTURN|M_HISTURN|M_PAUSED, "Stopper en sauvegardant" -> retour au menu principal
            if(MenuState==M_MYTURN)
                timer_stop(&timer_turn);
            timer_stop(&timer_glob);

            sauvegarder(&game);

            msg.type=MSG_END;
            msg.data[0]=0;  // keep histo file
            msg_send(&msg, 1);

            retour_menu(&game, 0 /* keep histo file */);

            MenuState=M_MAIN;
            break;
        case '5':   // M_MYTURN, "Mettre en pause"
            timer_pause(&timer_turn);
            timer_pause(&timer_glob);
            MenuState=M_PAUSED;
            break;
        case '6':   // M_PAUSED, "Reprendre"
            timer_resume(&timer_glob);
            timer_resume(&timer_turn);
            MenuState=((game_get_player(&game)==game_get_me(&game))?M_MYTURN:M_HISTURN);    // has to be M_MYTURN
            break;
        case '7':   // M_MYTURN|M_HISTURN|M_PAUSED, "Visualiser l'historique"
            if(MenuState==M_MYTURN)
                timer_pause(&timer_turn);
            timer_pause(&timer_glob);

            afficher_historique(&game);

            MenuState=M_PAUSED;
            break;
        case '8':   // M_MYTURN|M_HISTURN|M_PAUSED, "Stopper en visualisant l'historique" -> retour au menu principal (sans sauvegarder)
            if(MenuState==M_MYTURN)
                timer_stop(&timer_turn);
            timer_stop(&timer_glob);

            afficher_historique(&game);

            msg.type=MSG_END;
            msg.data[0]=1;  // delete histo file
            msg_send(&msg, 1);

            retour_menu(&game, 1 /* delete histo file */);

            MenuState=M_MAIN;
        case '9':   // M_*
            if(MenuState==M_MAIN) { // M_MAIN, "Quitter"
                exit(0);
            }
            else {  // M_WAITCON|M_MYTURN|M_HISTURN|M_PAUSED, "Retour au menu principal" (sans sauvegarder)
                if(MenuState==M_MYTURN)
                    timer_stop(&timer_turn);
                if(MenuState==M_WAITCON)
                    timer_stop(&timer_conn);
                else
                    timer_stop(&timer_glob);

                msg.type=MSG_END;
                msg.data[0]=1;  // delete histo file
                msg_send(&msg, 1);

                retour_menu(&game, 1 /* delete histo file */);

                MenuState=M_MAIN;
            }
            break;
        case '/':   // M_MYTURN|M_PAUSED, "Jouer un coup"/"Reprendre et jouer un coupe"
            timer_stop(&timer_turn);

            jouer_coup(&game, choix+1);
            if(game_get_state(&game)==GS_WIN) {
                timer_stop(&timer_glob);

                retour_menu(&game, 1 /* delete histo file */);

                MenuState=M_MAIN;
            }
            else
                MenuState=((game_get_player(&game)==game_get_me(&game))?M_MYTURN:M_HISTURN);

            if(MenuState==M_MYTURN)
                timer_start(&timer_turn, conf->t_turn);
            break;
        }

        // save the environment here, we will come back here if the timer expires or if we receive some messages from the other process (like end_of_game, ...)
        switch(sigsetjmp(jumpenv, 1 /* save signals mask */)) {
        case 0: // ok, environment saved
            break;
        case LJUMP_TIMER:   // M_WAITCON|M_MYTURN, timer expired
            if(MenuState!=M_WAITCON)
                timer_stop(&timer_glob);

            // TODO afficher message erreur

            msg.type=MSG_END;
            msg.data[0]=1;  // delete histo file
            msg_send(&msg, 1);   // let's tell the other process we're done

            retour_menu(&game, 1 /* delete histo file */);

            MenuState=M_MAIN;
            break;
        case LJUMP_ISR: // the other process ask me to do something
            switch(last_msg.type) {
            case MSG_JOINGAME:
                timer_stop(&timer_conn);   // ok someone is here, we can stop the connection wait timer

                MenuState=((game_get_player(&game)==game_get_me(&game))?M_MYTURN:M_HISTURN);

                timer_start(&timer_glob, conf->t_total);
                if(MenuState==M_MYTURN)
                    timer_start(&timer_turn, conf->t_turn);
                break;
            case MSG_GAMETURN:  // the other process send us the turn the player just did
                game_playturn(&game, (sGameTurn *)last_msg.data);   // play this turn locally
                if(game_get_state(&game)==GS_WIN) {
                    timer_stop(&timer_glob);

                    retour_menu(&game, 1 /* delete histo file */);

                    MenuState=M_MAIN;
                }
                else
                    MenuState=((game_get_player(&game)==game_get_me(&game))?M_MYTURN:M_HISTURN);

                if(MenuState==M_MYTURN)
                    timer_start(&timer_turn, conf->t_turn);
                break;
            case MSG_END:   // M_MYTURN|M_HISTURN|M_PAUSED, the other process quit
                if(MenuState==M_MYTURN)
                    timer_stop(&timer_turn);
                timer_stop(&timer_glob);

                retour_menu(&game, last_msg.data[0]); // we quit aswell

                MenuState=M_MAIN;
                break;
            default:
                break;
            }
            break;
        default:
            fprintf(stderr, "unhandled long jump\n");
            break;
        }
    }   // end while

    return 0;
}

