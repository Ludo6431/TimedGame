/************************** main.c ************************/
#include <stdio.h>

#include "menu.h"   // menu_run
#include "menu_functions.h" // ...
#include "timer.h"  // timer_stop
#include "msgs.h"   // to get the last received message
#include "longjump.h"   // long jump stuff

int main(int argc, char *argv[]) {
    eMenuState MenuState=M_MAIN;
    sGame game; // partie en cours
    sMsg msg;
    char buf[256];
    char *choix;

    while(1) {
        printf("\x1b[2J\x1b[0;0H");

        // TODO: flush the input buffer before printing the new menu
        // TODO: prévoir une zone à l'écran (avant ou après le menu) dans laquelle on pourra afficher des messages

        if(!(choix=menu_run(MenuState, buf, sizeof(buf))) || !strlen(choix))
            continue;   // loop

        timer_stop();

        switch(choix[0]) {
        case '1':   // M_MAIN, "Nouvelle partie"
            nouvelle_partie(&game);
            MenuState=M_WAIT;
            break;
        case '2':   // M_MAIN, "Connexion à une partie"
            connexion(&game);
            MenuState=((game_get_player(&game)==game_get_me(&game))?M_MYTURN:M_HISTURN);
//            if(MenuState==M_MYTURN)
                // TODO: timer_start(conf->t_turn...
            break;
        case '3':   // M_MAIN, "Charger une partie sauvegardée"
//            reprise_partie_sauvegarde(&game);
            MenuState=M_WAIT;
            break;
        case '4':   // M_MYTURN|M_HISTURN|M_PAUSED, "Stopper en sauvegardant" -> retour au menu principal
//            sauvegarder(&game);

            msg.type=MSG_ENDGAME;
            msg_send(&msg, 0);

            retour_menu(&game);
            MenuState=M_MAIN;
            break;
        case '5':   // M_MYTURN, "Mettre en pause"
//            pause(&game);
            MenuState=M_PAUSED;
            break;
        case '6':   // M_PAUSED, "Reprendre"
//            reprendre(&game);
            MenuState=((game_get_player(&game)==game_get_me(&game))?M_MYTURN:M_HISTURN);
//            if(MenuState==M_MYTURN)
                // TODO: timer_resume
            break;
        case '7':   // M_MYTURN|M_HISTURN|M_PAUSED, "Visualiser l'historique"
//            afficher_historique(&game);
            // on ne change pas d'état
            break;
        case '8':   // M_MYTURN|M_HISTURN|M_PAUSED, "Stopper en visualisant l'historique" -> retour au menu principal (sans sauvegarder)
//            afficher_historique(&game);

            msg.type=MSG_ENDGAME;
            msg_send(&msg, 0);

            retour_menu(&game);
            MenuState=M_MAIN;
        case '9':   // M_*
            if(MenuState==M_MAIN) { // M_MAIN, "Quitter"
                exit(0);
            }
            else {  // M_WAIT|M_MYTURN|M_HISTURN|M_PAUSED, "Retour au menu principal" (sans sauvegarder)
                msg.type=MSG_ENDGAME;
                msg_send(&msg, 0);

                retour_menu(&game);
                MenuState=M_MAIN;
            }
            break;
        case '/':   // M_MYTURN, "Jouer un coup"
            jouer_coup(&game, choix+1);
            if(game_get_state(&game)==GS_WIN) {
                retour_menu(&game);
                MenuState=M_MAIN;
            }
            else
                MenuState=((game_get_player(&game)==game_get_me(&game))?M_MYTURN:M_HISTURN);
            break;
        }

        // save the environment here, we will come back here if the timer expires or if we receive some messages from the other process (like end_of_game, ...)
        switch(sigsetjmp(jumpenv, 1 /* save signals mask */)) {
        case 0: // ok, environment saved
            break;
        case LJUMP_TIMER:   // timer expired
            // TODO afficher message erreur

            msg.type=MSG_ENDGAME;
            msg_send(&msg, 0 /* no additionnal payload */);   // let's tell the other process we're done

            retour_menu(&game);
            MenuState=M_MAIN;
            break;
        case LJUMP_ISR: // the other process ask me to do something
            switch(last_msg.type) {
            case MSG_JOINGAME:
                timer_stop();   // ok someone is here, we can stop the connection wait timer
                MenuState=((game_get_player(&game)==game_get_me(&game))?M_MYTURN:M_HISTURN);
//                if(MenuState==M_MYTURN)
                    // TODO: timer_start(conf->t_turn...
                break;
            case MSG_GAMETURN:
                game_playturn(&game, (sGameTurn *)last_msg.data);
                if(game_get_state(&game)==GS_WIN) {
                    retour_menu(&game);
                    MenuState=M_MAIN;
                }
                else
                    MenuState=((game_get_player(&game)==game_get_me(&game))?M_MYTURN:M_HISTURN);
//                    if(MenuState==M_MYTURN)
                        // TODO: timer_start(conf->t_turn...
                break;
            case MSG_ENDGAME:   // the other process quit
                retour_menu(&game); // we quit aswell
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

