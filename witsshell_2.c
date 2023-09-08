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
int num_paths =0;

void stringHandling(char *line);

void parallel(char *argv[], int argc, char* filename){
	int i=0;
	int file =0;

	for(i=0; i<numProcess;i++){
		for(int j=0; j< num_paths; j++){
			char* command = malloc(strlen(path[j]) + strlen(argv[0])+1);

			command[0] = '\0';
			//copy current env (path) into command
			strcpy(command, path[j]);
			//add / if user forgets
			strcat(command, '/');
			strcat(command, argv[0]);

			//check access
			if(access(command, X_OK)==0){
				int pid = fork();

				if(pid ==0){
					

					//checking for file
					if(filename!= NULL){
						//redirection
						file = open(filename, O_RDWR| O_CREAT | O_TRUNC,0600);
						dup2(file, 1);
						close(file);
					}
					//execute the command
					execv(command, argv);
					exit(0);
				}
				break;
			}
			else{
				if(i==numProcess-1){ //error
					write(STDERR_FILENO, error_message, strlen(error_message));
					return;
				}
			}
		}	
	}

}

void redirection(char* argv[], int argc, int start, int end){ //redirection(argv, argc, start, end)
	int file =0;
	//loop through i: start to end
	for(int i=start; i<end; i++){
		if(argv[i]=='>'){
			file = open(argv[i+1], O_RDWR| O_CREAT | O_TRUNC,0600);
				dup2(file, 1);
				close(file);
		}
	}
	for(int j=0; j<num_paths;j++){
		char *command = malloc(strlen(path[j])+strlen(argv[0])+1);
		strcpy(command, path[j]);
		strcat(command, "/");
		strcat(command, argv[0]);

		if(access(command, X_OK) ==0){
			execv(command, argv);
		}
	}
	
		//if argv[i] = ">"
			//change stdout -> redirection
		//end if
	//end loop
	//loop through num_paths
		//strcat, strcpy to make command
		//if access ==0 : execv()



	// if(argc ==0){
	// 	return;
	// }

	// int i=0;

	// char* newArgv[50];

	// while(i<argc && strcmp(argv[i], ">")!=0){
	// 	newArgv[i] = argv[i];
	// 	i++;
	// }
	// //assign last element of argv to be null
	// newArgv[i] = NULL;
	// if(i==argc){
	// 	//need to execute now : do execution in the non-built-in function ->send to parallel in case of parallel command
	// 	parallel(newArgv, argc, NULL); //No file so no redirection but possible parallel
	// 	return;
	// }
	// else if(i == argc-1 || i < argc-2 || i==0){ //if we 1 before end or 2 befor end : error
	// 	write(STDERR_FILENO, error_message, strlen(error_message));
	// 	return;	
	// }
	// else{ 
	// 	int args = argc -2;
	// 	char* passArgs = argv[argc-1];
	// 	parallel(newArgv, args, passArgs);

	// 	return;
	// }
}

void exec_commands(char* argVec[], int argCount)
{
    //if no arguments given just return
    if(argCount==0)
    {
        return;
    }
    
    /*check if command is a built in command*/
    
    //if that command is exit
    if(strcmp(argVec[0], "exit")==0)
    {
        //for exit command, if there are more args than just exit, error
        if(argCount!=1)
        {
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        else
        {
            exit(0);
        }
    }
    //if command is cd
    else if(strcmp(argVec[0], "cd")==0)
    {
        //if we have 2 args-> cd=1 and the new direciry =2
        if(argCount==2)
        {
            //if chdir returns a 0 then error
            if(chdir(argVec[1])!=0)
            {
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
        }
        //if argc!=2 then we have an error
        else
        {
            write(STDERR_FILENO, error_message, strlen(error_message));
            return;
        }
    }
    
    //handle path
    else if(strcmp(argVec[0], "path")==0)
    {	free(path);
        //get num of processes
        num_paths=argCount-1;
		path = malloc(num_paths*sizeof(char*));
        //create loop counter
        int j;
        for(j=0; j<argCount-1; j++)
        {	path[j] = malloc((strlen(argVec[j])+1)*sizeof(char));
            //overwrite any path variable
			strcpy(path[j], argVec[j+1]);
            
        }
    }
    
    //not a built in function
    else
    {	//add fork results to an array
		//loop through tokens array
		int start=0;
		int i=0;
		int* fork_result[256];
		while( i< argCount){
			if(argVec[i] =='&'){
				pid_t pid =fork();
				if(pid ==0){
					redirection(argVec, argCount, start, i-1);
				}	
			}
				i++;
			}
		pid_t pid = fork();
		if(pid == 0){
			redirection(argVec, argCount, start, argCount-1);
		}
		else{
			waitpid(pid, NULL, 0);
		}
		
		//var start=0
		//while i < argc 

			//if i = & then 
				//fork()
				//pid ==0{
				//redirection(argV, argc, start, i-1)
				//}
				//start = i +1
				
			//end if
			//i++
		//end of while
		//fork()
		//pid ==0
			//redirect(argv, argc, start, argc-1)
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

// void stringHandling(char* line){
// 	line[strcspn(line, "\n")] = 0; //Remove newline char
// 	//Separting the line into its components 
// 	//Use tokens where a token is a string separated by white spaces
// 	//use strsep()
// 	char **tokens; //2D array of chars = 1D array of strings 
// 	char *currentT; //current token = current string
// 	int count=1;
// 	int index =0; //token index in tokens
// 	int lenLine = sizeof(line)/sizeof(char); // gets the number of characters in the line
// 	for(int i=0; i< lenLine; i++){ //calculates how many words are in the line
// 		if(line[i] == ' '){
// 			count++;
// 		}
// 	}
// 	tokens = malloc(count * sizeof(char*));
// 	while((currentT = strsep(&line, " ")) != NULL){ //pulls out first string up until whitespace and puts in currentT and removes from line
// 	//then check that currentT is empty which means its reached the end of the line
// 			tokens[index] = currentT;
// 			index++;

// 	}
// 	for(int i=0; i<count; i++){
// 		printf("%s\n", tokens[i]);
// 	}
		
// 	exec_commands(tokens, count);
// }

void stringHandling(char* input)
{
    int i = 0;
    //vector of arguments
    char *argVec[256  * sizeof(char)];
    //number of arguments in the line of input
    int argCount = 0;
    
    //handle splitting the input string
    //loop while i is less than the inut strings len and while we are not at the end of the line
    while(i<strlen(input) && input[i]!='\n')
    {
        //skip over where the input is just a space
        if(input[i]!=' ')
        {
            //check for '>' indicating there is a redirect
            if(input[i]=='>')
            {
                //create a temp arg
                char *arg=malloc(256*sizeof(char));
                arg[0]='>';
                //null character value
                arg[1]='\0';
                //place this arg in the arg vector at argc
                argVec[argCount]=arg;
                //increment num of args
                argCount++;
                //increent loop counter
                i++;
                continue;
            }
            
            //check if '&' indicating parallel commands
            if(input[i]=='&')
            {
                //create a temp arg
                char *arg=malloc(256*sizeof(char));
                arg[0]='&';
                //null character value
                arg[1]='\0';
                //place this arg in the arg vector at argc
                argVec[argCount]=arg;
                //increment num of args
                argCount++;
                //increent loop counter
                i++;
                continue;
            }
            
            //declare a new var t, this will be used
            int t = 0;
            //create new arg varibale, will hold arguments
            char *arg = malloc(256  * sizeof(char));
            //loop while less than size of string (access every elem) and whle we not at the emd of the line
            while(i<strlen(input) && input[i]!=' ' && input[i]!='\n')
            {
                //if we have no special char (parallel and redirct)
                if(input[i]!= '&' && input[i]!='>')
                {
                    //set arg[t] to be the word in the input strig at i, from outer loop
                    arg[t] = input[i];
                    //increment t and i
                    i++;
                    t++;
                }
                else
                {
                    break;
                }
            }
            //after we are done adding all the words from the input, add the nul character value
            arg[t] = '\0';
            //add the arg to the arg vector at the pos
            argVec[argCount] =arg;
            //increment the num of args
            argCount++;
        }
        //otherwise increment i if not an empty space
        else
        {
            i++;
        }
        
    }
	for(int i=0; i<argCount; i++){
		printf("%s\n", argVec[i]);
	}
    exec_commands(argVec, argCount);
}

int main(int argc, char *argv[]){
	path = malloc(1*sizeof(char*));
	path[0] = "/bin/";
	num_paths = 1;
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
