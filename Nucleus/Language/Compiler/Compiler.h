#ifndef COMPILER_H
#define COMPILER_H

#include "../Language.h"
#include "../../String/StringAPI.h"
#include "../../Files/Files.h"

struct Compiler
{
	static std::string AddType(std::string& text, std::string t, bool& next);

	static void Compile(std::string url);
};

#endif