#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

static char** wordlist = NULL;
static int wordlist_counter = 0;
static int wordlist_max = 0;

static void initializeWordlist(){
    // Initialize the wordlist to hold pointers to chars
    wordlist = (char**) malloc(1 * sizeof (char*));
    if(wordlist == NULL){
        // Malloc did an oopsie woopsie fucky wucky
        perror("malloc");
        exit(EXIT_FAILURE); // YEEET
    }
    wordlist_max = 1;
}

static void extendWordlist(){
    wordlist_max = wordlist_max * 2;
    wordlist = (char**) realloc(wordlist, wordlist_max * sizeof (char*));
    if(wordlist == NULL){
        // Realloc did a big fucking oopsie woopsie fucky wucky
        perror("realloc");
        exit(EXIT_FAILURE); // YEEET
    }
}

static void fillWithZero(char* c, int n){
    for(int i=0;i<n;i++){
        c[i] = '\0';
    }
}

// Handles reading from stdin and storing all the words
static void readInput(){

    // Temporary string to store the current word
    char tmpWord[102]; // Size [0] to [101]
    // Fill the string with newline characters
    fillWithZero(tmpWord, 102);

    // Read from stdin using fgets
    /*
     * fgets()  reads in at most one less than size characters from stream and
     * stores them into the buffer pointed to by s.  Reading  stops  after  an
     * EOF  or a newline.  If a newline is read, it is stored into the buffer.
     * A terminating null byte ('\0') is stored after the  last  character  in
     * the buffer.
     *
     * Ein Wort umfasst alle Zeichen einer Zeile. Zeilen sind durch ein Zeilenumbruch-
     * Zeichen (\n) voneinander getrennt, das selbst nicht Teil des Wortes ist. Jede
     * Zeile endet mit einem Zeilenumbruch – lediglich die letzte Zeile muss nicht
     * zwangsläufig ein \n-Zeichen enthalten.
     */
    // Read Word [0] to [99], [100] has to be newline as the string is filled by
    // Newline chars. If [100] is anything else than newline it has been overwritten
    // and the word is too long. [101] has to be Nullbyte.
    // given 102 fgets will read 101 chars ([0] to [100]) and [101] will be Nullbyte
    while(fgets(tmpWord, 102, stdin) != NULL){

        // Check if the line is empty by checking if the first char is still newline
        if(tmpWord[0] == '\n'){
            // Skip word silently, no need to fill the tmp string again as it is untouched
            continue;
        }

        // Check if the word is too long using \n as kind of string canary
        // Last line will also have a \n as the String is filled with it beforehand
        if(tmpWord[100] != '\n' && tmpWord[100] != '\0'){
            // Clear the String again with newline
            fillWithZero(tmpWord, 102);
            // Log the error and continue
            fprintf(stderr, "Error: Word too long, word ignored");
            // Try to find the end of the word
            char c;
            while((c = (char)getchar()) != '\n' && c != EOF){}
            // If this crashes so will fgets, and we do error handling there
            continue;
        }

        // Malloc for the word to store it
        char* newWord = (char*) malloc(102 * sizeof(char));
        if(newWord == NULL){
            // Malloc did an oopsie woopsie fucky wucky
            perror("malloc");
            exit(EXIT_FAILURE); // YEEET
        }

        // Copy the tmp word over to the allocated memory
        // If this fails we are fucked :) (but we cant know)
        strncpy(newWord, tmpWord, 102);

        // Extend wordlist if needed
        if(wordlist_counter == wordlist_max){
            extendWordlist();
        }

        // Add the pointer to the allocated memory to the wordlist
        wordlist[wordlist_counter] = newWord;
        wordlist_counter++;

        // Clear the tmp String again with newline
        fillWithZero(tmpWord, 102);
    }
    // Error handling for fgets
    if (!feof(stdin)){
        // fegts throws NULL without the input being EOF -> we fucked up somehow
        perror("fgets");
        exit(EXIT_FAILURE); // YEEET
    }
}

static int compareStrings(const void* a, const void* b){
    const char* ia = *(const char **) a;
    const char* ib = *(const char **) b;
    return (strcmp(ia, ib));
}

static void sortList(){
    qsort(wordlist, wordlist_counter, sizeof(char*), compareStrings);
}

static void printWordlist(){
    for(int i=0;i<wordlist_counter;i++){
/*I----> +--------------------------------------------------------------------+
         | Hier sollte eigentlich fputs verwendet werden, aber da printf auch |
         | auf den stdin schreibt, ist das okay                               |
         +-------------------------------------------------------------------*/
        int a = printf("%.101s", wordlist[i]);
        if(a < 0){
            perror("printf");
            exit(EXIT_FAILURE);
        }
        free(wordlist[i]);
    }
    free(wordlist);
}

int main (int argc, char* argv[]) {

    // Prepare list to store the words
    initializeWordlist();

    // Fetch all the words
    readInput();

    // Sort all the words
    sortList();

    // Print all the Words
    printWordlist();

    if(fflush(NULL) != 0){
        perror("fflush");
        exit(EXIT_FAILURE); // YEEET
    }
    exit(EXIT_SUCCESS); // YEEET
}

/*I----> +--------------------------------------------------------------------+
         | Gut gemacht                                                        |
         +-------------------------------------------------------------------*/


/*P----> +--------------------------------------------------------------------+
         | Punktabzug in dieser Datei: 0.0 Punkte                             |
         +-------------------------------------------------------------------*/
