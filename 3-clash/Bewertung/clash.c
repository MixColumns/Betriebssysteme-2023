// Korrektur: XXXXXXXXXXX
// Punkte: 12 von 18



#include "plist.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


static void printDir(){
	// because fuck you OS
	char* buffer = NULL;
	int buffsize = 256;
	buffer = malloc(sizeof(char) * buffsize);
	if(buffer == NULL){perror("malloc"); exit(EXIT_FAILURE);} // suicide
	while(1){
		if(getcwd(buffer, buffsize-1) != NULL){
			printf("%s: ", buffer); // it worked and all it took was
			// like 20 lines of fucking code to print the dir what a joke,
			// give me a
			break;
		}
		if(errno == ERANGE){
			buffsize = buffsize * 2;
			buffer = (char*) realloc (buffer, buffsize);
			if(buffer == NULL){perror("malloc"); exit(EXIT_FAILURE);} // suicide
		}else{
			perror("GETCWD");
			exit(EXIT_FAILURE); // suicide
		}
	}
	fflush(NULL);
}

static _Bool callBackJobs(pid_t a, const char * c){

	printf("%i %s\n", a, c);
	fflush(NULL);
	return false;
}

int main (int argc, char* argv[]){

	printDir();

	char command[1338];
	memset(command, 0, sizeof(command));

/*I----> +--------------------------------------------------------------------+
         | hier muss  natürlich dann auch  1338 hin, da  das \0 byte  am Ende |
         | noch kommt (-0.5)                                                  |
         +-------------------------------------------------------------------*/

	while(fgets(command, 1337, stdin) != NULL){

		// Check if there are any zombies
		pid_t p;
		int s;
		while((p = waitpid(-1, &s, WNOHANG)) != 0){
			if(p < 0){
				if(errno == ECHILD){
					break;
				}
				perror("waitpid"); exit(EXIT_FAILURE);
			}
			char* cmd = malloc(1338*sizeof(char));
			if(cmd == NULL){perror("malloc"); exit(EXIT_FAILURE);}
			removeElement(p, cmd, 1338);

/*I----> +--------------------------------------------------------------------+
         | WEXITSTATUS soll  laut  manpage  nur dann  verwendet  werden, wenn |
         | WIFEXITED true liefert. (-1.0)                                     |
         +-------------------------------------------------------------------*/

			printf("Exitstatus [%s] = %d\n", cmd ,WEXITSTATUS(s));
		}

/*I----> +--------------------------------------------------------------------+
         | Dein Code springt  hier nicht in die if,  da deine Bedingung nicht |
         | korrekt ist. Mit  der Annahme, dass du  oben fgets(1338) aufrufst, |
         | gilt  für  eine  überlange  Zeile  if(strlen(command)  ==  1337 && |
         | command[1336] != '\n') (-0.5)                                      |
         +-------------------------------------------------------------------*/

		// Check if command is too long so we can say "No GTFO RTFM"
		if(command[1337] != '\n' && command[1337] != '\0'){
			memset(command, 0, sizeof(command));
			printf("Error: Command too long, ignored");
			char c;while((c=(char)getchar())!='\n'&&c!=EOF){}
			printDir();
			continue;
		}

		// Empty line do nothing
		if(command[0] == '\0' || command[0] == '\n' || command[0] == '&'){
			memset(command, 0, sizeof(command));
			printDir();
			continue;
		}

		// Remove trailing newline if any
		int len = strlen(command);
		if(len > 0 && command[len-1]=='\n'){
			command[len-1] = '\0';
		}

		// Check for & in command
		int bg = 0;
		len = strlen(command);
		if(len > 0 && command[len-1]=='&'){
			bg = 1;
			command[len-1]='\0';
		}

		// Create Backup of command as strtok modifies
		char command_backup[1338];
		strcpy(command_backup, command);

		// Extract Tokens from Input
		char* nextToken;
		int tokens = 0;
		char* tokenList[1000]; // Could max be ~ 1337 / 2
		for(int i=0; i<1000; i++){tokenList[i] = NULL;}

		// Fetch first token
		nextToken = strtok(command_backup, " \t");

/*I----> +--------------------------------------------------------------------+
         | Wenn " \n" (<leer> + enter) bzw. "\t\n" (<tab> + enter) eingegeben |
         | wird, dann ist hier  nextToken = NULL, da ja bei  " \t" mit strtok |
         | oben geparsed  wird. Dadurch kommt  es dann beim  Aufruf unten mit |
         | char*  tmp    =    malloc((strlen(nextToken)+1)*sizeof(char));  => |
         | strlen(NULL) zur einem Segmentation Fault (-1). Du kannst das z.B. |
         | beheben, indem du unter nextToken = strtok(command_backup, " \t"); |
         | so etwas wie: if(!nextToken) {printDir(); continue;} schreibst.    |
         +-------------------------------------------------------------------*/


		// Malloc Token
		char* tmp = malloc((strlen(nextToken)+1)*sizeof(char));
		if(tmp == NULL){perror("malloc"); exit(EXIT_FAILURE);}
		// Copy Token to malloced Space
		strcpy(tmp, nextToken);
		// Add Token to List
		tokenList[tokens] = tmp;
		// Change amount of tokens
		tokens+=1;
		// Fetch other tokens as above
		while(nextToken != NULL){
			nextToken = strtok(NULL, " \t");
			if (nextToken != NULL){
				char* tmp = malloc((strlen(nextToken)+1)*sizeof(char));
				if(tmp == NULL){perror("malloc"); exit(EXIT_FAILURE);}
				strcpy(tmp, nextToken);
				tokenList[tokens] = tmp;
				tokens+=1;
			}
		}



		// Check if the Command is chdir
		if(strcmp(tokenList[0], "cd") == 0){
			if(tokenList[1] == NULL){
				printf("usage: cd <directory>\n"); // RTFM
			}else{
				if(chdir(tokenList[1]) != 0){

/*I----> +--------------------------------------------------------------------+
         | besser mit  perror  die  Fehlermeldung  ausgeben,  wenn  die errno |
         | gesetzt wird (perror greift automatisch  auf die gesetzte errno zu |
         | ohne dass man die errno direkt lesen muss) (siehe man 3p perror)   |
         +-------------------------------------------------------------------*/

					int err = errno;

/*I----> +--------------------------------------------------------------------+
         | Fehlermeldungen auf stderr ausgeben (-0.5)                         |
         +-------------------------------------------------------------------*/

					printf("chdir: %s\n", strerror(err));
				}
			}
			// Clean Command
			memset(command, 0, sizeof(command));
			// Free Token List
			for(int i=0; i<tokens; i++){if(tokenList[i] != NULL){
				free(tokenList[i]);}
				tokenList[i] = NULL;
			}
			printDir();
			continue;
		}

		// If Command is jobs show processes
		if(strcmp(tokenList[0], "jobs") == 0){

			walkList(callBackJobs);

			// Clean Command
			memset(command, 0, sizeof(command));
			// Free Token List
			for(int i=0; i<tokens; i++){if(tokenList[i] != NULL){
				free(tokenList[i]);}
				tokenList[i] = NULL;
			}
			printDir();
			continue;
		}

		// Fork and execute Command
		int pid = fork();
		if(pid < 0){

/*I----> +--------------------------------------------------------------------+
         | perror verwenden plus Fehlermeldung auf stderr (s.o)               |
         +-------------------------------------------------------------------*/

			printf("fork: %s\n", strerror(errno));
			// Clean Command
			memset(command, 0, sizeof(command));
			// Free Token List
			for(int i=0; i<tokens; i++){if(tokenList[i] != NULL){
				free(tokenList[i]);}
				tokenList[i] = NULL;
			}
			printDir();
			continue;
		}

/*I----> +--------------------------------------------------------------------+
         | Hinweis: Hier ist es  nicht zwingen nötig auch Vordergrundprozesse |
         | in  die  Liste  einzuhängen,  da  diese  dann  direkt  nach  deren |
         | Beendigung wieder entnommen werden.                                |
         +-------------------------------------------------------------------*/

		if(-2 == insertElement(pid, command)){
			fprintf(stderr, "out of memory");

/*I----> +--------------------------------------------------------------------+
         | exit fehlt (-0.5)                                                  |
         +-------------------------------------------------------------------*/
		}

		if(pid == 0){
			// Child
			execvp(tokenList[0], tokenList);

/*I----> +--------------------------------------------------------------------+
         | perror fehlt (-0.5)                                                |
         +-------------------------------------------------------------------*/

			exit(EXIT_FAILURE);
		}else{
			// Parent
			if(bg){

			}else{
				int status = 0;
				if(waitpid(pid, &status, 0) < 0){
					perror("waitpid");
					exit(EXIT_FAILURE);
				}
				char* cmd = malloc(1338*sizeof(char));
				if(cmd == NULL){perror("malloc"); exit(EXIT_FAILURE);}
				removeElement(pid, cmd, 1338);

/*I----> +--------------------------------------------------------------------+
         | WEXITSTATUS soll  laut  manpage  nur dann  verwendet  werden, wenn |
         | WIFEXITED true liefert. (-1.0)                                     |
         +-------------------------------------------------------------------*/

				printf("Exitstatus [%s] = %d\n", cmd ,WEXITSTATUS(status));
			}
		}

		// Reset Stuff
		// Clean Command
		memset(command, 0, sizeof(command));
		// Free Token List
		for(int i=0; i<tokens; i++){if(tokenList[i] != NULL){
			free(tokenList[i]);}
			tokenList[i] = NULL;
		}
		printDir();
	}
	// Fgets fucked up so we jump out the window
	if(!feof(stdin)){perror("FGETS");exit(EXIT_FAILURE);}
	fflush(NULL);
}


/*P----> +--------------------------------------------------------------------+
         | Punktabzug in dieser Datei: 5.5 Punkte                             |
         +-------------------------------------------------------------------*/
