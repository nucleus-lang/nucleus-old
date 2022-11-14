#include "Compiler.h"

void Compiler::Compile(std::string url)
{
	std::cout << "Compiling to C++..." << std::endl;

	std::string finalCompilation;
	std::vector<int> t = Language::Lexer::tokenList;
	bool nextIsFnType = false;
	bool nextIsVarType = false;
	bool replaceDotCommaWithEnd = false;
	bool nextIsUseUrl = false;

	std::string finalUrlForDupCheck;

	finalCompilation += "#ifndef " + StringAPI::ReplaceAll(Files::GetFilenameFromDirectory(url.c_str()), ".", "_") + "_dup_check\n";
	finalCompilation += "#define " + StringAPI::ReplaceAll(Files::GetFilenameFromDirectory(url.c_str()), ".", "_") + "_dup_check\n";
	finalCompilation += "\n";

	for(int i = 0; i < t.size(); i++)
	{
		if(t[i] == Language::Token::Function)
			finalCompilation += "{func-type} ";
		else if(t[i] == Language::Token::Variable)
			finalCompilation += "{var-type} ";
		else if(t[i] == Language::Token::Import)
		{
			finalCompilation += "#include ";
			replaceDotCommaWithEnd = true;
			nextIsUseUrl = true;
		}
		else if(t[i] == Language::Token::Name)
		{
			if(i + 1 < t.size())
			{
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
		else if(t[i] == Language::Token::TwoDots)
			nextIsVarType = true;

		else if (t[i] == Language::Token::Integer)
		{
			if (nextIsFnType)
			{
				finalCompilation = StringAPI::ReplaceAll(finalCompilation, "{func-type}", "int");
				nextIsFnType = false;
			}
			else if(nextIsVarType)
			{
				finalCompilation = StringAPI::ReplaceAll(finalCompilation, "{var-type}", "int");
				nextIsVarType = false;
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
			else if(nextIsVarType)
			{
				finalCompilation = StringAPI::ReplaceAll(finalCompilation, "{var-type}", "float");
				nextIsVarType = false;
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
			else if(nextIsVarType)
			{
				finalCompilation = StringAPI::ReplaceAll(finalCompilation, "{var-type}", "string");
				nextIsVarType = false;
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
			else if(nextIsVarType)
			{
				finalCompilation = StringAPI::ReplaceAll(finalCompilation, "{var-type}", "char");
				nextIsVarType = false;
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
				if(Language::Lexer::allNames[t[i + 1]].first == Language::GetNameFor::StringCnt)
				{
					if(!nextIsUseUrl)
					{
						finalCompilation += Language::Lexer::allNames[t[i + 1]].second;
						i++;
					}
					else
					{
						std::string finalUrl = Language::Lexer::allNames[t[i + 1]].second + ".h";
						finalCompilation += finalUrl;
						nextIsUseUrl = false;
						i++;
					}
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
		{
			if(replaceDotCommaWithEnd)
			{
				finalCompilation += "\n";
				replaceDotCommaWithEnd = false;
			}
			else
				finalCompilation += "; ";
		}

		else if(t[i] == Language::Token::LeftBracket)
		{
			if(nextIsFnType)
			{
				finalCompilation = StringAPI::ReplaceAll(finalCompilation, "{func-type}", "void");
				nextIsFnType = false;
			}

			finalCompilation += " { ";
		}
		else if(t[i] == Language::Token::RightBracket)
			finalCompilation += " } ";

		else if(t[i] == Language::Token::Return)
			finalCompilation += "return ";

		else if(t[i] == Language::Token::Add)
			finalCompilation += "+";
		else if(t[i] == Language::Token::Subtract)
			finalCompilation += "-";
		else if(t[i] == Language::Token::Multiply)
			finalCompilation += "*";
		else if(t[i] == Language::Token::Divide)
			finalCompilation += "/";
		else if(t[i] == Language::Token::Equal)
			finalCompilation += "=";
	}

	finalCompilation += "\n";
	finalCompilation += "#endif\n";

	std::cout << "Writing File..." << std::endl;

	std::string cacheDir = Files::GetDirectoryOf(Files::GetExecutablePath().c_str()) + "/cache";

	if(!Files::DirectoryExists(cacheDir.c_str()))
		Files::CreateDirectory(cacheDir.c_str());

	std::string finalUrl = cacheDir + "/" + Files::GetFilenameFromDirectory(url.c_str()) + ".cpp";

	Files::Write(finalUrl.c_str(), finalCompilation);
}