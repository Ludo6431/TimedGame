/* 12 - Gestion des signaux - temporisation */
/* Mise en place d'une temporisation - Utilisation d'un saut
non local
Le programme principal attend 5 seconde la saisie d'une valeur
Quand le delai est depasse, il affiche un message et se
termine
- Envoi du signal SIGALRM au bout de 5 secondes
- La fonction de gestion du signal renvoie au programme
principal*/

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <setjmp.h>


int a=0;
void gestionAlarme(int numSig, int a) {

printf("passage dans gestionalarme\n");
a=1;

}

int main(void) {
    int i;

    /* Installation du handler gestionAlarme pour SIGALRM */
    signal(SIGALRM,gestionAlarme);

    /* Traitement */
    printf("Vous avez 5 secondes pour entrer un nombre entier\n");
        alarm(5); /* Mise en place du timer */
    /* Memorisation du contexte d'execution */
    if (a >= 1) { /* 1er passage */

       // alarm(0); /* saisie effectuee, on annule le timer */
        printf("OK: passage dans if\n",i);
    }

    else {
        printf("pas de passage dans if!\n");
    }
sleep(6);
return(0);
}


