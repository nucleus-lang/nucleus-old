#ifndef PARSER_HPP
#define PARSER_HPP

#include "AbstractSyntaxTree.hpp"
#include "Lexer.hpp"
#include <vector>

struct Parser
{
	static int CurrentToken;

	static std::unique_ptr<AST::Expression> LogError(std::string str)
	{
		std::cout << "Error: " << str << "\n";
		exit(1);
		return nullptr;
	}

	static std::unique_ptr<AST::Prototype> LogErrorP(std::string str)
	{
		std::cout << "Error: " << str << "\n";
		exit(1);
		return nullptr;
	}

	static std::unique_ptr<AST::Function> LogErrorF(std::string str)
	{
		std::cout << "Error: " << str << "\n";
		exit(1);
		return nullptr;
	}

	static void Start()
	{
		CurrentToken = 0;

		std::cout << "Starting Parser...\n";

		// After finishing the conversion, you parse it to create the Abstract Syntax Tree.

		// Here's where the program starts:

		while(CurrentToken < Lexer::Tokens.size())
		{
			if(Lexer::Tokens[CurrentToken] == Lexer::Token::TK_Function)
			{
				ParseFunction();
			}

			CurrentToken++;
		}
	}

	static std::unique_ptr<AST::Function> ParseFunction()
	{
		CurrentToken++;

		std::string FunctionName;

		// First, we get the name of the function:

		if(Lexer::Tokens[CurrentToken] == Lexer::Token::TK_Identifier)
		{
			CurrentToken++;

			if(Lexer::Tokens[CurrentToken] < 0)
			{
				// If the token is smaller than 0, we throw an error because its not
				// something part of the "Global String Info" vector/array.

				LogErrorF("Unknown identifier.");
			}
			else
			{
				FunctionName = Lexer::GlobalStringInfo[Lexer::Tokens[CurrentToken]];
				std::cout << "Function Name: " << FunctionName << "\n";
			}
		}
		else
		{
			LogErrorF("Identifier not found.");
		}

		// Here is where we find the prototype first:

		// The Prototype is basically the contents that are inside the parenthesis of the Function.
		// func main --> (int a, int b, ...) <--

		auto Proto = ParsePrototype(FunctionName);

		return nullptr;
	}

	static bool ProtoWithParams()
	{
		return Lexer::Tokens[CurrentToken] == Lexer::Token::TK_Identifier 
		|| Lexer::Tokens[CurrentToken] == Lexer::Token::TK_SetType
		|| Lexer::Tokens[CurrentToken] == Lexer::Token::TK_Integer
		|| Lexer::Tokens[CurrentToken] == Lexer::Token::TK_Double
		|| Lexer::Tokens[CurrentToken] == Lexer::Token::TK_Comma;
	}

	static std::unique_ptr<AST::Expression> ParsePrototypeVariable()
	{
		if(Lexer::Tokens[CurrentToken] != Lexer::Token::TK_SetType)
			return LogError("No set type found inside prototype.");

		CurrentToken++;

		std::unique_ptr<AST::Type> grabType;

		if(Lexer::Tokens[CurrentToken] == Lexer::Token::TK_Integer)
			grabType = std::make_unique<AST::Integer>(0);
		else if(Lexer::Tokens[CurrentToken] == Lexer::Token::TK_Double)
			grabType = std::make_unique<AST::Double>(0);
		else
			return LogError("Unknown type found inside prototype.");

		CurrentToken++;

		if(Lexer::Tokens[CurrentToken] != Lexer::Token::TK_Identifier)
			return LogError("No identifier found inside prototype");

		CurrentToken++;

		if(Lexer::Tokens[CurrentToken] < 0)
			return LogError("Unknown identifier found inside prototype");

		std::string idName = Lexer::GlobalStringInfo[Lexer::Tokens[CurrentToken]];

		CurrentToken++;

		if(Lexer::Tokens[CurrentToken] == Lexer::Token::TK_Comma || Lexer::Tokens[CurrentToken] == Lexer::Token::TK_RightParent)
		{
			std::cout << "Var name: " << idName << "\n";
			return std::make_unique<AST::Variable>(std::move(grabType), idName);
		}
		else
			return LogError("Expected ',' or ')' at end of variable or prototype.");
	}

	static std::unique_ptr<AST::Prototype> ParsePrototype(std::string fnName)
	{
		CurrentToken++;

		std::vector<std::unique_ptr<AST::Expression>> variables;

		if(Lexer::Tokens[CurrentToken] != Lexer::Token::TK_LeftParent)
			LogErrorP("Expected '(' at the start of the prototype.");

		CurrentToken++;

		while(ProtoWithParams())
		{
			variables.push_back(ParsePrototypeVariable());
			CurrentToken++;
		}

		CurrentToken++;

		if(Lexer::Tokens[CurrentToken] != Lexer::Token::TK_LeftBracket)
			return LogErrorP("Expected '{' at start of function.");

		CurrentToken++;

		return nullptr;
	}
};

#endif