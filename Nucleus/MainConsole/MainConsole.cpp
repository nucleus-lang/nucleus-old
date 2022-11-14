#include "MainConsole.h"
#include "../CompileItself/CompileItself.h"
#include "../Language/Language.h"
#include "../Language/Compiler/Compiler.h"
#include "../Files/Files.h"

std::string MainConsole::_input = "";

void MainConsole::Run()
{
	PrintWelcomeScreen();
	PrintArrows();

	std::getline(std::cin, GetInput());

	RunCommand(GetInput(), true);
}

void MainConsole::RunCommand(std::string cmd, bool spawnInput)
{
    if(cmd == "--clear")
	Files::ClearConsole();
 
    else if(cmd == "--compile-itself")
    	CompileItself::Compile();
  
    else if(cmd == "--exit")
    	exit(0);

    else if(cmd == "--run-test")
    {
    	std::cout << "Finding Files Test...\n";
    	Language::Lexer::FindFiles();

    	std::reverse(Language::Lexer::allFiles.begin(), Language::Lexer::allFiles.end());

    	for(auto i : Language::Lexer::allFiles)
    	{
    		std::cout << "Lexer Test From " << Files::GetFilenameFromDirectory(i.c_str()) << " ...\n ====== \n";

    		std::vector<int> t = Language::Lexer::Start(Files::Read(i.c_str()));
    		Language::Lexer::TokenTesting(t);

    		std::cout << " VVVVVV \n";

    		Compiler::Compile(i);

    		std::cout << " ====== \n";
    	}
    }

    if(spawnInput)
	SpawnInputOnly();
}

void MainConsole::SpawnInputOnly()
{
	PrintArrows();

	std::getline(std::cin, GetInput());

	RunCommand(GetInput(), true);
}

void MainConsole::PrintArrows()
{
	std::cout << ">> ";
}

void MainConsole::PrintWelcomeScreen()
{
	std::cout << "Welcome to Nucleus v0.1!" << std::endl;
}

std::string& MainConsole::GetInput()
{
	return _input;
}
