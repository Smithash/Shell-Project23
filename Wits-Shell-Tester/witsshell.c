#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>


//message to be displayed when an error occurs
char error_message[30]="An error has occurred\n";
//path to be used, by default its /bin/ but can be changed with path command
char* path="/bin/";
//store the num of processes
int num_processes=0;
char* env[50]; //env is an environmental variable used to store of folder locations the shell will search through



void parallel(char *argv[],int argc, char* filename){
    int i = 0;
    int file= 0;
    //loop over num processes
    for(i=0;i<num_processes;i++){
        //commandLine will contain the path, "/" if user forgets and the commands i.e ls
        char *commandLine = malloc(strlen(env[i])+strlen(argv[0])+1);
        commandLine[0] = '\0';              //set null character
        strcpy(commandLine, env[i]);        //copy curr env into fullName
        strcat(commandLine, "/");           //add / in case user dosent
        strcat(commandLine, argv[0]);  //add the arguements 



        //check access
        if(access(commandLine,X_OK)==0){
            //fork a n process
            int pid = fork();
            //child process executes the command
            if(pid==0){
                //set last elem of args to null
                argv[argc] = NULL;
                //if we have a file (i.e redirect)
                if(filename!=NULL){ 
                    file = open(filename, O_RDWR|O_CREAT|O_TRUNC, 0600);
                    //create copy of file descripter
                    dup2(file, 1);
                    close(file);
                }
                //excute the command
                execv(commandLine, argv);
                exit(0);
            }
            break;
        }
        else
        {
            if(i==num_processes-1){
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
        }
    }
}


void redirection(char* argv[], int argc)
{
    //cannt have no arguments
    if(argc==0)
    {
        return;
    }
    
    
    int i=0;
    //stores the new arguments 
    char* newargv[50];
    
    //loop while i is less than num of args in arg vec and while we are not reading '>' indicating a redirect
    while(i<argc && strcmp(argv[i], ">")!=0)
    {
        newargv[i]=argv[i];
        i++;
    }
    //assign last elem of argv to be null
    newargv[i]=NULL;
    //if we are at end of arguments array when we exit the while loop, need to now execute -> send to parallel incase there is parallel command
    if(i==argc)
    { //no redirect (no output file) but possible parallel command
        parallel(newargv, argc, NULL);
        return;
    }
    
    else if(i==argc-1 || i<argc-2 || i==0)
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }
    else
    { //normal redirection
        int args=argc-2;
        char* passedArgs=argv[argc-1];
        
        parallel(newargv, args, passedArgs);
        return;
    }
}

void exec_commands(char* argv[], int argc)
{
    //if no arguments given just return
    if(argc==0)
    {
        return;
    }
    
    /*check if command is a built in command*/
    
    //if that command is exit
    if(strcmp(argv[0], "exit")==0)
    {
        //for exit command, if there are more arguments than just exit, error
        if(argc!=1)
        {
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        else
        {
            exit(0);
        }
    }
    //if command is cd
    else if(strcmp(argv[0], "cd")==0)
    {
        //if we have 2 args, cd and the directory (only 2 allowed arguments)
        if(argc==2)
        {
            //if chdir returns a 0 then error
            if(chdir(argv[1])!=0)
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
    else if(strcmp(argv[0], "path")==0)
    {
        //get num of processes
        num_processes=argc-1;
        //create loop counter
        int i;
        for(i=0; i<argc-1; i++)
        {
            //overwrite the original path
            env[i]=argv[i+1];
        }
    }
    
    //not a built in function
    else
    {
        //check if we have processes>0
        if(num_processes==0)
        {
            //write error
            write(STDERR_FILENO, error_message, strlen(error_message));
            return;
        }
        
        //init loop counter i
        int i=0;
        //init the number of command
        int numCommand=0;
        while(i<argc)
        {
            //create array of current args
            char* currArgs[50];
            //another loop counter
            int j=0;
            //loop while we are less than the num of elems nad none of them are indicating  parallel command
            while(i<argc && strcmp(argv[i], "&")!=0)
            {
                //assign our curr arguments vec at index j to be the arg in our arg vec at i
                currArgs[j]=argv[i];
                //increment both counters
                i++;
                j++;
            }
            //increment the number of commands
            numCommand++;
            //set currargs to null (erase what the inner while did)
            currArgs[j]=NULL;
            //handle redirect
            redirection(currArgs,j);
            i++;
        }
        while(numCommand-- >0)
        {
            wait(NULL);
        }
    }
}

//execute function, takes in input line
void stringHandling(char* line)
{
    int i = 0;
    //vector of arguments
    char *argv[256  * sizeof(char)];
    //number of arguments in the line of input
    int count = 0;
    
    //handle splitting the input string
    //loop while i is less than the inut strings len and while we are not at the end of the line
    while(i<strlen(line) && line[i]!='\n')
    {
        //skip over where the input is just a space
        if(line[i]!=' ')
        {
            //check for '>' indicating there is a redirect
            if(line[i]=='>')
            {
                //create a temp arg
                char arg[256];
                arg[0]='>';
                //null character value
                arg[1]='\0';
                //place this arg in the arg vector at argc
                argv[count]=arg;
                //increment num of args
                count++;
                //increent loop counter
                i++;
                continue;
            }
            
            //check if '&' indicating parallel commands
            if(line[i]=='&')
            {
                //create a temp arg
                //char *arg=malloc(256*sizeof(char));
                char arg[256];
                arg[0]='&';
                //null character value
                arg[1]='\0';
                //place this arg in the arg vector at argc
                argv[count]=arg;
                //increment num of args
                count++;
                //increent loop counter
                i++;
                continue;
            }
            
            //declare a new var t, this will be used
            int t = 0;
            //create new arg varibale, will hold arguments
            char *arg = malloc(256  * sizeof(char));
            //loop while less than size of string (access every elem) and whle we not at the emd of the line
            while(i<strlen(line) && line[i]!=' ' && line[i]!='\n')
            {
                //if we have no special char (parallel and redirct)
                if(line[i]!= '&' && line[i]!='>')
                {
                    //set arg[t] to be the word in the input strig at i, from outer loop
                    arg[t] = line[i];
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
            argv[count] =arg;
            //increment the num of args
            count++;
        }
        // increment i if not an empty space
        else
        {
            i++;
        }
        
    }
    exec_commands(argv, count);
}
void getInputInteractiveMode(){ 
	char line[1024];
	int bufLen =1024; //buffer lengths

    env[0] = path;
    num_processes = 1;
	
	while(1){
		char cwd[128];
        getcwd(cwd, 128);

        fputs("witsh: ", stdout); fputs(cwd, stdout); fputs(" >> ", stdout);
		fgets(line, bufLen, stdin);

		stringHandling(line);
	}
	
}

void getInputBatchMode(char *filepath){
	//Read in the file 
	//While not end of file and do string handling for each line

	FILE* file = fopen(filepath, "r"); //opening the file
	char buffer[1024]; //input line
	
    env[0] = path;
    num_processes = 1;

	if(file==NULL){
            //if no file, produce error and exit
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
    }
	//String handling of each line of the file
	while(fgets(buffer, 1024, file) !=NULL){
		stringHandling(buffer);
	}
	
	
}

int main(int argc, char *argv[]){
    
    //Interactive mode
    if (argc == 1){ 
		getInputInteractiveMode();
	}
    //batch mode
	else if(argc == 2){
		getInputBatchMode(argv[1]);
	}
    //Error
    else
    {
        
        env[0]=path;
        num_processes=1;
        //an error has occured
        write(STDERR_FILENO, error_message, strlen(error_message));
        //exit program if error
        exit(1);
    }
    return 0;
}
