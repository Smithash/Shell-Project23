#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>

//Error message to be displayed
char error_message[30] = "An error has occurred\n";
//path to be used, default is /bin/
char** path;
//storing number of processes
int numProcess =0;
char* env[50];

void stringHandling(char *line);

//executing shell commands 
void exec_commands(char* argcVec[], int argCount){
	
	//if there is no arguments given
	if(argCount ==0){
		return;
	}

	//printf("%s %d\n", argcVec[0],strcmp(argcVec[0],"cd"));
	/*Checking if command is a built in command*/

	//Exit command 
	if(strcmp(argcVec[0], "exit")==0){
		exit(0);
	}

	//CD command 
	else if(strcmp(argcVec[0], "cd")==0){
		// We will have 2 arguments : cd and the new directory so our count is 2, if not 2 we have an error and if 0 then also have an error (as it fails)
		if(argCount ==2){
			 //if chdir returns a 0 then error
			
            if(chdir(argcVec[1])!=0)
            {
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
		}
	}
	//Path commmand 
	else if(strcmp(argcVec[0], "path")==0){
		numProcess = argCount -1;
		//loop counter
		int i;
		for(i=0; i<numProcess; i++){
			//overwrite env variable : overrides the old path
			env[i] = argcVec[i+1];
			
		}
	}
	//non built-in commands
	else{
		
		bool was_found = false;
		pid_t pid = fork();

		if(pid ==0){
			char binPath[256];
			for(size_t i=0; i < numProcess; i++){
				strcpy(binPath, path[i]);
				strcat(binPath, argcVec[0]);

				if(access(binPath, X_OK)== 0){
					execv(binPath, argcVec);
					was_found = true;
					break;
				}
			//end for loop
			}
		}
		if(!was_found){
			//error message 
		}
		else{
			wait(NULL);
		}
	}

	
}


void getInputInteractiveMode(){ 
	char line[1024];
	
	int bufLen =1024; //buffer lengths

	while(1){
		char cwd[128];
        getcwd(cwd, 128);

        fputs("witsh: ", stdout); fputs(cwd, stdout); fputs(" >> ", stdout);
		fgets(line, bufLen, stdin);

		stringHandling(line);
	}
	
}

//pointer = heap
//no pointer = stack
void getInputBatchMode(char *filepath){
	//Read in the file 
	//While not end of file and do string handling for each line

	FILE* file = fopen(filepath, "r"); //opening the file
	char buffer[1024]; //input line

	//check if the file is empty
	if(file == NULL){
		fputs("An Error has occurred, no file found", stderr);
	}
	//String handling of each line of the file
	while(fgets(buffer, 1024, file) !=NULL){
		stringHandling(buffer);
	}
	
	
}

void stringHandling(char* line){
	line[strcspn(line, "\n")] = 0; //Remove newline char
	//Separting the line into its components 
	//Use tokens where a token is a string separated by white spaces
	//use strsep()
	char **tokens; //2D array of chars = 1D array of strings 
	char *currentT; //current token = current string
	int count=1;
	int index =0; //token index in tokens
	int lenLine = sizeof(line)/sizeof(char); // gets the number of characters in the line
	for(int i=0; i< lenLine; i++){ //calculates how many words are in the line
		if(line[i] == ' '){
			count++;
		}
	}
	tokens = malloc(count * sizeof(char*));
	while((currentT = strsep(&line, " ")) != NULL){ //pulls out first string up until whitespace and puts in currentT and removes from line
	//then check that currentT is  empty which means its reached the end of the line
		tokens[index] = currentT;
		index++;
	}
	//String handling for Basic is done! the next string handling is for parallel

	exec_commands(tokens, count);
}

int main(int argc, char *argv[]){
	path = malloc(1*sizeof(char*));
	path[0] = "/bin/";
	if (argc == 1){
		getInputInteractiveMode();
	}
	else if(argc == 2){
		getInputBatchMode(argv[1]);
	}
	else{
		printf("\nInvalid number of arguments");
	}



	return(0);
}
