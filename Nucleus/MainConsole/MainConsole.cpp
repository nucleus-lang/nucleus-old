#include "MainConsole.h"
#include "../CompileItself/CompileItself.h"
#include "../Language/Language.h"

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
	{
		Files::ClearConsole();
	}
	else if(cmd == "--compile-itself")
    {
    	CompileItself::Compile();
    }
    else if(cmd == "--exit")
    {
    	exit(0);
    }
    else if(cmd == "--run-test")
    {
    	std::cout << "Lexer Test...\n";
    	PrintArrows();

    	std::getline(std::cin, GetInput());

    	std::cout << "Input Result: " << GetInput() << std::endl;

    	std::vector<Language::Token> t = Language::Lexer::Start(GetInput());
    	Language::Lexer::TokenTesting(t);
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