
/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

%token 	NOTOKEN GREAT NEWLINE PIPE AMPERSAND GREATAMPERSAND GREATGREATAMPERSAND LESS GREATGREAT

%union	{
		char   *string_val;
	}

%{
//#define yylex yylex
#include <stdio.h>
#include <string.h>
#include <string>
#include "command.h"
//commands
//string io_redir = "redirection";
//string send_back = "send_to_background";
//string piping = "pipe_to";
void yyerror(const char * s);
int yylex();

%}

%%

goal:
	commands
	;

commands:
	command
	| commands command
	;

command: simple_command
	| simple_command PIPE command {
		Command::_currentCommand.execute();
	}
        ;

simple_command:
	command_and_args iomodifier_list NEWLINE {
		/*printf("   Yacc: Execute command\n");*/
		Command::_currentCommand.execute();
	}
	| command_and_args NEWLINE {
		/*printf("   Yacc: Execute command\n");*/
		Command::_currentCommand.execute();
	}
	| command_and_args {
		/*printf("   Yacc: Execute command with piping\n");*/
		/*Command::_currentCommand.execute();*/
	}
	| command_and_args background_opt NEWLINE {
		Command::_currentCommand.execute();
	}
	| NEWLINE {
	/*	Command::_currentCommand.clear();*/
		Command::_currentCommand.prompt();
	}
	/*| command_and_args SUBSHELLEXPRESSION NEWLINE {
		Command::_currentCommand.print();
		Command::_currentCommand.execute();
	}*/
	| error NEWLINE { yyerrok; }
	;

command_and_args:
	command_word argument_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

argument_list:
	argument_list argument
	| /* can be empty */
	;

argument:
	WORD {
              /* printf("   Yacc: insert argument \"%s\"\n", $1);*/

	       /*Command::_currentSimpleCommand->insertArgument( $1 );*/
	       if(!strchr($1, '*') && !strchr($1, '?'))
	       		Command::_currentSimpleCommand->insertArgument($1);
	       else{
		   		/*printf("-----------------------%s\n", $1);*/
		   		Command::_currentSimpleCommand->wildcardExpansion("",$1,0);
		   	}
	}
	/*| could be empty*/
	;

command_word:
	WORD {
              /* printf("   Yacc: insert command \"%s\"\n", $1);*/

	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	     /*  printf(Command::_currentSimpleCommand->_arguments[0]);
	       printf("\n");*/
	}
	;

iomodifier_opt:
	iomodifier_opt iomodifier_opt
	|
	GREAT WORD {
		/*printf("   Yacc: insert output \"%s\"\n", $2);*/
		if(Command::_currentCommand._outFile)
			printf("Ambiguous output redirect.\n");
		else{
			Command::_currentCommand._outFile = $2;
			/*Command::_currentCommand._append = 1;*/
		}
			
		/*char * file_name = strdup($2);*/
		/*Command::_currentCommand.IO_redirection(1,file_name);*/
		/*Command::_currentCommand.execute();*/
	}
	|LESS WORD {
		/*printf("   Yacc: insert input \"%s\"\n", $2);*/
		Command::_currentCommand._inFile = $2;
		/*Command::_currentCommand.execute();*/
	}
	|GREATAMPERSAND WORD {
		/*printf("   Yacc: redirecting out put to \"%s\"\n", $2);*/
	/*	Command::_currentCommand._background = 1;*/
		Command::_currentCommand._outFile = $2;
		char * str = strdup($2);
		Command::_currentCommand._errFile = str;
		/*Command::_currentCommand.execute();*/
	}
	|GREATGREATAMPERSAND WORD {
		/*printf("   Yacc: redirecting output to \"%s\"\n",$2);*/
	/*	Command::_currentCommand._background = 1;*/
		Command::_currentCommand._outFile = $2;
		char * str = strdup($2);
		Command::_currentCommand._errFile = str;
		Command::_currentCommand._append = 1;
		/*Command::_currentCommand.execute();*/
	}
	|
	GREATGREAT WORD {
		/*printf("	Yacc: redirecting output to \"%s\"\n",$2);*/
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._append = 1;
		/*Command::_currentCommand.execute();*/
	}
	;

iomodifier_list:
	iomodifier_opt iomodifier_list
	|/*empty*/
	;

background_opt:
	AMPERSAND {
		/*printf("   Yacc: starting process in background...\n");*/
		Command::_currentCommand._background = 1;
		/*printf("   Yacc: Execute Command\n");*/
	}
	;

pipe_opt:
	PIPE {
		/*printf("   Yacc: Piping, accepting input from previous command\n");*/
		/*Command::_currentCommand.execute();*/
	}
	|
	;
%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
