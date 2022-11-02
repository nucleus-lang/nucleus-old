#include <iostream>
#include "Files/Files.h"
#include "String/StringAPI.h"

#include "CompileItself/CompileItself.h"
#include "MainConsole/MainConsole.h"

int main(int argc, char** argv)
{
    for (int i = 0; i < argc; i++)
    {
    	std::string commandLine = argv[i];
        MainConsole::RunCommand(commandLine, false);
    }

    if(argc < 2)
    {
    	MainConsole::Run();
   	}

	return 0;
}