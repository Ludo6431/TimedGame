/************************** memoirepartagee.c ************************/
/* Ce fichier comporte :
 - creation_memoire_partage
 - attachement_memoire_partage

*/



/*creer une structure qui contient :
    - mutex
    - l'etat de la partie
    */

typedef struct memoirepartagee memoirepartagee;
struct memoirepartagee
{
    int dureepartie:
    int dureecoup;
    int connexion;
};



/*fonction appelé par nouvelle_partie(), qui crée la clé à partir du nom du joueur et de la partie,
et qui crée l'espace de mémoire partagée*/
int ouverture_memoire_partage(int cle){

    /* Creation d'un segment de memoire partagee de 80 octets */
    if ((id = shmget(cle, 80, 0600|IPC_CREAT))==-1) {
        perror("Creation memoire partagee");
        exit(1);
    }

    return id;
}


/* Attachement de la zone de memoire a l'espace memoire du processus, connaissant l'id */
void attachement_memoire_partage(int id){
    char *adresse; /* adresse du segment de memoire partagee */

    if ((int)(adresse = shmat(id, NULL, 0)) == -1) {
        perror("Attachement memoire partagee");
        shmctl(id, IPC_RMID, NULL); /* Suppression du segment */
        exit(1);
    }
}




void mise_a_jour_partie(void){

}


