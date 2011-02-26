/* Marverti Federico (yfede@tiscali.it) */
/* La parte in C accetta un numero variabile di parametri che rappresentano nomi
 * relativi semplici di file f0, f1, f2 ... fN-1 . Il processo padre deve
 * generare un numero di processi figli pari al numero di file passati come
 * parametri: ogni processo figlio  associato ad uno dei file fi (con i che
 * varia da 0 a N-1). Ognuno di tali processi figli esegue concorrentemente
 * leggendo dal file associato una selezione dei caratteri riportandoli sullo
 * standard output.  I processi figli devono coordinarsi vicendevolmente in modo
 * che le scritture su standard output avvengano in uno specifico ordine.
 * In dettaglio, ogni processo figlio, ciclicamente partendo dal primo processo
 * fino all'ultimo processo, deve selezionare i caratteri di indice i + k * N
 * (con i che varia da 0 a N-1 e con k che varia da 0, 1, 2, ...) e deve
 * stampare il carattere selezionato solo se il processo associato al file di
 * indice precedente ha gia` effettuato la sua stampa. Ad esempio supponendo che
 * i parametri di invocazione siano 3 con nomi file0, file1 e file2, il processo
 * di indice 0 deve leggere da file0 il carattere di indice 0 (=0+0*3) e
 * riportarlo su standard output, quindi il processo di indice 1 deve leggere da
 * file1 il carattere di indice 1 (=1+0*3) e riportarlo su standard output (dopo
 * essersi coordinato con il processo precedente), infine il processo di indice 2
 * deve leggere da file2 il carattere di indice 2 (=2+0*3) riportarlo su standard
 * output (dopo essersi coordinato con il processo precedente); a questo punto
 * si deve ricominciare nel senso che il processo di indice 0 deve leggere da
 * file0 il carattere di indice 3 (=0+1*3) e riportarlo su standard output (dopo
 * essersi coordinato con il processo precedente), quindi il processo di indice 1
 * deve leggere da file1 il carattere di indice 4 (=1+1*3) e riportarlo su
 * standard output (dopo essersi coordinato con il processo precedente), infine
 * il processo di indice 2 deve leggere da file2 il carattere di indice 5
 * (=2+1*3) riportarlo su standard output (dopo essersi coordinato con il
 * processo precedente); etc..
 * Si consideri comunque che le lunghezze dei file siano tali che i processi
 * terminano in cascata, dal primo all'ultimo.
 * yfede: io questa ultima assunzione non l'ho presa per buona, mi piace
 * complicarmi la vita. Pero` funziona ed e` piu` robusto.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Defines utilizzate per identificare i lati lettura e scrittura delle pipe */
#define R 0 /* Lato lettura */
#define W 1 /* Lato scrittura */

typedef int pipe_t[2];	/* Definisce un tipo per le pipe */

int main(int argc, char **argv) {
    int n;	/* Numero di processi figli da creare */
    int i;	/* Contatore per la generazione delle pipe e dei figli */
    int j;	/* Contatore per la chiusura dei lati non usati delle pipe */
    pipe_t *p;	/* Puntatore ad un array di pipe */
    char sync;  /* Carattere usato per il sincronismo con le pipe */

    /* Variabili per il funzionamento del processo padre */
    int pidfiglio;

    /* Controllo parametri dati a riga di comando */
    if( argc < 2) {
        printf("Uso: %s nomefile ...\n", argv[0]);
        exit(1);
    }

    /* Calcolo del numero di figli da generare */
    n = argc-1;

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
        int inputpipe;  /* Pipe di ingresso per la sincronizzazione */
        int fd;         /* Descrittore del file */
        int count = 0;  /* Conteggio dei caratteri letti */
        char c;         /* Carattere letto dal file */
        /* --- fine variabili usate solo dai processi figli --- */

        switch(fork()) {    /* Genera un figlio */
            case -1:	/* Errore nella fork */
                printf("Impossibile creare un nuovo processo\n");
                exit(1);
            break;		/* Non necessario, ma per chiarezza... */

            case 0:		/* Processo figlio */
                /* Lasciamo aperte le pipe in input per poterci sincronizzare
                 * con tutti i processi
                 */

                /* Aperta solo la pipe per innescare il processo successivo */
                for(j=0;j<n;j++) if(j!=i) close(p[j][W]);

                /* Valore iniziale della pipe con cui si sincronizza */
                if(i==0) inputpipe = n-1;
                else inputpipe = i-1;

                /* --- inizio codice del  comportamento del figlio --- */
                fd = open(argv[i+1], O_RDONLY);   /* Apre il file */
                if(fd<0) {
                    printf("Errore nell'apertura del file %s\n", argv[i+1]);
                    exit(1);
                }

                for(;;) {   /* Termina all'interno con break */
                    /* Sincronizzazione con il processo precedente */
                    j = read(p[inputpipe][R],&sync,sizeof(char));
                    while(j==0) {       /* se quella pipe e` chiusa */
                        inputpipe--;    /* ne cerca una precedente */
                        if(inputpipe<0) inputpipe = n-1;
                        j = read(p[inputpipe][R],&sync,sizeof(char));
                    }

                    while(count%n != i) {   /* fino al primo carattere valido */
                        read(fd,&c,sizeof(char));
                        count++;    /* Incrementa i caratteri letti */
                    }
                    if(read(fd,&c,sizeof(char)) <= 0)   /* Read fallita? */
                        break;  /* Se si, e` finito il file, quindi esce */
                    count++;    /* Incrementa i caratteri letti */
                    printf("Processo %d, count=%d, c=%c\n",i,count,c);

                    write(p[i][W],&sync,sizeof(char)); /* Via al successivo */
                }
                close(fd);  /* Chiude il file */
                write(p[i][W],&sync,sizeof(char)); /* Via al successivo */
                close(p[i][W]);	/* Chiude il lato di scrittura usato */
                exit(0);   /* Ricordarsi di uscire per evitare i nipoti! */
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
        if(j!=n-1) close(p[j][W]);  /* Aperto l'innesco per il primo processo */
        close(p[j][R]);     /* Chiude tutte le letture */
    }

    write(p[n-1][W],&sync,sizeof(char));    /* Innesca il primo figlio */
    close(p[n-1][W]);                       /* Chiude la pipe di innesco */

    /* --- inizio codice del comportamento del padre --- */
    /* --- fine codice del comportamento del padre --- */

    /* Attesa della terminazione dei figli */
    for(i=0;i<n;i++) {
        pidfiglio = wait(&j);
        if(WIFEXITED(j))    /* Se il figlio e` uscito normalmente */
            printf("Figlio %d terminato con %d\n", pidfiglio, WEXITSTATUS(j));
        else
            printf("Figlio %d terminato in modo anomalo\n", pidfiglio);
    }

    exit(0);    /* The End */
}
