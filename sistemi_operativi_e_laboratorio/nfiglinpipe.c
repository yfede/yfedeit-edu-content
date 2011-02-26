/* Marverti Federico (yfede@tiscali.it) */
/* La parte in C accetta un parametro che rappresenta il nome di un file F.
 * Il processo padre deve generare 2 processi figli, uno associato alle linee
 * pari di F e l'altro associato alle linee dispari. I processi figli eseguono
 * concorrentemente, calcolano la lunghezza della linee associate e comunicano
 * al padre tali valori. Il processo padre riceve dai figli le lunghezze delle
 * linee, e le stampa sullo standard output, rispettando l.ordine originale
 * delle linee del file F. Al termine, ogni figlio ritorna al padre la lunghezza
 * della linea pi lunga tra quelle che ha elaborato e il padre, per ogni figlio,
 * deve stampare su standard output il pid del figlio e il valore ricevuto
 */

/* Esercizio risolto in modo generico considerando la possibilita` di creare
 * un numero arbitrario n di figli, ciascuno comunicante col padre attraverso
 * una pipe distinta. */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Defines utilizzate per identificare i lati lettura e scrittura delle pipe */
#define R 0
#define W 1

typedef int pipe_t[2];	/* Definisce un tipo per le pipe */

int main(int argc, char **argv) {
    int n;	/* Numero di processi figli da creare */
    int i;	/* Contatore per la generazione delle pipe e dei figli */
    int j;	/* Contatore per la chiusura dei lati non usati delle pipe */
    pipe_t *p;	/* Puntatore ad un array di pipe */

    /* Variabili per il funzionamento del processo padre */
    int lungh;
    int letti;
    pid_t pidfiglio;

    /* Controllo parametri dati a riga di comando */
    if( argc != 2) {
        printf("Uso: %s nomefile\n", argv[0]);
        exit(1);
    }

    /* Calcolo del numero di figli da generare */
    n = 2;

    /* Allocazione del vettore di pipe */
    p = malloc(sizeof(pipe_t)*n);
    if( p == NULL ) {
        printf("Impossibile allocare il vettore di pipe\n");
        exit(1);
    }

    /* Generazione di tutte le pipe */
    for(i=0; i<n; i++) {
        if(pipe(p[i]) != 0) {
            printf("Errore nella creazione della pipe %d\n", i);
            exit(1);
        }
    }

    /* Generazione dei figli e definizione del loro comportamento */
    for(i=0; i<n; i++) {
        /* --- inizio variabili usate solo dai processi figli --- */
        int nlinea = 0;	/* Numero della linea corrente nel file */
        int maxlun = 0;	/* Lunghezza massima trovata */
        int lun = 0;	/* Lunghezza della linea corrente */
        int fd;         /* Descrittore del file */
        char c;         /* Ultimo carattere letto dal file */
        /* --- fine variabili usate solo dai processi figli --- */

        switch(fork()) {    /* Genera un figlio */
            case -1:	/* Errore nella fork */
                printf("Impossibile creare un nuovo processo\n");
                exit(1);
            break;		/* Non necessario, ma per chiarezza... */

            case 0:		/* Processo figlio */
                for(j=0; j<n; j++) {	/* Chiudiamo i fd non usati */
                    if(j!=i)
                        close(p[j][W]); /* Lascia aperto il lato scrittura */
                    close(p[j][R]);	    /* Chiude tutti i lati lettura */
                }

                /* --- inizio codice del  comportamento del figlio --- */
                fd = open(argv[1], O_RDONLY);   /* Apre il file */
                if(fd<0) {
                    printf("Errore nell'apertura del file %s\n", argv[1]);
                    exit(1);
                }

                while(read(fd,&c,sizeof(char))>0) { /* Legge un carattere */
                    if(nlinea%n==i) {   /* Se la linea e` del giusto ordine */
                        if(c=='\n') {   /* Quando trova la fine della linea...*/
                            if(lun>maxlun)  /* Aggiorna la lunghezza massima */
                                maxlun=lun;
                            /* Comunica al padre la lunghezza della linea */
                            write(p[i][W],&lun,sizeof(int));
                            lun = 0;    /* Azzera per la linea successiva */
                            nlinea++;   /* Incrementa il numero di linea */
                        } else lun++;   /* Altrimenti incrementa la lunghezza */
                    } else {    /* Se la linea non e` del giusto ordine */
                        if(c=='\n') {   /* Quando trova un ritorno a capo */
                            nlinea++;   /* Incrementa solo il numero di linea */
                        }
                    }
                }
                /* Manda i dati dell'ultima linea se e` del giusto ordine */
                if(nlinea%n==i) write(p[i][W],&lun,sizeof(int));

                close(fd);  /* Chiude il file */
                /* --- fine codice del  comportamento del figlio --- */

                close(p[i][W]);	/* Chiude il lato di scrittura usato */
                exit(maxlun);   /* Ricordarsi di uscire per evitare i nipoti! */
            break;

            default:	/* Processo padre */
            /* Qui va inserito l'eventuale codice del processo padre che
             * necessiti di essere iterato per ciascun processo generato.
             * Il codice del padre da eseguire una sola volta al termine della
             * creazione dei figli va inserito dopo la chiusura del ciclo for.
             */
            break;
        }
    }

    /* Processo padre */

    for(j=0; j<n; j++) {	/* Chiusura lati non utilizzati delle pipe */
        close(p[j][W]);
    }

    /* --- inizio codice del comportamento del padre --- */
    do {
        letti = 0;
        for(i=0;i<n;i++) {
            j=read(p[i][R],&lungh,sizeof(int));
            if(j>0) printf("%d\n", lungh);
            letti += j;
        }
    } while(letti>0);
    /* --- fine codice del comportamento del padre --- */

    /* Attesa della terminazione dei figli */
    for(i=0;i<n;i++) {
        pidfiglio = wait(&j);
        if(WIFEXITED(j))    /* Se il figlio e` uscito normalmente */
            printf("Figlio %d terminato con %d\n", pidfiglio, WEXITSTATUS(j));
        else
            printf("Figlio %d terminato in modo anomalo\n", pidfiglio);
    }

    for(j=0; j<n; j++) {    /* Definitiva chiusura delle pipe */
        close(p[j][R]);
    }

    exit(0);    /* The End */
}
