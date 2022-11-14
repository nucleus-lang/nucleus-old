#include "Language.h"
#include <experimental/filesystem>

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
std::vector<int> Language::Lexer::tokenList;
Language::GetNameFor Language::Lexer::getNameFor;
std::vector<std::pair<Language::GetNameFor, std::string>> Language::Lexer::allNames;
bool Language::Lexer::isStringOpen = false;
std::vector<std::string> Language::Lexer::allNumbers;
std::vector<std::string> Language::Lexer::allFiles;

int Language::Position::line;
int Language::Position::column;
int Language::Position::idx;

void Language::Lexer::FindFiles()
{
	for(auto& p: std::experimental::filesystem::recursive_directory_iterator(std::experimental::filesystem::current_path()))
	{
		std::string cacheFolder = Files::GetDirectoryOf(Files::GetExecutablePath().c_str()) + "/cache";
		std::string pathToStr = p.path().u8string();

       	if(pathToStr.find(".nk") != std::string::npos && pathToStr.find(".nk.cpp") == std::string::npos)
       	{
       		std::cout << Files::GetFilenameFromDirectory(pathToStr.c_str()) << std::endl;
       		Language::Lexer::allFiles.push_back(pathToStr);
       	}
	}
}

std::vector<int> Language::Lexer::Start(std::string text)
{
	Language::Lexer::tokenList.clear();

	mainText = text;

	position = -1;
	Language::Position::line = 1;
	Language::Position::idx = 0;
	Language::Position::column = 0;

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
			allNumbers.push_back(numberStr);
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
		currentChar != '(' &&
		currentChar != ')' &&
		currentChar != ';' &&
		currentChar != '{' &&
		currentChar != '}' &&
		currentChar != ';' &&
		currentChar != ':')
	{
		if(currentChar == '"')
		{
			if(position - 1 > -1)
				if(mainText[position - 1] != '\\')
					break;
		}

		if(currentChar == ' ')
			if(!isStringOpen)
				break;

		word += currentChar;
		Step();
	}

	if(word == "int")
	{
		tokenList.push_back(Language::Token::Integer);
		Lexer::getNameFor = Language::GetNameFor::VariableName;
	}
	else if(word == "float")
	{
		tokenList.push_back(Language::Token::Float);
		Lexer::getNameFor = Language::GetNameFor::VariableName;
	}
	else if(word == "string")
	{
		tokenList.push_back(Language::Token::String);
		Lexer::getNameFor = Language::GetNameFor::VariableName;
	}
	else if(word == "char")
	{
		tokenList.push_back(Language::Token::Char);
		Lexer::getNameFor = Language::GetNameFor::VariableName;
	}
	else if(word == "func")
	{
		tokenList.push_back(Language::Token::Function);
		Lexer::getNameFor = Language::GetNameFor::FunctionName;
	}
	else if(word == "var")
	{
		tokenList.push_back(Language::Token::Variable);
		Lexer::getNameFor = Language::GetNameFor::FunctionName;
	}
	else if(word == "return")
	{
		tokenList.push_back(Language::Token::Return);
	}
	else if(word == "use")
	{
		tokenList.push_back(Language::Token::Import);
	}
	else if(Lexer::getNameFor != Language::GetNameFor::Disabled)
	{
		if(!isStringOpen)
		{
			bool nameExists = false;
			for(auto i : Lexer::allNames)
			{
				if(i.second == word)
				{
					nameExists = true;
					break;
				}
			}

			if(!nameExists)
				Lexer::allNames.push_back(std::make_pair(Lexer::getNameFor, word));

			Lexer::getNameFor = Language::GetNameFor::Disabled;

			tokenList.push_back(Language::Token::Name);
			tokenList.push_back(Lexer::allNames.size() - 1);
		}
		else
		{
			Lexer::allNames.push_back(std::make_pair(Language::GetNameFor::StringCnt, word));

			tokenList.push_back(Language::Token::StringContent);
			tokenList.push_back(Lexer::allNames.size() - 1);

			Lexer::getNameFor = Language::GetNameFor::Disabled;
		}
	}
	else
	{
		bool nameExists = false;
		for(int i = 0; i < Lexer::allNames.size(); i++)
		{
			if(Lexer::allNames[i].second == word)
			{
				nameExists = true;
				tokenList.push_back(Language::Token::Name);
				tokenList.push_back(i);
				break;
			}
		}

		if(!nameExists)
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
			if(position + 1 < mainText.size())
			{
				if(mainText[position + 1] == '>')
				{
					tokenList.push_back(Token::Arrow);
					Step();
				}
				else
					tokenList.push_back(Token::Subtract);
			}
			else
				tokenList.push_back(Token::Subtract);

			Step();
		}
		else if(currentChar == ':')
		{
			tokenList.push_back(Token::TwoDots);
			Lexer::getNameFor = Language::GetNameFor::Disabled;
			Step();
		}
		else if(currentChar == '=')
		{
			tokenList.push_back(Token::Equal);
			Lexer::getNameFor = Language::GetNameFor::Disabled;
			Step();
		}
		else if(currentChar == '*')
		{
			tokenList.push_back(Token::Multiply);
			Lexer::getNameFor = Language::GetNameFor::Disabled;
			Step();
		}
		else if(currentChar == '/')
		{
			tokenList.push_back(Token::Divide);
			Lexer::getNameFor = Language::GetNameFor::Disabled;
			Step();
		}
		else if(currentChar == '(')
		{
			tokenList.push_back(Token::LeftParenthesis);
			Lexer::getNameFor = Language::GetNameFor::Disabled;
			Step();
		}
		else if(currentChar == ')')
		{
			tokenList.push_back(Token::RightParenthesis);
			Lexer::getNameFor = Language::GetNameFor::Disabled;
			Step();
		}
		else if(currentChar == '{')
		{
			tokenList.push_back(Token::LeftBracket);
			Lexer::getNameFor = Language::GetNameFor::Disabled;
			Step();
		}
		else if(currentChar == '}')
		{
			tokenList.push_back(Token::RightBracket);
			Lexer::getNameFor = Language::GetNameFor::Disabled;
			Step();
		}
		else if(currentChar == ';')
		{
			tokenList.push_back(Token::DotComma);
			Lexer::getNameFor = Language::GetNameFor::Disabled;
			Step();
		}
		else if(currentChar == '"')
		{
			if(!isStringOpen)
				tokenList.push_back(Token::OpenString);
			else
				tokenList.push_back(Token::CloseString);

			isStringOpen = !isStringOpen;
			Lexer::getNameFor = Language::GetNameFor::StringCnt;

			Step();
		}
		else if(std::isdigit(currentChar))
		{
			tokenList.push_back(Lexer::NumberTreatment());
			tokenList.push_back(allNumbers.size() - 1);
			Lexer::getNameFor = Language::GetNameFor::Disabled;
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

void Language::Lexer::TokenTesting(std::vector<int> t)
{
	/*
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

		Arrow = 13,
		Name = 14,

		LeftBracket = 15,
		RightBracket = 16,
		Return = 17,
		DotComma = 18,

		String = 19,
		Char = 20,

		OpenString = 21,
		CloseString = 22,

		StringContent = 23,

		Variable = 24,
	*/

	for(int i = 0; i < t.size(); i++)
    {
    	if(t[i] == Language::Token::Function)
    		std::cout << "Function\n";
    	else if(t[i] == Language::Token::Import)
    		std::cout << "Import\n";

    	else if(t[i] == Language::Token::Identifier)
    		std::cout << "Identifier\n";
    	else if(t[i] == Language::Token::Number)
    	{
    		std::cout << "Number (";
    		if(i + 1 < t.size())
    		{
    			std::cout << Lexer::allNumbers[int(t[i + 1])] << ")\n";
    			i++;
    		}
    	}

    	else if(t[i] == Language::Token::Add)
    		std::cout << "Add\n";
    	else if(t[i] == Language::Token::Subtract)
    		std::cout << "Subtract\n";
    	else if(t[i] == Language::Token::Multiply)
    		std::cout << "Multiply\n";
    	else if(t[i] == Language::Token::Divide)
    		std::cout << "Divide\n";

    	else if(t[i] == Language::Token::LeftParenthesis)
    		std::cout << "Left Parenthesis\n";
    	else if(t[i] == Language::Token::RightParenthesis)
    		std::cout << "Right Parenthesis\n";

    	else if(t[i] == Language::Token::Integer)
    		std::cout << "Integer\n";
    	else if(t[i] == Language::Token::Float)
    		std::cout << "Float\n";

    	else if(t[i] == Language::Token::Arrow)
    		std::cout << "Arrow\n";

    	else if(t[i] == Language::Token::Name)
    	{
    		std::cout << "Name (";
    		if(i + 1 < t.size())
    		{
    			std::cout << Lexer::allNames[int(t[i + 1])].second << ")\n";
    			i++;
    		}
    	}

    	else if(t[i] == Language::Token::LeftBracket)
    		std::cout << "Left Bracket\n";
    	else if(t[i] == Language::Token::RightBracket)
    		std::cout << "Right Bracket\n";

    	else if(t[i] == Language::Token::Return)
    		std::cout << "Return\n";

    	else if(t[i] == Language::Token::DotComma)
    		std::cout << "Dot Comma\n";

    	else if(t[i] == Language::Token::String)
    		std::cout << "String\n";

    	else if(t[i] == Language::Token::Char)
    		std::cout << "Char\n";

    	else if(t[i] == Language::Token::OpenString)
    		std::cout << "Open String\n";

    	else if(t[i] == Language::Token::CloseString)
    		std::cout << "Close String\n";

    	else if(t[i] == Language::Token::StringContent)
    	{
    		std::cout << "String Content (";
    		if(i + 1 < t.size())
    		{
    			std::cout << Lexer::allNames[int(t[i + 1])].second << ")\n";
    			i++;
    		}
    	}

    	else if(t[i] == Language::Token::Variable)
    		std::cout << "Variable\n";
    }
}