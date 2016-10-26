
/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <wait.h>
#include <regex.h>
#include <dirent.h>
#include <cassert>
#include <vector>
#include <pwd.h>


#include "command.h"


SimpleCommand::SimpleCommand()
{
	// Create available space for 5 arguments
	_numOfAvailableArguments = 5;
	_numOfArguments = 0;
	_arguments = (char **) malloc( _numOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	//char * actual = (char*)calloc(1024, sizeof(char));
	//fprintf(stderr, "argument is : %s\n", argument);
	if ( _numOfAvailableArguments == _numOfArguments  + 1 ) {
		// Double the available space
		_numOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numOfAvailableArguments * sizeof( char * ) );
	}
	//
	//Tilde Expansion part
	//
	if(argument[0] == '~'){
		if(strlen(argument) == 1){
			_arguments[_numOfArguments] = strdup( getenv("HOME"));
		}
		else{
			char * dup = strdup(argument);
	//		fprintf(stderr, "argument is: %s\n", dup);
			char * user = (char *)calloc(512, sizeof(char));
			char * realUser = user;
			dup++;
			while(*dup != '/' && *dup != '\0'){
				*user = *dup;
				user++;
				dup++;
			}
			*user = '\0';
	//		fprintf(stderr, "user is: %s\n", realUser);
			struct passwd *  pw = getpwnam(realUser);
			char * realArgTilde = (char*)calloc(1024, sizeof(char));
			strcat(realArgTilde, pw->pw_dir);
			char * slash = strchr(dup, '/');
			if(slash){
				strcat(realArgTilde, slash);
	//			fprintf(stderr, "real path is: %s\n", realArgTilde);
				_arguments[_numOfArguments] = strdup(realArgTilde);
			}else{
				_arguments[_numOfArguments] = strdup(pw->pw_dir);
			}
			
		}
	}
	//
	//Variable Expansion part
	else if(strchr(argument, '$')){
		char * r = (char*)malloc(19 * sizeof(char));
		//char * r = "^.*${[^}]*}.*$";
		strcpy(r, "^.*${[^}]*}.*$");
		//char * reg = strdup(r);
		//fprintf(stdout, "%s\n", r);
		regex_t re;
		int expbuf = regcomp(&re, r, 0);
		if(expbuf != 0){
			printf("regex compilation failed\n");
		}else{
			regmatch_t match;
			int counter = 0;
			char * realArg = (char * )calloc(1024, sizeof(char));
				if(regexec(&re, argument, 5, &match, 0) == 0){
					char * arg = strdup(argument);
					while(*arg != '\0'){
						if(*arg == '$'){
							char * vvarName = (char *)calloc(50, sizeof(char));
							char * rrealVar = vvarName;
							arg = arg + 2;
							while(*arg != '}'){
								*vvarName = *arg;
								vvarName++;
								arg++;
							}//while
							*vvarName = '\0';
							if(!strcmp(rrealVar, "$")){
								pid_t cpid = getpid();
								char * eenv = (char*)calloc(1024, sizeof(char));
								sprintf(eenv, "%ld", (long)cpid);
								strcat(realArg, eenv);
								counter += strlen(eenv);
							}
							else{
								char * eenv = getenv(rrealVar);
								strcat(realArg, eenv);
								counter += strlen(eenv);
							}
						}
						if(*arg >= 97 && *arg <=122){
							realArg[counter] = *arg;
							counter++;
							
						}
						arg++;
					}//while
					realArg[counter] = '\0';
					//actual = realArg；
				}
				_arguments[ _numOfArguments ] = realArg;
			}
		}else{
			//argument = actual;
			_arguments[ _numOfArguments ] = argument;
		}
		

	// Add NULL argument at the end
	_arguments[ _numOfArguments + 1] = NULL;

	_numOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numOfSimpleCommands = 0;
	_outFile = 0;
	_inFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numOfAvailableSimpleCommands == _numOfSimpleCommands ) {
		_numOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}

	_simpleCommands[ _numOfSimpleCommands ] = simpleCommand;
	_numOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}

		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile ) {
		free( _outFile );
	}

	if ( _inFile ) {
		free( _inFile );
	}

	if ( _errFile ) {
		free( _errFile );
	}

	_numOfSimpleCommands = 0;
	_outFile = 0;
	_inFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");

	for ( int i = 0; i < _numOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numOfArguments; j++ ) {
			if(j == _simpleCommands[i]->_numOfArguments - 1)
				printf("\"%s\" \t\n", _simpleCommands[i]->_arguments[j]);
			else
				printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inFile?_inFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );

}

int compareFunction(const void * one, const void * two){
	return (strcmp(*(char **)one,*(char **) two));
}

//int max_entries = 40;
//int num_entry = 0;
//char ** array = (char**)malloc(max_entries * sizeof(char*));
//int flag = 0;
int max_entries, num_entry;
char ** array;
char dot;
//DIR * d;

void SimpleCommand::wildcardExpansion(char * prefix, char * suffix, int depth){
	//int max_entries;
	//int num_entry;
	if(depth == 0){
		max_entries = 40;
		num_entry = 0;
		array = (char**)calloc(max_entries, sizeof(char*));
		dot = suffix[0];
	}
	if(*suffix == '\0'){
		/*if(dot == '.'){
			if(num_entry == max_entries){
				max_entries *= 2;
				array = (char**)realloc(array, max_entries * sizeof(char*));
				assert(array != NULL);
			}
			char * dup = strdup(prefix);
			dup[strlen(dup) - 1] = '\0';
			array[num_entry] = dup;
			num_entry++;
		}
		return;

		}*/
		//fprintf(stdout, "number of entry in array is: %d\n", num_entry);
			if(num_entry == max_entries){
				max_entries *= 2;
				array = (char**)realloc(array, max_entries * sizeof(char*));
				assert(array != NULL);
			}
			char * dup = strdup(prefix);
			//char * slash = strchr(dup, '/');
			//char * real = (char*)malloc(1024 * sizeof(char));
			//strncpy(real, dup, slash - dup);
			dup[strlen(dup) - 1] = '\0';
		//	fprintf(stdout, "-------------------------%s\n", dup);
			array[num_entry] = dup;
			num_entry++;
			return;
		}
		//return;

	char * component = (char*)calloc(1024, sizeof(char));
	char * s = strchr(suffix, '/');
	if(s){
		strncpy(component, suffix, s-suffix);
		suffix = s + 1;
	//	fprintf(stdout, "component is: %s\n", component);
	}
	else{
		strcpy(component, suffix);
		suffix = suffix + strlen(suffix);
	}
	//flag = 1;
	char * newPrefix = (char*)calloc(1024, sizeof(char));
	if(!strchr(component, '*') && !strchr(component, '?')){
		sprintf(newPrefix, "%s%s%s", prefix, component, "/");
		wildcardExpansion(newPrefix, suffix, depth+1);
		//for(int i = 0; i < num_entry ;++i)
		if(depth == 0){
			qsort(array, num_entry, sizeof(char*), compareFunction);
			for(int i = 0; i < num_entry; ++i)
				Command::_currentSimpleCommand->insertArgument(array[i]);
			free(array);
			//closedir(d);
			return;
		}
		//closedir(d);
		return;
	}
	char * regex = (char*)calloc(1024, sizeof(char));
	char * aa = component;
	char * rr = regex;
	*rr = '^';
	++rr;
	while(*aa){
		if(*aa == '*'){
			*rr = '.';
			++rr;
			*rr = '*';
			++rr;
		}
		else if(*aa == '?'){
			*rr = '.';
			++rr;
		}
		else if(*aa == '.'){
			*rr = '\\';
			++rr;
			*rr = '.';
			++rr;
		}
		else{
			*rr = *aa;
			++rr;
		}
		aa++;
	}
	*rr = '$';
	++rr;
	*rr = '\0';
	//fprintf(stdout, "%s\n", regex);
	regex_t rre;
	int expbuf = regcomp(&rre, regex, REG_EXTENDED|REG_NOSUB);
	if(expbuf != 0){
		//perror("regcomp failed\n");
		return;
	}
	char * dir;
	if(prefix[0] == '\0')
		dir = ".";
	else
		dir = strdup(prefix);
	//fprintf(stdout, "path is: %s\n", dir);
	DIR * d = opendir(dir);
	//fprintf(stdout, "GOT HERE11111111111\n");
	if(!d){
		//perror("open directory failed\n");
		return;
	}
	struct dirent * ent;
	while((ent = readdir(d)) != NULL){
		//fprintf(stdout, "GOT HERE2222222222\n");
		regmatch_t mmatch;
		if(regexec(&rre, ent->d_name, 5, &mmatch, 0) == 0){
			//flag = 1;
			sprintf(newPrefix, "%s%s%s", prefix, ent->d_name, "/");
			//fprintf(stdout, "GOT HERE2222222222\n");
			//fprintf(stdout, "new prefix is: %s\n", newPrefix);
			if(dot != '.'){
				if(ent->d_name[0] != '.'){
					wildcardExpansion(newPrefix, suffix, depth+1);
				}
			}
			else{
				wildcardExpansion(newPrefix, suffix, depth+1);
			}
			//fprintf(stdout, "current depth: %d\n", depth);

		}
	}
	//closedir(d);
	if(depth == 0){
			qsort(array, num_entry, sizeof(char*), compareFunction);
			for(int i = 0; i < num_entry; ++i)
				Command::_currentSimpleCommand->insertArgument(array[i]);
			free(array);
			//closedir(d);
			return;
	}


	closedir(d);


}




void cd(char * directory){
	//char * path = (char*)malloc((strlen(directory) + 5) * sizeof(char));
	//fprintf(stdout, "directory is: %s\n", directory);
	if(directory[0] == '/'){
		char * path = (char*)malloc((strlen(directory) + 2) * sizeof(char));
		strcat(path, directory);
		strcat(path, "/");
		chdir(path);
		setenv("PWD", path, 1);
		return;
	}
	if(!strcmp(directory, ".")){
		const char * homePath = getenv("HOME");
		chdir(homePath);
		setenv("PWD", homePath, 1);
		return;
	}
	char * buff = (char*)malloc(1024 * sizeof(char));
	char * path = getcwd(buff, 1024);
	//fprintf(stdout, "string length of path is: %d\n", strlen(path));
	char * tail = path + strlen(path);
	//fprintf(stdout, "tail is pointint at: %c\n", *tail);
	if(!strcmp(directory, "..")){
		char * prevDir = (char*)malloc(strlen(path) * sizeof(char));
		int backCounter = 0;
		char * tail = path + strlen(path) - 1;
		while(backCounter < strlen(path)){
			if(*tail != '/'){
				tail--;
				backCounter++;
			}
			else{
				strncpy(prevDir, path, tail-path+1);
				//fprintf(stdout, "previous directory is: %s\n", prevDir);
				break;
			}
		}
		chdir(prevDir);
		setenv("PWD", prevDir, 1);
		return;
	}
	//path[0] = '.';
	//path[1] = '/';
	strcat(path,"/");
	strcat(path, directory);
	//fprintf(stdout, "directory is going to be set to: %s\n", path);
	int ret = chdir(path);
	if(ret == -1){
		perror("failed to change PWD\n");
		exit(1);
	}
	setenv("PWD", path, 1);
	return;

}


extern char ** environ;
int setPrompt = 0;

void
Command::execute()

{
	// Don't do anything if there are no simple commands
	//printf("number of simple command: %d\n", _numOfSimpleCommands);
	if ( _numOfSimpleCommands == 0 ) {
		clear();
		prompt();
		return;
	}
	//Check if the command is exit, if yes, gracefully exit the shell.
	if(_numOfSimpleCommands == 1){
		if(!strcmp(_simpleCommands[0]->_arguments[0],"exit")){
			//printf("Bye! Have A Good Day! \n");
			exit(0);
		}
	}

	if(_numOfSimpleCommands == 1){
		if(!strcmp(_simpleCommands[0]->_arguments[0], "cd")){
			if(!(_simpleCommands[0]->_arguments[1])){
				cd(".");
				clear();
				prompt();
				return;
			}
			cd(_simpleCommands[0]->_arguments[1]);
			clear();
			prompt();
			return;
		}
	}

	if(_numOfSimpleCommands == 1){
		if(!strcmp(_simpleCommands[0]->_arguments[0], "setenv")){
			setenv(_simpleCommands[0]->_arguments[1], _simpleCommands[0]->_arguments[2], 1);
			if(!strcmp(_simpleCommands[0]->_arguments[1], "PROMPT"))
				setPrompt = 1;
			clear();
			prompt();
			return;
		}
	}

	if(_numOfSimpleCommands == 1){
		if(!strcmp(_simpleCommands[0]->_arguments[0], "unsetenv")){
			unsetenv(_simpleCommands[0]->_arguments[1]);
			if(!strcmp(_simpleCommands[0]->_arguments[1], "PROMPT"))
				setPrompt = 0;
			clear();
			prompt();
			return;
		}
	}

	if(_numOfSimpleCommands == 1){
		if(!strcmp(_simpleCommands[0]->_arguments[0], "jobs")){

		}
	}


	//save defaut in/out


	// Print contents of Command data structure
	//print();

	// Add execution here
	// For every simple command fork a new process
	// Setup i/o redirection
	// and call exec

	//save defaut in/out
	int default_in = dup(0);
	int default_out = dup(1);
	int default_err = dup(2);

	//set the initial Input
	int init_in;
	if(_inFile){
		init_in = open(_inFile, O_RDONLY);
		if(init_in < 0){
			//perror("open file failed\n");
			exit(2);
		}
	}
	else{
		init_in = dup(default_in);
	}

	int pid;
	int init_out;
	int init_err;
	for(int i = 0; i < _numOfSimpleCommands; ++i){
		//redirect Input
		dup2(init_in, 0);
		close(init_in);
		//set up output
		if(i == _numOfSimpleCommands - 1){
			if(_errFile){
				if(_append){
					init_err = open(_errFile, O_WRONLY | O_APPEND);
				}else{
					init_err = open(_errFile, O_WRONLY | O_CREAT, 0666);
				}
				if(init_err < 0){
					printf("err file failed\n");
					exit(2);
				}
			}else{
				init_err = dup(default_err);
			}
			if(_outFile){
				if(_append){
					init_out = open(_outFile, O_WRONLY | O_APPEND);
				}else{
					init_out = open(_outFile, O_WRONLY | O_CREAT, 0666);
				}
				//init_out = open(_outFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
				if(init_out < 0){
					//perror("open file failed\n");
					exit(2);
				}
			}
			else{
				init_out = dup(default_out);
				//init_err = dup(default_err);
			}
		}
		else{
			int fdpipe[2];
			/*if(pipe(fdpipe) == -1){
				perror("pipe failed\n");
				exit(2);
			}*/

			pipe(fdpipe);
			init_out = fdpipe[1];
			init_in = fdpipe[0];
		}

		//redirect output
		dup2(init_out, 1);
		close(init_out);
		if(_errFile){
			dup2(init_err, 2);
			close(init_err);
		}

		//child process
		pid = fork();
		if(pid < 0){
			perror("fork failed\n");
			exit(2);
		}
		if(pid == 0){
			//in child process
			execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
			//if execvp returns, that means there is an error
			perror("command not found");
			exit(1);
		}
	}
	dup2(default_in, 0);
	dup2(default_out, 1);
	dup2(default_err, 2);
	close(default_in);
	close(default_out);
	close(default_err);

	//wait for last command
	if(!_background){
		waitpid(pid, NULL,0);
	}



	// Clear to prepare for next command
	clear();

	// Print new prompt
	//int tmp_in = dup(0);
	prompt();
	return;
//}
}

// Shell implementation
int needCat = 1;
void
Command::prompt()
{
		if(isatty(0)){
			//printf("Isatty!!!\n");

			if(setPrompt == 1){
				char * p = (char*)calloc(512, sizeof(char));
				p = getenv("PROMPT");
				if(needCat){
					strcat(p, ">");
					needCat = 0;
				}
				printf(p);
				free(p);
			}else{
			printf("myshell>");
			}
			fflush(stdout);
		}
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

/*iextern "C"*/ void disp(int sig){
	//Command::_currentCommand.prompt();
	//Command::_currentCommand.clear();
	//Command::_currentCommand.prompt();
	fprintf(stderr,"\n");
}

void killZombie(int sig){
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

main()
{
	/*
	 *
	 *
	 * part : ctrl-C starts here*/
	struct sigaction sa;
	sa.sa_handler = disp;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	/*if(sigaction(SIGINT, &sa, NULL)){
		perror("sigaction");
		exit(1);
	}*/
	signal(SIGINT, disp);
	/*if(sigaction(SIGCHLD, &sa, NULL)){
		perror("sigaction");
		exit(-1);
	}*/
	struct sigaction kz;
	kz.sa_handler = killZombie;
	sigemptyset(&kz.sa_mask);
	kz.sa_flags = 0;
	signal(SIGCHLD, killZombie);
	/*if(sigaction(SIGCHLD, &sa, NULL)){
		perror("sigaction");
		exit(-1);
	}*/
	Command::_currentCommand.clear();
	Command::_currentCommand.prompt();

	yyparse();
}
