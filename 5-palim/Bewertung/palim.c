#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include "sem.h"
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

struct statistics {
	int lines;
	int lineHits;
	int files;
	int fileHits;
	int dirs;
	int activeGrepThreads;
	int maxGrepThreads;
	int activeCrawlThreads;
};

// (module-)global variables
static struct statistics stats;
// TODO: add variables if necessary

// function declarations
static void* processTree(void* path);
static void processDir(char* path);
static void processEntry(char* path, struct dirent* entry);
static void* processFile(void* path);
// TODO: add declarations if necessary
// Locking the Stats
static SEM *statLock;
// For the amound of Threads + Waiting
static SEM *grepThreads;
// Search String
static char* needle;
// If stats are new or old
static SEM *newStat;

static void usage(void) {
	fprintf(stderr, "Usage: palim <string> <max-grep-threads> <trees...>\n");
	exit(EXIT_FAILURE);
}

static void die(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

/**
 * \brief Initializes necessary data structures and spawns one crawl-Thread per tree.
 * Subsequently, waits passively on statistic updates and on update prints the new statistics.
 * If all threads terminated, it frees all allocated resources and exits/returns.
 */

int main(int argc, char** argv) {
	if(argc < 4) {
		usage();
	}

	// convert argv[2] (<max-grep-threads>) into long with strtol()
	errno = 0;
	char *endptr;
	stats.maxGrepThreads = strtol(argv[2], &endptr, 10);

	// argv[2] can not be converted into long without error
	if(errno != 0 || endptr == argv[2] || *endptr != '\0') {
		usage();
	}

	if(stats.maxGrepThreads <= 0) {
		fprintf(stderr, "max-grep-threads must not be negative or zero\n");
		usage();
	}


	// TODO: implement me!
	
	// Initialize the stats
	stats.activeCrawlThreads = 0;
	stats.activeGrepThreads = 0;
	stats.dirs = 0;
	stats.fileHits = 0;
	stats.files = 0;
	stats.lineHits = 0;
	stats.lines = 0;
	
	// Set the search string
	needle = argv[1];
	
	// Create Semaphore to lock the Stats
	statLock = semCreate(1);
    if(statLock == NULL){die("semCreate() failed");}
    
    // Create Semaphore that keeps track of the threads plus waiting
    grepThreads = semCreate(stats.maxGrepThreads);
    if(grepThreads == NULL){die("semCreate() failed");}
    
    // Anything new on the stats?
    newStat = semCreate(0);
    if(newStat == NULL){die("semCreate() failed");}
    
	// Create the Crawl Threads and detach them
	pthread_t thraeds[argc-3]; // we have argc-3 many dirs
	for(int i=3; i<argc; i++){
		// create a thread with the given directory as parameter
		errno = pthread_create(&thraeds[i-3], NULL, processTree, argv[i]);
		if(errno != 0){die("pthread_create");}
		// detach thread
		errno = pthread_detach(thraeds[i-3]);
		if(errno != 0){die("pthread_detach");}
		// Update the stats but because we are still in the main thread
		// we dont need to update the statupdatecheker var. The threads
		// will do that until then anyway.
		P(statLock); // Lock stats
		stats.activeCrawlThreads+=1; // Update Threads
		V(statLock); // Unlock stats
	}

	// Wait for threads to finish and print stats until then
	while(1){
		
		// Var to copy the stats
		struct statistics stat = stats;
		
		// Lock Stats
		P(newStat);
		P(statLock);
		stat = stats; // copy stats to local var for long print time
		V(statLock); // unlock the stats
		
		// print the stats in the given format
		printf("\r%i/%i lines, %i/%i files, %i directories, %i active threads",
		stat.lineHits,stat.lines,stat.fileHits,stat.files,stat.dirs, stat.activeGrepThreads);
/*I----> +--------------------------------------------------------------------+
         | printf() ist  eine  gepufferte  Bibliotheksfunktion,  so  dass der |
         | Inhalt ggf. nicht sofort  wirklich an das Betriebssystem übergeben |
         | wird. Üblicherweise wird der Puffer an das Betriebssystem gegeben, |
         | wenn ein Newline geschrieben wird  '\n'. Nachdem in dieser Aufgabe |
         | kein Newline  geschrieben wird,  müsst  ihr manuell  dafür sorgen, |
         | dass der Puffer  zeitnah ans Betriebssystem  zur Ausgabe übergeben |
         | wird, sonst wartet  die libc ggf. mehrere  printf()-Aufrufe ab und |
         | schreibt dann alle gleichzeitig, so dass die Nutzer einige Updates |
         | nie sehen. Mit fflush(stdout) könnt ihr manuell dafür sorgen, dass |
         | die libc den Puffer leert. (-0.5)                                  |
         +-------------------------------------------------------------------*/
		
		// we are done when crawl and grep threads are done. this does not
		// need to be in a semaphore because we dont write to the value and
		// crawl threads can only decrement over time. 
		// just like my will to study.
		if(stat.activeCrawlThreads == 0 && stat.activeGrepThreads == 0){break;}
	
	}
	
	// Clean our mess up qwq
    semDestroy(statLock);
    semDestroy(grepThreads);
/*I----> +--------------------------------------------------------------------+
         | Die Freigabe von `newStat` fehlt (-0.5)                            |
         +-------------------------------------------------------------------*/
	printf("\n");
	fflush(NULL); // awwwww
/*I----> +--------------------------------------------------------------------+
         | `fflush` ist  hier am  Ende nicht  notwendig, es  fehlt allerdings |
         | oben bei der Ausgabe mit '\r'.                                     |
         |                                                                    |
         | Statt NULL  zu übergeben ist  es allgemein sauberer  den konkreten |
         | Stream wie hier `stdout` zu übergeben.                             |
         +-------------------------------------------------------------------*/
	
	return EXIT_SUCCESS;
}

/**
 * \brief Acts as start_routine for crawl-Threads and calls processDir().
 *
 * It updates the stats.activeCrawlThreads field.
 *
 * \param path Path to the directory to process
 *
 * \return Always returns NULL
 */
static void* processTree(void* path) {
	//TODO: implement me!
	// Call Process Dir with the given path
	processDir(path);
	P(statLock); // Lock the stats
	stats.activeCrawlThreads-=1; // we are done so decr. counter before return
	V(newStat); // stats changed too so plz check mommy mainthread
	V(statLock); // unlock stats
	return NULL; // GO BACK!
}

/**
 * \brief Iterates over all directory entries of path and calls processEntry()
 * on each entry (except "." and "..").
 *
 * It updates the stats.dirs field.
 *
 * \param path Path to directory to process
 */

static void processDir(char* path) {
	// TODO: implement me!
	
	// Pointer for da dir
	DIR *dir;
    struct dirent *entry; // even a structure how fancy

	// Open up https://www.youtube.com/watch?v=hZj9bi7YNmI
    dir = opendir(path);
    if (dir == NULL) {
		// Check if it worked. If not we probably dont have perms or smth. RET.
        perror("opendir");
        return;
    }
	
	// To see if we had something going wrong
	errno = 0;
    while ((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, "..\0") == 0){ // check if .. folder optional do something
			// skip
		}else if(strcmp(entry->d_name, ".\0") == 0){ // check if . folder optional do something
			// skip
		}else{
/*I----> +--------------------------------------------------------------------+
         | +----------------------------------------------------------------+ |
         | | if(strcmp(entry->d_name, "..") != 0                            | |
         | |    && strcmp(entry->d_name, ".") != 0)                         | |
         | | {                                                              | |
         | |     processEntry(...)                                          | |
         | | }                                                              | |
         | +----------------------------------------------------------------+ |
         +-------------------------------------------------------------------*/
			processEntry(path, entry); // YAY Finally something to process
		}
/*I----> +--------------------------------------------------------------------+
         | Hier müsste `errno` erneut auf  0 gesetzt werden, da eine Funktion |
         | in `processEntry` eventuell den Wert überschrieben hat. (-0.5)     |
         +-------------------------------------------------------------------*/
    }if(errno != 0){perror("readdir");} // Well that didnt work out

    int err = closedir(dir);
    if(err != 0){perror("closedir");} // neither did this. fuck.
    
    // generally i dont want to commit seppuku at this point because
    // this might be due to some weird file perms or other things going
    // on. Lets just try to keep running like a brave student through
    // the end of the semester barely holding it together but making it somehow.
    
    // Update the stats :) just like before :D
    P(statLock);
	stats.dirs+=1;
	V(newStat);
	V(statLock);

	// R E T U R N
	return;
}

/**
 * \brief Spawns a new grep-Thread if the entry is a regular file and calls processDir() if the entry
 * is a directory.
 *
 * It updates the stats.activeGrepThreads if necessary. If the maximum number of active grep-Threads is
 * reached the functions waits passively until another grep-Threads can be spawned.
 *
 * \param path Path to the directory of the entry
 * \param entry Pointer to struct dirent as returned by readdir() of the entry
 */
static void processEntry(char* path, struct dirent* entry) {
	//TODO: implement me!
	
	// Calculate the Path plus filename to call process file later.
	// Have to join the strings somehow.
	int len = 0;
	len += strlen(path);
	len += strlen(entry->d_name);
	char* fullpath = malloc(sizeof(char) * len + 4);
/*I----> +--------------------------------------------------------------------+
         | +----------------------------------------------------------------+ |
         | | ... = malloc(len + 2);                                         | |
         | +----------------------------------------------------------------+ |
         +-------------------------------------------------------------------*/
	if(fullpath == NULL){
		die("malloc"); // Mommy Melloc is gone ;_;
	}
	strcpy(fullpath, path);
	strcat(fullpath, "/");
	strcat(fullpath, entry->d_name); // <3 uwu join those strings
/*I----> +--------------------------------------------------------------------+
         | +----------------------------------------------------------------+ |
         | | snprintf(fullpath, len+2, "%s/%s", path, entry->d_name);       | |
         | +----------------------------------------------------------------+ |
         +-------------------------------------------------------------------*/
	
	struct stat file_stat;
	lstat(fullpath, &file_stat);
/*I----> +--------------------------------------------------------------------+
         | Fehlerbehandlung von `lstat` fehlt. (-0.5)                         |
         +-------------------------------------------------------------------*/
	if (S_ISDIR(file_stat.st_mode)) { // check if dir
		processDir(fullpath); //
	}else if(S_ISREG(file_stat.st_mode)){ // check if file.
		
		// Update our stats
		P(grepThreads);
		P(statLock);
		stats.activeGrepThreads+=1;
		V(newStat);
		V(statLock);
		
		// We need more THRDS
		pthread_t throed;
		errno = pthread_create(&throed, NULL, processFile, (void*) fullpath);
		if(errno != 0){die("pthread_create");}
		errno = pthread_detach(throed);
		if(errno != 0){die("pthread_detach");}		
	}
/*I----> +--------------------------------------------------------------------+
         | Die Freigabe  von `fullpath` fehlt,  welcher aktuell nur  im grep- |
         | Thread für durchsuchte Dateien  freigegeben wird. Eine Lösung wäre |
         | z.B.:                                                              |
         |                                                                    |
         | +----------------------------------------------------------------+ |
         | | if(!S_ISREG(file_stat.st_mode))                                | |
         | |     free(fullpath);                                            | |
         | +----------------------------------------------------------------+ |
         |                                                                    |
         | (-0.5)                                                             |
         +-------------------------------------------------------------------*/
	
	// <- ciao kakao
	return;
}

/**
 * \brief Acts as start_routine for grep-Threads and searches all lines of the file for the
 * search pattern.
 *
 * It updates the stats.files, stats.lines, stats.fileHits, stats.lineHits
 * stats.activeGrepThreads fields if necessary.
 *
 * \param path Path to the file to process
 *
 * \return Always returns NULL
 */
static void* processFile(void* path) {
	
	// Hit those lines
	int t_lineHits = 0;
	int t_lines = 0;

	// Open da file
	FILE *fp = fopen(path, "r");
	if(fp == NULL){
		die("fopen");
	}
	
	// line by line
	char *linebuf = malloc(sizeof(char)*(4096+2));
	if(linebuf == NULL){
		die("malloc"); // or die
	}
	
	// WAIT THERES STILL MOREEEE OwO
	while(fgets(linebuf, 4096+2, fp) != NULL){
		
		t_lines += 1;

		char *ptr = linebuf;	
		ptr = strstr(ptr, needle);

		if(ptr != NULL){
			// Holy fucking bingles we found something POG
			t_lineHits += 1;
		}
	
		memset(linebuf,0,strlen(linebuf));		
		
	}if (!feof(fp)){
        die("fgets"); // F
    }

	int e = fclose(fp);
	if(e != 0){die("fclose");} // WAIT WHY COULD THIS HAPPEN WTFASJASDKLMAL
	
	// finally
	free(linebuf); // free
	V(grepThreads);
	P(statLock);
	stats.lines+=t_lines;
	stats.lineHits+=t_lineHits;
	stats.activeGrepThreads-=1;
	stats.files+=1;
	if(t_lineHits!=0){stats.fileHits+=1;}
/*I----> +--------------------------------------------------------------------+
         | Die Aktualisierung der  Statistiken könnte auch  pro Zeile gemacht |
         | werden, so  würde schneller ein  Zwischenergebnis sichtbar werden, |
         | gerade dann, wenn die Dateien etwas länger sind.                   |
         +-------------------------------------------------------------------*/
	V(newStat);
	V(statLock);
	free(path);
	return NULL;
	
	
	// I hope the comments were as both fun and awful to read as this was
	// to program.
}


/*P----> +--------------------------------------------------------------------+
         | Fehlerabzug in dieser Datei: 2.5 Punkte                            |
         | ------------------------------------------------------------------ |
         | Insgesamt wurden 19.5 von 22.0 Punkten in dieser Datei erreicht.   |
         +-------------------------------------------------------------------*/
