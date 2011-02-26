#!/bin/bash
# Marverti Federico (yfede@tiscali.it)
# Script ricorsivo invocato dallo script di inizializzazione dopo aver
# controllato la validita` dei parametri passati a riga di comando, oppure da
# altre istanze di questo stesso script ricorsivo.
# In ogni caso diamo per scontato che gli argomenti della riga di comando siano
# validi come in effetti dovrebbero.
# si assume che $1 sia il percorso assoluto di una directory accessibile.
# si assume che $2 sia una qualsiasi stringa
# si assume che $3 sia un numero decimale (in realta` non e` strettamente
# necessario ai fini del funzionamento del programma).

# L'unico controllo che facciamo prima di proseguire e` che la directory, oltre
# che accessibile, sia anche leggibile. Se non lo e` non possiamo ottenere la
# lista dei file quindi possiamo proseguire oltre terminando questa istanza
# del programma ricorsivo. In ogni caso stampiamo un messaggio di warning
# su quanto accaduto..

if test ! -r "$1"
then
  echo "warning: $1 non e\` leggibile!" >&2
  echo "         non e\` stato possibile leggerne il contenuto!" >&2
  exit 0
fi

# Se tutto va bene andiamo nella directory specificata e procediamo
cd "$1"
conta=0

# Invocazione ricorsiva su tutti i sottodirettori accessibili.
for i in *
do
  if test -d $i -a -x $i
  then
    echo "invocazione: recursive `pwd`/$i $2 $3"
    recursive `pwd`/$i $2 $3
    # Incrementa una variabile di conteggio in base ai valori di ritorno
    conta=`expr "$conta" + $?`
  fi
done

# Bene, ora veniamo al compito dello script ricorsivo vero e proprio:
# intanto cerchiamo tutti i file che nel nome contengono la stringa $2
for i in *$2*
do
  if test -f $i -a -r $i # se $i e` un file leggibile
  then
    echo "trovato `pwd`/$i che corrisponde al pattern *$2*" # messaggino
    if grep "$3" <"`pwd`/$i" >/dev/null 2>/dev/null  # se c'e il numero cercato
    then
      echo "`pwd`/$i contiene $3"  # messaggino
      conta=`expr "$conta" + 1`    # .. e incrementa il conteggio
    fi
  fi
done

exit $conta  # ritorna il valore contato da questa istanza e da tutte le figlie

