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
