#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>



void stringHandling(char *line);

void getInputInteractiveMode(){ 
	char line[1024];
	
	int bufLen =1024; //buffer lengths

	while(1){
		printf("witsshell> ");
		fgets(line, bufLen, stdin);

		stringHandling(line);
	}
	//free(line);
	
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
}
int main(int argc, char *argv[]){

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
