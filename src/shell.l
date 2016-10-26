
/*
 *
 * CS-252 Fall 2013
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "y.tab.h"

extern "C" char * read_line();

int mygetc(FILE * f) {
	static char * p;
	char ch;

	if(!isatty(0)){
		//stdin is not a tty. call real getc
		return getc(f);
	}

	//stdin is a tty. Call our read_line.

	if(p == NULL || *p == 0){
		char *s = read_line();
		p = s;
	}
	ch = *p;
	p++;

	return ch;
}
#undef getc
#define getc(f) mygetc(f)

static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
    unput(c);
}

%}

%%

\n 	{
		return NEWLINE;
	}

[ \t] 	{
		/* Discard spaces and tabs */
     	}

">" 	{
		return GREAT;
	}
"|"	{
		return PIPE;
	}
"&"	{
		return AMPERSAND;
	}
">&"	{
		return GREATAMPERSAND;
	}
">>&"	{
		return GREATGREATAMPERSAND;
	}
"<"	{
		return LESS;
	}
">>" {
		return GREATGREAT;
	 }

`[^\\\|\>\<\&\t\n`][^\\\|\>\<\&\t\n`]*` {
		char * ssExpr = (char*)calloc(512, sizeof(char));
		char * rssExpr = ssExpr;
		char * duplicate = strdup(yytext);
		duplicate++;
		while(*duplicate != '`'){
			*ssExpr = *duplicate;
			ssExpr++;
			duplicate++;
		}
		*ssExpr = '\0';
		/*fprintf(stderr, "subshell expression: %s\n", rssExpr);*/
		strcat(rssExpr, "\nexit\n\0");
		/*yylval.string_val = strdup(rssExpr);*/
		char * buffer = (char*)calloc(1024, sizeof(char));

		int d_in = dup(0);
		int d_out = dup(1);

		int pipe_to_ss[2];
		pipe(pipe_to_ss);
		dup2(pipe_to_ss[0], 0);
		close(pipe_to_ss[0]);

		int pid;
		pid = fork();

		int pipe_from_ss[2];
		pipe(pipe_from_ss);
		dup2(pipe_from_ss[1],1);
		close(pipe_from_ss[1]);

		write(1, rssExpr, strlen(rssExpr));

		if(pid == 0){
			dup2(pipe_from_ss[0], 0);
			close(pipe_from_ss[0]);

			dup2(pipe_to_ss[1], 1);
			close(pipe_to_ss[1]);

			char * sscmd[2];
			sscmd[0] = getenv("_");
			sscmd[1] = '\0';
			execvp(sscmd[0], sscmd);
			perror("ssexecvp");
			exit(1);
		}
		else{
			dup2(d_out, 1);
			close(d_out);
			close(pipe_from_ss[1]);
			close(pipe_to_ss[0]);
			close(pipe_to_ss[1]);

			char res = 0;
			int counter = 0;
			while(read(0, &res, 1) != 0){
				buffer[counter] = res;
				counter++;
			}
			buffer[counter] = '\0';
			dup2(d_in, 0);
			close(d_in);
		}
		waitpid(pid, NULL,0);
		for(int i = strlen(buffer) - 1; i >=0 ;i--){
			if(buffer[i] == '\n')
				buffer[i] = ' ';
			unput(buffer[i]);
		}
		
	}

[^\\\|\>\<\& \t\n`][^\\\|\>\<\& \t\n`]*    {
		yylval.string_val = strdup(yytext);
		/*fprintf(stderr, "dadfafadfadf is: %s\n", yytext);*/
		return WORD;
	}
\"[^\n\"]*\" {
		char * token = strdup(yytext);
		char * leftQuote = strchr(token, '\"');
		char * realWord = (char*)calloc(1024, sizeof(char));
		int counter = 0;
		leftQuote = leftQuote + 1;
		while(*leftQuote != '\"'){
			realWord[counter] = *leftQuote;
			counter++;
			leftQuote++;
		}
		realWord[counter] = '\0';
		yylval.string_val = strdup(realWord);
		return WORD;
}
[^ \t\n]*[\\]+[^ \t\n]* {
		char * token = strdup(yytext);
		char * realWord = (char*)calloc(1024, sizeof(char));
		int counter = 0;
		while(*token != '\0'){
			if(*token == '\\'){
				token++;
				realWord[counter] = *token;
				token++;
				counter++;
			}
			else{
				realWord[counter] = *token;
				counter++;
				token++;
			}
		}
		realWord[counter] = '\0';
		yylval.string_val = strdup(realWord);
		return WORD;
}
%%

/*int main(void){
	//call lex
	yylex();
	return 0;
}*/
