#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*I----> +--------------------------------------------------------------------+
         | Korrektur: xxxxxxxxxxxxxxxxxx                                      |
         +-------------------------------------------------------------------*/

const int MAX = 100;

int main(int argc, char *argv[]) {
    
    // Array für die Zahlen
    bool prime[MAX];
/*I----> +--------------------------------------------------------------------+
         | Dein Array ist um einen Eintrag zu klein: In C beginnt die         |
         | Indizierung von Arrays bei 0; daher muss dein Array MAX + 1        |
         | Einträge groß sein, wenn du Einträge bis einschließlich MAX    |
         | indizieren möchtest. So ein Fehler gibt in späteren Abgaben      |
         | einen Punkt Abzug. (-1)                                            |
         +-------------------------------------------------------------------*/


    // Initialisieren
    for(int i=0; i<=MAX; i++){
/*I----> +--------------------------------------------------------------------+
         | Wenn dein Array MAX Element hat, ist der letzte gültige Index     |
         | (MAX - 1). Du betrachtest aber Indizes i <= MAX -> damit ist der   |
         | letzte Zugriff auf prime[MAX] außerhalb der Arraygrenzen und      |
         | ungültig! So ein Fehler gibt in späteren Abgaben einen Punkt     |
         | Abzug. (-1)                                                        |
         +-------------------------------------------------------------------*/
        prime[i] = true;
    }
    prime[0] = false;
    prime[1] = false;

    // Können bei sqrt(MAX) cappen
    for(int i=2; i*i<=MAX; i++){
/*I----> +--------------------------------------------------------------------+
         | hier wird, wie oben, ein Element zu viel betrachtet (prime[MAX]).  |
         +-------------------------------------------------------------------*/
        // Wenn Prim dann alle vielfachen aussieben
        if(prime[i]){
            for(int j=i*2; j<=MAX; j+=i){
                prime[j] = false;
            }
        }else{

            // Wenn nicht Prim nächste Zahl
            continue;
        }
    }

    // Primzahlen ausgeben, die anderen ignorieren
    for(int i=0; i<=MAX; i++){
        if(prime[i]){
            printf("%i\n", i);
        }
    }
    
    return EXIT_SUCCESS;
}
/*P----> +--------------------------------------------------------------------+
         | Punktabzug in dieser Datei: 2 Punkte                               |
         +-------------------------------------------------------------------*/
