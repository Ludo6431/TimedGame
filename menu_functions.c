/************************** fonctionsmenu.c ************************/
/* Ce fichier comporte les fonctions appelées par choix_menu
*/
#include <stdio.h>  // printf
#include <pthread.h>    // pthread_*
#include <errno.h>  // errno
#include <sys/types.h>
#include <sys/ipc.h>    // IPC_*
#include <unistd.h>
#include <sys/wait.h>

#include "tools.h"  // readStdin, exitOnErrSyst
#include "game.h"   // game_*
#include "timer.h"  // timer_*
#include "longjump.h"   // long jump stuff
#include "sigmsg.h" // sigmsg*
#include "msgs.h"   // msgs_handler

#include "menu_functions.h"

int nouvelle_partie(sGame *g) {
    sGameConf *conf;
    char tmp[256];
    time_t ttmp;

    /* récupération des informations sur la partie et initialisation de g */
    printf("Nom de la partie       : ");
    readStdin(tmp, sizeof(tmp));
    game_new(g, tmp); // game_new intialise le jeu
    conf=game_get_conf(g, NULL);

    printf("Vous êtes le joueur n°1\n");
    printf("Votre pseudo           : ");
    readStdin(tmp, sizeof(tmp));
    strcpy(conf->playername[P_1], tmp);

    // TODO: demander qui commence la partie et modifier conf->firstplayer

    printf("Durée de la partie (s) : ");
    readStdin(tmp, sizeof(tmp));
    ttmp = (time_t)atoi(tmp);
    conf->t_total=ttmp;
    game_set_remainingtime(g, ttmp);

    printf("Durée par coup (s)     : ");
    readStdin(tmp, sizeof(tmp));
    conf->t_turn=(time_t)atoi(tmp);

    /* création fichier .histo vide pour créer la mémoire partagée */
    game_histo_save(g);

    /* création canal de communication */
    if(msg_init(game_get_filepath(g), 0600|IPC_CREAT|IPC_EXCL, (void *)g)==-1)
        exitOnErrSyst("msg_init", NULL);

    return 0;
}

int connexion(sGame *g) {
    char tmp[256];
    unsigned int utmp;
    LIST *l, *ltmp;
    sMsg msg;

    printf("Parties ouvertes :\n");
    ltmp=l=game_histo_getlist();
    while(ltmp) {
        printf("\t%s\n", (char *)ltmp->data);
        // TODO vérifier que les parties sont en cours

        ltmp=ltmp->next;
    }

    printf("Entrez le nom de la partie que vous voulez rejoindre :\n");
    readStdin(tmp, sizeof(tmp));
    game_new(g, tmp); // game_new intialise le jeu
    // TODO: choisir une partie dans la liste avec son numéro ?

    /* récupération des informations sur la partie et initialisation de g */
    printf("Vous êtes le joueur n°2\n");
    game_set_me(g, P_2);

    printf("Entrez votre nom :\n");
    readStdin(tmp, sizeof(tmp));
    utmp=strlen(tmp)+1;
//    game_set_playername(g, P_2, tmp);

    // ouverture de la mémoire partagée existante
    if(msg_init(game_get_filepath(g), 0600, g)==-1)
        exitOnErrSyst("msg_init", NULL);

    // on se signale à l'autre processus (en indiquant notre nom)
    msg.type=MSG_JOINGAME;
    strcpy(msg.data, tmp);
    if(msg_transfer(&msg, &utmp)==-1)
        exitOnErrSyst("msg_transfer", tmp);

    // l'autre processus devrait nous renvoyer la conf complète du jeu
    if(msg.type==MSG_CONFUPDATE) {
        sGameConf *conf = (sGameConf *)msg.data;

        game_set_conf(g, conf);    // configuration synchronized
        game_set_player(g, conf->firstplayer);

        // FIXME: attention, ceci est recouvert par le menu
        printf("Configuration du jeu :\n");
        printf("Joueur 1 : %s\n", conf->playername[P_1]);
        printf("Joueur 2 : %s\n", conf->playername[P_2]);
        printf("Le joueur %d commence\n", conf->firstplayer+1);
        printf("Durée de la partie : %us\n", (unsigned int)conf->t_total);
        printf("Durée par coup     : %us\n", (unsigned int)conf->t_turn);
    }
    else {
        // TODO: return to menu nicely or ask the config
        fprintf(stderr, "we didn't get the config\n");
    }

    return 0;
}

void reprise_partie_sauvegarde(sGame *g) {
    // TODO

    char tmp[256];
    LIST *l, *ltmp;
    int comp1=1;
    int comp2=1;

    //choix de la partie sauvegardée
    printf("Parties ouvertes :\n");
    ltmp=l=game_histo_getlist();
    while(ltmp) {
        printf("\t%s\n", (char *)ltmp->data);
        // TODO vérifier que les parties sont en cours

        ltmp=ltmp->next;
    }

    printf("Entrez le nom de la partie que vous voulez rejoindre :\n");
    readStdin(tmp, sizeof(tmp));

    if(!game_histo_load(g,tmp)){
	exitOnErrSyst("game_histo_load", NULL);
    }
    //FIXME empecher les autres de pouvoir ouvrir la meme sauvegarde
    //Celui qui charge la partie choisit entre J1 et J2
    while(comp1!=0 || comp2!=0){
        printf("Entrez le nom du joueur que vous étiez : %s (J1) ou %s (J2) \n",g->conf.playername[0],g->conf.playername[1]);
        readStdin(tmp, sizeof(tmp));
        comp1=strncmp(tmp,g->conf.playername[0],8);
        comp2=strncmp(tmp,g->conf.playername[1],8);
    }
    //TODO
    /* ouverture mem partagée et canal de communication */

    
}

void sauvegarder(sGame *g) {

	if(!game_histo_save(g)){
		exitOnErrSyst("game_histo_save", NULL);
	}
}

/*void pause(sGame *g) {
    // TODO
}*/

void reprendre(sGame *g) {
    // TODO
}

void afficher_historique(sGame *g) {

    int pid=0,buflenght=0,tube[2];
    int status; /*pour code retour wait() */
    char buf[150];
    LIST *turns=g->turns;


/* Creation du tube */
    if(pipe(tube)==-1){
        exitOnErrSyst("pipe","creation du pipe impossible");
    }


    
/*ecriture dans le tube des données de la partie*/
    sprintf(buf, "Nom du Joueur 1 : %-8s \nNom du Joueur 2 : %-8s\n Temps total : %04u \n Temps par tour : %04u\r\n", g->conf.playername[0], g->conf.playername[1], (unsigned int)g->conf.t_total, (unsigned int)g->conf.t_turn);
    buflenght=strlen(buf);

	//ecriture des donnees dans le tube
     if((write(tube[1],buf,buflenght))==-1){
                exitOnErrSyst("write","ecriture dans le tube");
     }

    while(turns) {
        sprintf(buf, "%04u%s %u\r\n", (unsigned int)((sGameTurn *)turns->data)->t_remaining, g->conf.playername[((sGameTurn *)turns->data)->player], ((sGameTurn *)turns->data)->type);
	buflenght=strlen(buf);
//ecriture des donnees dans le tube
     	if((write(tube[1],buf,buflenght))==-1){
                exitOnErrSyst("write","ecriture dans le tube");
     	}

        turns=turns->next;
    }


    if((pid=fork())==-1){
        exitOnErrSyst("fork","creation fils");
    }

    if(pid==0){
        /*Code du fils*/

        /*fermeture du descripteur en ecriture sur le tube (on ne l'utilise pas ici)*/
        close(tube[1]);
        close(0);//pour pouvoir recopier tube[0]
        dup(tube[0]);  
        close(tube[0]);
        
        /* lecture donnees du tube, et affichage*/
  

    int nbOctetLus=1; //pour rentrer dans la boucle. Servira a savoir si on a fini de lire le message utilisateur entre au clavier
    char buffer[100]; //pour stocker le message entre par l'utilisateur
    
    /*boucle tant que le message de l'utilisateur n'est pas entirement lu*/
    while(nbOctetLus>0){
        /*lecture de l'entree standard (0).*/
        nbOctetLus=read(0,buffer,100);

        /*affichage sur la sortie standard (l'ecran : 1). On affiche seulement la ou il y a du texte*/
        write(1,buffer,nbOctetLus);
     }

     exit(0);
    }// fin fils 
    
    /*fermeture du descripteur en ecriture sur le tube (on ne l'utilise pas ici)*/
    close(tube[0]);

    wait(&status);
     
}


void jouer_coup(sGame *g, char *s) {
    sGameTurn turn;
    sMsg msg;

    turn.player=game_get_player(g);
    turn.t_remaining=game_get_remainingtime(g);

    if(!strcasecmp(s, "normal") || !strcasecmp(s, ":)"))
        turn.type=T_OK;
    else if(!strcasecmp(s, "gagne") || !strcasecmp(s, ":D"))
        turn.type=T_WIN;
    else
        turn.type=T_INVALID;

    // FIXME: attention, ceci est recouvert par le menu
    printf("%s : ", game_get_conf(g, NULL)->playername[turn.player]);

    switch(turn.type) {
    case T_OK:
        printf("coup normal :)\n");
        break;
    case T_INVALID:
        printf("coup invalide :x\n");
        break;
    case T_WIN:
        printf("coup gagnant :D\n");
        break;
    }

    game_playturn(g, &turn);

    msg.type=MSG_GAMETURN;
    memcpy(msg.data, &turn, sizeof(turn));
    msg_send(&msg, sizeof(turn));
}

void retour_menu(sGame *g) {
//    unlink()

    msg_deinit(!game_get_me(g));  // destroy if we are the host of the game
}

