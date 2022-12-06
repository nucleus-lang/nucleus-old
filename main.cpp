#include "llvm/ADT/APFloat.h"
#include "Language/Lexer.hpp"
#include "Language/AbstractSyntaxTree.hpp"
#include "Language/Parser.hpp"

int Lexer::CurrentToken;
std::string Lexer::IdentifierStr;
std::string Lexer::NumValString;
std::map<char, int> Parser::BinaryOpPrecedence;

void MainLoop()
{
	while(true)
	{
		fprintf(stderr, "nucleus> ");
		switch(Lexer::CurrentToken)
		{
			case Token::TK_EndOfFile:
				exit(0);
				break;
			case Token::TK_Define:
				std::cout << "Looking for Function...\n";
				ParseTesting::Definition();
				Lexer::GetNextToken();
				break;
			case Token::TK_Extern:
				std::cout << "Looking for Extern...\n";
				ParseTesting::Extern();
				Lexer::GetNextToken();
				break;
			//default:
			//	std::cout << "Looking for Top Level Expression...\n";
			//	ParseTesting::TopLevelExpression();
			//	break;
			default:
				exit(0);
				break;
		}
	}
}

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

extern "C" DLLEXPORT double putchard(double X) {
  fputc((char)X, stderr);
  return 0;
}

extern "C" DLLEXPORT double printd(double X) {
  fprintf(stderr, "%f\n", X);
  return 0;
}

int main()
{
	llvm::InitializeNativeTarget();
  	llvm::InitializeNativeTargetAsmPrinter();
  	llvm::InitializeNativeTargetAsmParser();

	Parser::BinaryOpPrecedence['<'] = 10;
	Parser::BinaryOpPrecedence['+'] = 20;
	Parser::BinaryOpPrecedence['-'] = 20;
	Parser::BinaryOpPrecedence['*'] = 40;

	fprintf(stderr, "nucleus> ");
	Lexer::GetNextToken();

	CodeGeneration::StartJIT();

	CodeGeneration::Initialize();

	MainLoop();

	return 0;
}
