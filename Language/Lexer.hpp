#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include <iostream>

struct Lexer
{
	enum Token
	{
		TK_EOF = -1,

		TK_Function = -2,
		TK_Extern = -3,

		TK_Identifier = -4,
		TK_Double = -5,
		TK_Comma = -6,

		TK_Plus = -7,
		TK_Minus = -8,
		TK_Mul = -9,
		TK_Div = -10,

		TK_LeftParent = -11,
		TK_RightParent = -12,

		TK_Integer = -13,
		TK_Float = -14,

		TK_Number = -15,

		TK_Equals = -16,

		TK_LeftBracket = -17,
		TK_RightBracket = -18,

		TK_Arrow = -19,

		TK_Variable = -20,
		TK_TwoDots = -21,

		TK_Return = -22,

		TK_DotComma = -23,

		TK_SetType = -24,
	};

	static std::string Text;
	static unsigned int Position;
	static char CurrentChar;
	static std::vector<int> Tokens;
	static std::vector<std::string> GlobalStringInfo;

	static void Init(std::string input)
	{
		Text = input;
		Position = -1;
		CurrentChar = '\0';
		Advance();
	}

	static void Advance()
	{
		Position += 1;

		if(Position < Text.size())
			CurrentChar = Text[Position];
		else
			CurrentChar = '\0';
	}

	static bool CharIsNotFinished(char c)
	{
		return CurrentChar != '\0' 
		&& CurrentChar != ',' 
		&& CurrentChar != '\t' 
		&& CurrentChar != '\n' 
		&& CurrentChar != '\r' 
		&& CurrentChar != ' ' 
		&& CurrentChar != '(' 
		&& CurrentChar != ')'
		&& CurrentChar != ';';
	}

	static void GetWord()
	{
		std::string IdentifierStr;

		while(CharIsNotFinished(CurrentChar))
		{
			IdentifierStr += CurrentChar;
			Advance();
		}

		if(IdentifierStr == "func")
			Tokens.push_back(Token::TK_Function);
		else if(IdentifierStr == "extern")
			Tokens.push_back(Token::TK_Extern);
		else if(IdentifierStr == "int")
		{
			Tokens.push_back(Token::TK_SetType);
			Tokens.push_back(Token::TK_Integer);
		}
		else if(IdentifierStr == "double")
		{
			Tokens.push_back(Token::TK_SetType);
			Tokens.push_back(Token::TK_Double);
		}
		else if(IdentifierStr == "var")
			Tokens.push_back(Token::TK_Variable);
		else if(IdentifierStr == "return")
			Tokens.push_back(Token::TK_Return);
		else
		{
			Tokens.push_back(Token::TK_Identifier);
			Tokens.push_back(GlobalStringInfo.size());
			GlobalStringInfo.push_back(IdentifierStr);
		}
	}

	static void GetTokens()
	{
		while(CurrentChar != '\0' && Position < Text.size())
		{
			std::cout << CurrentChar << std::endl;

			if(CurrentChar == '\t' || CurrentChar == ' ' || CurrentChar == '\n' || CurrentChar == '\r')
				Advance();

			else if(CurrentChar == '+')
			{
				Tokens.push_back(Token::TK_Plus);
				Advance();
			}
			else if(CurrentChar == '-')
			{
				if(Text[Position + 1] == '>')
				{
					Tokens.push_back(Token::TK_Arrow);
					Advance();
				}
				else
					Tokens.push_back(Token::TK_Minus);

				Advance();
			}
			else if(CurrentChar == '*')
			{
				Tokens.push_back(Token::TK_Mul);
				Advance();
			}
			else if(CurrentChar == '/')
			{
				Tokens.push_back(Token::TK_Div);
				Advance();
			}

			else if(CurrentChar == '(')
			{
				Tokens.push_back(Token::TK_LeftParent);
				Advance();
			}
			else if(CurrentChar == ')')
			{
				Tokens.push_back(Token::TK_RightParent);
				Advance();
			}

			else if(CurrentChar == ',')
			{
				Tokens.push_back(Token::TK_Comma);
				Advance();
			}

			else if(CurrentChar == '=')
			{
				Tokens.push_back(Token::TK_Equals);
				Advance();
			}
			else if(CurrentChar == '{')
			{
				Tokens.push_back(Token::TK_LeftBracket);
				Advance();
			}
			else if(CurrentChar == '}')
			{
				Tokens.push_back(Token::TK_RightBracket);
				Advance();
			}
			else if(CurrentChar == ':')
			{
				Tokens.push_back(Token::TK_TwoDots);
				Advance();
			}
			else if(CurrentChar == ';')
			{
				Tokens.push_back(Token::TK_DotComma);
				Advance();
			}

			else if(isdigit(CurrentChar))
			{
				GetNumber();
			}

			else
			{
				GetWord();
			}
		}
	}

	static void GetNumber()
	{
		std::string NumStr;
		int dotCount = 0;

		while(CharIsNotFinished(CurrentChar))
		{
			if(CurrentChar == '.')
			{
				dotCount++;
				NumStr += '.';
			}
			else if(isdigit(CurrentChar))
			{
				NumStr += CurrentChar;
			}
			Advance();
		}

		Tokens.push_back(Token::TK_Number);
		Tokens.push_back(GlobalStringInfo.size());
		GlobalStringInfo.push_back(NumStr);
	}

	static std::string GetTokenAsString(int tok)
	{
		switch(tok)
		{
			case Token::TK_EOF:
				return "End Of File";

			case Token::TK_Function:
				return "Function";
			case Token::TK_Extern:
				return "Extern";

			case Token::TK_Identifier:
				return "Identifier";
			case Token::TK_Double:
				return "Double";
			case Token::TK_Comma:
				return "Comma";

			case Token::TK_Plus:
				return "Plus";
			case Token::TK_Minus:
				return "Minus";
			case Token::TK_Mul:
				return "Mul";
			case Token::TK_Div:
				return "Div";

			case Token::TK_LeftParent:
				return "Left Parenthesis";
			case Token::TK_RightParent:
				return "Right Parenthesis";

			case Token::TK_Integer:
				return "Integer";

			case Token::TK_Number:
				return "Number";

			case Token::TK_Equals:
				return "Equals";

			case Token::TK_LeftBracket:
				return "Left Bracket";
			case Token::TK_RightBracket:
				return "Right Bracket";

			case Token::TK_Arrow:
				return "-- Arrow -->";

			case Token::TK_Variable:
				return "Variable";

			case Token::TK_TwoDots:
				return ": Two Dots :";

			case Token::TK_Return:
				return "Return";

			case Token::TK_DotComma:
				return "; Dot Comma ;";

			case Token::TK_SetType:
				return "Set Type: ";

			default:
				return "Unknown";
		}
	}
};

#endif