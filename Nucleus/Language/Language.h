#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <iostream>
#include <string>
#include <vector>
#include <ctype.h>
#include "../String/StringAPI.h"

struct Language
{
	enum Token
	{
		EndOfFile = -1,

		Function = 1,
		Import = 2,

		Identifier = 3,
		Number = 4,

		Add = 5,
		Subtract = 6,
		Multiply = 7,
		Divide = 8,

		LeftParenthesis = 9,
		RightParenthesis = 10,

		Integer = 11,
		Float = 12,
	};

	struct Position
	{
		static int line;
		static int column;
		static int idx;
	};

	struct Error
	{
		static void PrintErrorAndExit(std::string message, int line, int column);
		static void IllegalCharacter(char c, int line, int column);
		static void IllegalWord(std::string s, int line, int column);
	};

	struct Lexer
	{
		static int position;
		static char currentChar;
		static std::string mainText;
		static std::vector<Language::Token> tokenList;
		static bool addName;

		static std::vector<Language::Token> Start(std::string text);

		static Token NumberTreatment();

		static void Step();

		static void TokenTreatment();

		static void GetWords();

		static void TokenTesting(std::vector<Language::Token> t);
	};
};

#endif