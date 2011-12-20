#include <signal.h> // signal, kill
#include "shm.h"
#include "game.h"

#include "synchro.h"

//TODO rajouter "eShmsynchro ask" , "t_time time" et "char pid[2]" dans sShm
//TODO remplacer dans MAIN.C     sGame game 	par	sGame game	pour pouvoir effectuer les actions de syncho_handler.. ?


//TODO changer les valeurs de MenuState dans synchro_handler
//TODO vérifier que les signaux envoyés par kill sont bien réçus par l'autre joueur ... 

void synchro_handler(int numSig){

	if(numSig==SIGUSR1){ //le 2eme joueur a modifier shm->ask, et le joueur1 traite le signal
	    switch(shm->ask) {
		case 'SHM_ASKNAME': 
			shm_lock(shm);
			shm->nom=game.playername[1][];
			shm->ask=SHM_GAVENAME;
			kill(shm->pid[1],SIGUSR2); 
			shm_unlock(shm);
		case 'SHM_ASKTTURN': 
			shm_lock(shm);
			shm->time=game->t_turn;
			shm->ask=SHM_GAVETTURN;
			kill(shm->pid[1],SIGUSR2); 
			shm_unlock(shm);
		break;
		case 'SHM_ASKTTOTAL': 
			shm_lock(shm);
			shm->time=game->t_total;
			shm->ask=SHM_GAVETTOTAL;
			kill(shm->pid[1],SIGUSR2); 
			shm_unlock(shm);
		break;
		case 'SHM_ASKTREMAINING': 
			shm_lock(shm);
			shm->time=game->t_remainning;
			shm->ask=SHM_GAVETREMAINING;
			kill(shm->pid[1],SIGUSR2); 
			shm_unlock(shm);
		break;
		/*case 'SHM_ASKREADY': 

		break;*/
		case 'SHM_ASKQUIT': 
			shm_lock(shm);	
			shm->ask=SHM_QUIT;
			kill(shm->pid[1],SIGUSR2); 
			//faire ce qe fait retour_menu()
			shm_unlock(shm);
		break;


		case 'SHM_GAVENAME': 
			shm_lock(shm);
			game.playername[2][]=shm->nom;
			shm->ask=SHM_SYNCHRO_OK;
			kill(shm->pid[1],SIGUSR2); 
			shm_unlock(shm);
		break;
		case 'SHM_GAVETTURN': 
			shm_lock(shm);
			game->t_turn=shm->time;
			shm->ask=SHM_SYNCHRO_OK;
			kill(shm->pid[1],SIGUSR2); 
			shm_unlock(shm);
		break;
		case 'SHM_GAVETTOTAL': 
			shm_lock(shm);
			game->t_total=shm->time;
			shm->ask=SHM_SYNCHRO_OK;
			kill(shm->pid[1],SIGUSR2); 
			shm_unlock(shm);
		break;
		case 'SHM_GAVETREMAINING': 
			shm_lock(shm);
			game->t_remaining=shm->time;
			shm->ask=SHM_SYNCHRO_OK;
			kill(shm->pid[1],SIGUSR2); 
			shm_unlock(shm);
		break;
		case 'SHM_READY': 
			shm_lock(shm);
			//mettre le MenuState a M_INGAME;
			shm->ask=SHM_SYNCHRO_OK;
			kill(shm->pid[1],SIGUSR2); 
			shm_unlock(shm);
		break;
		case 'SHM_QUIT': 
			shm_lock(shm);
			//faire ce qe fait retour_menu()	
			shm->ask=SHM_SYNCHRO_OK;// ou pas, puisque l'autre se sera deja détaché de la memoire partagée..
			kill(shm->pid[1],SIGUSR2); //ide
			shm_unlock(shm);
		break;

	}


	else if(numSig==SIGUSR2){
	  switch(shm->ask) {
		case 'SHM_ASKNAME': 
			shm_lock(shm);
			shm->nom=game.playername[2][];
			shm->ask=SHM_GAVENAME;
			kill(shm->pid[2],SIGUSR1); 
			shm_unlock(shm);
		case 'SHM_ASKTTURN': 
			shm_lock(shm);
			shm->time=game->t_turn;
			shm->ask=SHM_GAVETTURN;
			kill(shm->pid[2],SIGUSR1); 
			shm_unlock(shm);
		break;
		case 'SHM_ASKTTOTAL': 
			shm_lock(shm);
			shm->time=game->t_total;
			shm->ask=SHM_GAVETTOTAL;
			kill(shm->pid[2],SIGUSR1); 
			shm_unlock(shm);
		break;
		case 'SHM_ASKTREMAINING': 
			shm_lock(shm);
			shm->time=game->t_remainning;
			shm->ask=SHM_GAVETREMAINING;
			kill(shm->pid[2],SIGUSR1); 
			shm_unlock(shm);
		break;
		/*case 'SHM_ASKREADY': 

		break;*/
		case 'SHM_ASKQUIT': 
			shm_lock(shm);	
			shm->ask=SHM_QUIT;
			kill(shm->pid[0],SIGUSR1); 
			//faire ce qe fait retour_menu()
			shm_unlock(shm);
		break;

		case 'SHM_GAVENAME': 
			shm_lock(shm);
			game.playername[1][]=shm->nom;
			shm->ask=SHM_SYNCHRO_OK;
			kill(shm->pid[0],SIGUSR1); 
			shm_unlock(shm);
		break;
		case 'SHM_GAVETTURN': 
			shm_lock(shm);
			game->t_turn=shm->time;
			shm->ask=SHM_SYNCHRO_OK;
			kill(shm->pid[0],SIGUSR1); 
			shm_unlock(shm);
		break;
		case 'SHM_GAVETTOTAL': 
			shm_lock(shm);
			game->t_total=shm->time;
			shm->ask=SHM_SYNCHRO_OK;
			kill(shm->pid[0],SIGUSR1); 
			shm_unlock(shm);
		break;
		case 'SHM_GAVETREMAINING': 
			shm_lock(shm);
			game->t_remaining=shm->time;
			shm->ask=SHM_SYNCHRO_OK;
			kill(shm->pid[0],SIGUSR1); 
			shm_unlock(shm);
		break;
		case 'SHM_READY': 
			shm_lock(shm);
			//mettre le MenuState a M_INGAME;
			shm->ask=SHM_SYNCHRO_OK;
			kill(shm->pid[0],SIGUSR1); 
			shm_unlock(shm);
		break;
		case 'SHM_QUIT': 
			shm_lock(shm);
			//faire ce qe fait retour_menu()	
			shm->ask=SHM_SYNCHRO_OK;// ou pas, puisque l'autre se sera deja détaché de la memoire partagée..
			kill(shm->pid[0],SIGUSR1); //ide
			shm_unlock(shm);
		break;
	}


}



