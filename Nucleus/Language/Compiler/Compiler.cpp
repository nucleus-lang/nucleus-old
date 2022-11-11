#include "Compiler.h"

void Compiler::Compile()
{
	std::cout << "Compiling to C++..." << std::endl;

	std::string finalCompilation;
	std::vector<int> t = Language::Lexer::tokenList;
	bool nextIsFnType = false;
	for(int i = 0; i < t.size(); i++)
	{
		if(t[i] == Language::Token::Function)
			finalCompilation += "{func-type} ";
		else if(t[i] == Language::Token::Name)
		{
			if(i + 1 < t.size())
			{
				std::cout << "t[i + 1] is " << t[i + 1] << std::endl;
				finalCompilation += Language::Lexer::allNames[t[i + 1]].second;
				i++;
			}
		}
		else if(t[i] == Language::Token::LeftParenthesis)
			finalCompilation += "(";
		else if(t[i] == Language::Token::RightParenthesis)
			finalCompilation += ")";
		else if(t[i] == Language::Token::Arrow)
			nextIsFnType = true;

		else if (t[i] == Language::Token::Integer)
		{
			if (nextIsFnType)
			{
				finalCompilation = StringAPI::ReplaceAll(finalCompilation, "{func-type}", "int");
				nextIsFnType = false;
			}
			else
			{
				finalCompilation += "int ";
			}
		}
		else if(t[i] == Language::Token::Float)
		{
			if (nextIsFnType)
			{
				finalCompilation = StringAPI::ReplaceAll(finalCompilation, "{func-type}", "float");
				nextIsFnType = false;
			}
			else
			{
				finalCompilation += "float ";
			}
		}
		else if(t[i] == Language::Token::String)
		{
			if (nextIsFnType)
			{
				finalCompilation = StringAPI::ReplaceAll(finalCompilation, "{func-type}", "string");
				nextIsFnType = false;
			}
			else
			{
				finalCompilation += "string ";
			}
		}
		else if(t[i] == Language::Token::Char)
		{
			if (nextIsFnType)
			{
				finalCompilation = StringAPI::ReplaceAll(finalCompilation, "{func-type}", "char");
				nextIsFnType = false;
			}
			else
			{
				finalCompilation += "char ";
			}
		}

		else if(t[i] == Language::Token::OpenString || t[i] == Language::Token::CloseString)
			finalCompilation += "\"";
		else if(t[i] == Language::Token::StringContent)
		{
			std::cout << "Adding String Content..." << std::endl;
			if(i + 1 < t.size())
			{
				std::cout << "t[i + 1] is " << t[i + 1] << std::endl;
				if(Language::Lexer::allNames[t[i + 1]].first == Language::GetNameFor::StringCnt)
				{
					finalCompilation += Language::Lexer::allNames[t[i + 1]].second;
					i++;
				}
			}
			std::cout << "String Content Added!" << std::endl;
		}

		else if(t[i] == Language::Token::Number)
		{
			if(i + 1 < t.size())
			{
				std::cout << "t[i + 1] is " << t[i + 1] << std::endl;
				finalCompilation += Language::Lexer::allNumbers[t[i + 1]];
				i++;
			}
		}

		else if(t[i] == Language::Token::DotComma)
			finalCompilation += ";";

		else if(t[i] == Language::Token::LeftBracket)
			finalCompilation += "{";
		else if(t[i] == Language::Token::RightBracket)
			finalCompilation += "}";

		else if(t[i] == Language::Token::Return)
			finalCompilation += "return ";
	}

	std::cout << "Writing File..." << std::endl;
	Files::Write("main.nk.cpp", finalCompilation);
}