#!/bin/bash
# Marverti Federico (yfede@tiscali.it)
# Script di avvio operazioni ricorsive su una gerarchia
# come esempio implementiamo un programma che stampa tutti i direttori
# contenenti un file che ha nel nome la stringa specificata come secondo
# parametro da riga di comando e che contenga al suo interno il numero
# specificato dal terzo.

# I parametri devono essere 3, giusto?
if test "$#" -ne 3
then
  echo "specificare il corretto numero di parametri"
  echo "Uso: $0 gerarchia stringa1 stringa2"
  exit 1
fi
# giusto!

# il primo parametro deve essere un percorso assoluto di directory accessibile
case $1 in
  /*)
    if test -d $1 -a -x $1
    then
      echo "ok, $1 e\` il percorso assoluto di una directory accessibile"
    else
      echo "$1 deve essere il percorso assoluto di una directory accessibile"
      exit 3
    fi
  ;;
  *)
    echo "$1 deve essere il percorso assoluto di una directory accessibile"
    exit 2
  ;;
esac
# ... lo e`!

# il secondo parametro e` una stringa quindi non ci sono da fare particolari
# controlli su di essa.

# il terzo argomento e` un numero?
expr "$3" + 0 >/dev/null 2>/dev/null
# test ritorna il valore 2 se l'espressione non ha significato.
# Non e` sufficiente controllare che il valore di ritorno sia diverso da 0
# perche` se il risultato dell'operazione e` 0 o null, test ritorna 1
if test $? = 2
then
  echo "$3 non e\` un numero decimale valido"
  exit 4
fi
# si, allora comunichiamolo
echo "ok, $3 e\` un numero decimale valido"

# aggiungere a PATH la directory corrente per consentire l'esecuzione dello
# script ricorsivo puo` non essere sufficiente: se infatti lo script di
# inizializzazione viene richiamato da un'altra directory specificandone il
# percorso relativo, viene aggiunta al PATH una directory che probabilmente
# non contiene lo script ricorsivo che si vorrebbe chiamare.
# Con il codice seguente salviamo nella variabile scriptdir la directory
# assoluta in cui si trova questo file comandi e la aggiungiamo al path,
# assumendo che vi si trovi anche lo script da chiamare ricorsivamente.
scriptdir=`dirname $0`
# dirname ritorna il percorso che precede il file specificato come argomento
if test "$scriptdir" != ""
then
  cd "$scriptdir"
fi
# salta alla directory in cui si trova lo script.
export PATH=`pwd`:$PATH
echo "\$PATH = \"$PATH\""
# essa viene aggiunta in ogni caso al path con il suo percorso assoluto, nella
# eventualita` che PATH contenga anche la directory "." come percorso di ricerca
# in cui trovare i comandi. Sotto tali condizioni la prima invocazione dello
# script ricorsivo andrebbe a buon fine essendo effettuata dalla stessa
# directory in cui esso si trova, ma se essa viene cambiata prima di lanciare
# la seconda invocazione dello script ricorsivo, esso potrebbe risultare non
# piu` raggiungibile perche` non incluso nella nuova directory di lavoro.

# Bene, a questo punto possiamo chiamare la prima istanza dello script
# ricorsivo.
echo "Invocazione: recursive $1 $2 $3"
recursive $1 $2 $3
echo "Valore di ritorno: $?"

