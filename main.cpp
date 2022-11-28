#include "Language/Lexer.hpp"
#include "Language/Parser.hpp"
#include <string>
#include <fstream>
#include <sstream>

std::string FileToString(std::string url)
{
	std::ifstream t(url.c_str());
	std::stringstream buffer;
	buffer << t.rdbuf();
	return buffer.str();
}

int main(int argc, char const *argv[])
{
	std::string text = FileToString("main.nk");

	Lexer::Init(text);
	Lexer::GetTokens();

	std::cout << "[";

	for(int i = 0; i < Lexer::Tokens.size(); i++)
	{
		std::cout << Lexer::GetTokenAsString(Lexer::Tokens[i]);

		if(Lexer::Tokens[i] == Lexer::Token::TK_Number || Lexer::Tokens[i] == Lexer::Token::TK_Identifier)
		{
			std::cout << " (" << Lexer::GlobalStringInfo[Lexer::Tokens[i + 1]] << ")";
			i++;
		}

		if(i < Lexer::Tokens.size())
			std::cout << ", ";
	}

	std::cout << "End Of File]\n";

	Parser::Start();

	return 0;
}