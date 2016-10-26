
#ifndef command_h
#define command_h

// Command Data Structure
struct SimpleCommand {
	// Available space for arguments currently preallocated
	int _numOfAvailableArguments;

	// Number of arguments
	int _numOfArguments;
	char ** _arguments;

	SimpleCommand();
	void insertArgument( char * argument );
	void wildcardExpansion(char * prefix, char * suffix, int depth);
//	int compareFunction(const char * one, const char * two);
};

struct Command {
	int _numOfAvailableSimpleCommands;
	int _numOfSimpleCommands;
	SimpleCommand ** _simpleCommands;
	char * _outFile;
	char * _inFile;
	char * _errFile;
	int _background;
	int _append;

	void prompt();
	void print();
	void execute();
	void clear();
	//void IO_redirection(int redir_type, char * file_name);

	Command();
	void insertSimpleCommand( SimpleCommand * simpleCommand );
	//void wildcardExpansion(char * arg);

	static Command _currentCommand;
	static SimpleCommand *_currentSimpleCommand;
};

#endif
