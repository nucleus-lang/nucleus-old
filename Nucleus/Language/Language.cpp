#include "Language.h"

void Language::Error::PrintErrorAndExit(std::string message, int line, int column)
{
	std::cout << "[ERROR] at (" << line << ", " << column << ") >> " << message << std::endl;
	exit(1);
}

void Language::Error::IllegalCharacter(const char c, int line, int column)
{
	std::string m = "The character \"" + std::string(1, c) + "\" can't be used in there!";
	PrintErrorAndExit(m, line, column);
}

int Language::Lexer::position = -1;
char Language::Lexer::currentChar = '\0';
std::string Language::Lexer::mainText;
std::vector<Language::Token> Language::Lexer::tokenList;

int Language::Position::line;
int Language::Position::column;
int Language::Position::idx;

std::vector<Language::Token> Language::Lexer::Start(std::string text)
{
	mainText = text;

	Language::Position::line = 1;

	Step();
	TokenTreatment();

	return tokenList;
}

Language::Token Language::Lexer::NumberTreatment()
{
	std::string numberStr;
	int dotCount = 0;
	
	bool charCheck = std::isdigit(currentChar) || currentChar == '.';
	
	while(true)
	{
		if(currentChar == '.')
		{
			if(dotCount == 1)
				break;
	
			dotCount++;
			numberStr += '.';

			Step();
		}
		else if(std::isdigit(currentChar))
		{
			numberStr += currentChar;

			Step();
		}
		else
		{
			std::cout << "(" << numberStr << ")\n";
			position -= 1;
			Language::Position::idx -= 1;
			Language::Position::column -= 1;
			break;
		}
	}
	
	return Language::Token::Number;
}

void Language::Lexer::Step()
{
	position += 1;

	if(position < mainText.size())
	{
		currentChar = mainText[position];

		Language::Position::idx += 1;
		Language::Position::column += 1;

		if(currentChar == '\n')
		{
			Language::Position::line += 1;
			Language::Position::column = 0;
		}
	}
	else
		currentChar = '\0';
}

void Language::Lexer::TokenTreatment()
{
	while(currentChar != '\0')
	{
		if(currentChar == '\t' || currentChar == '\n' || currentChar == '\r' || currentChar == ' ')
		{
			Step();
		}
		else if(currentChar == '+')
		{
			tokenList.push_back(Token::Add);
			Step();
		}
		else if(currentChar == '-')
		{
			tokenList.push_back(Token::Subtract);
			Step();
		}
		else if(currentChar == '*')
		{
			tokenList.push_back(Token::Multiply);
			Step();
		}
		else if(currentChar == '/')
		{
			tokenList.push_back(Token::Divide);
			Step();
		}
		else if(currentChar == '(')
		{
			tokenList.push_back(Token::LeftParenthesis);
			Step();
		}
		else if(currentChar == ')')
		{
			tokenList.push_back(Token::RightParenthesis);
			Step();
		}
		else if(std::isdigit(currentChar))
		{
			tokenList.push_back(Lexer::NumberTreatment());
			Step();
		}
		else
			Language::Error::IllegalCharacter(currentChar, Language::Position::line, Language::Position::column);

		TokenTreatment();
	}
}