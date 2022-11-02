#ifndef MAIN_CONSOLE_H
#define MAIN_CONSOLE_H

#include "../Files/Files.h"
#include "../String/StringAPI.h"

struct MainConsole
{
	static std::string _input;

	static void Run();

	static std::string& GetInput();

	static void PrintArrows();
	static void PrintWelcomeScreen();

	static void SpawnInputOnly();

	static void RunCommand(std::string cmd, bool spawnInput);
};

#endif