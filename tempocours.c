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

sigjmp_buf contexteAlarme;

void gestionAlarme(int numSig) {
siglongjmp( contexteAlarme, 1); /* branchement a sigsetjmp() en
retournant 1 */
}

int main(void) {
    int i;

    /* Installation du handler gestionAlarme pour SIGALRM */
    signal(SIGALRM,gestionAlarme);

    /* Traitement */
    printf("Vous avez 5 secondes pour entrer un nombre entier\n");

    /* Memorisation du contexte d'execution */
    if (sigsetjmp(contexteAlarme, 1) == 0) { /* 1er passage */
        alarm(5); /* Mise en place du timer */
        scanf("%d", &i);

        alarm(0); /* saisie effectuee, on annule le timer */
        printf("OK: %d saisi\n",i);
    }

    else {
        printf("Trop tard!!!\n");
    }

return(0);
}
