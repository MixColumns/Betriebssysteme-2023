#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

const int MAX = 100;

int main(int argc, char *argv[]) {
	
	// Array für die Zahlen
    bool prime[MAX];

    // Initialisieren
    for(int i=0; i<=MAX; i++){
        prime[i] = true;
    }
    prime[0] = false;
    prime[1] = false;

    // Können bei sqrt(MAX) cappen
    for(int i=2; i*i<=MAX; i++){
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
