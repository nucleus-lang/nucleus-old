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

void Language::Error::IllegalWord(std::string s, int line, int column)
{
	std::string m = "The word \"" + s + "\" is unknown!";
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

void Language::Lexer::GetWords()
{
	std::string word;

	while(currentChar != '\0' && 
		currentChar != '\t' && 
		currentChar != '\n' && 
		currentChar != '\r' && 
		currentChar != ' ' &&
		currentChar != '(' &&
		currentChar != ')' &&
		currentChar != ';')
	{
		word += currentChar;
		Step();
	}

	if(word == "int")
		tokenList.push_back(Language::Token::Integer);
	else if(word == "float")
		tokenList.push_back(Language::Token::Float);
	else
	{
		Language::Error::IllegalWord(word, Language::Position::line, Language::Position::column);
	}

	position -= 1;
	Language::Position::idx -= 1;
	Language::Position::column -= 1;

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
		{
			Lexer::GetWords();
			Step();
		}

		TokenTreatment();
	}
}

void Language::Lexer::TokenTesting(std::vector<Language::Token> t)
{
	/*
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
	*/

	for(auto i : t)
    {
    	if(i == Language::Token::Function)
    		std::cout << "Function\n";
    	else if(i == Language::Token::Import)
    		std::cout << "Import\n";

    	else if(i == Language::Token::Identifier)
    		std::cout << "Identifier\n";
    	else if(i == Language::Token::Number)
    		std::cout << "Number\n";

    	else if(i == Language::Token::Add)
    		std::cout << "Add\n";
    	else if(i == Language::Token::Subtract)
    		std::cout << "Subtract\n";
    	else if(i == Language::Token::Multiply)
    		std::cout << "Multiply\n";
    	else if(i == Language::Token::Divide)
    		std::cout << "Divide\n";

    	else if(i == Language::Token::LeftParenthesis)
    		std::cout << "Left Parenthesis\n";
    	else if(i == Language::Token::RightParenthesis)
    		std::cout << "Right Parenthesis\n";

    	else if(i == Language::Token::Integer)
    		std::cout << "Integer\n";
    	else if(i == Language::Token::Float)
    		std::cout << "Float\n";
    }
}