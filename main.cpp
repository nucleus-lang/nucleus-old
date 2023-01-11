#include "llvm/ADT/APFloat.h"
#include "Language/Lexer.hpp"
#include "Language/AbstractSyntaxTree.hpp"
#include "Language/Parser.hpp"
#include <fstream>
#include <filesystem>
#include "Language/NucleusToml.hpp"

int Lexer::CurrentToken;
std::string Lexer::IdentifierStr;
std::string Lexer::NumValString;
char Lexer::CharVal;
std::string Lexer::StringString;
std::string Lexer::EntireScriptContent;
int Lexer::LastECChar = -1;
std::string Lexer::GetSavedString = "";
bool Lexer::RecordString = false;

std::map<char, int> Parser::BinaryOpPrecedence;
AST::Array* Parser::lastArray = nullptr;
AST::NestedArray* Parser::lastNestedArray = nullptr;
unsigned int Parser::bracketCount = 0;
std::vector<std::string> Parser::localArrayNames, Parser::localNestedArrayNames;
std::string Parser::globalAutoExterns;

std::vector<std::pair<std::string, std::string>> Parser::localStructVariables;

std::string Parser::currentIdentifierString;
bool Parser::dotCommaAsOperator = true, Parser::beginNestedArray = false, Parser::endNestedArray = true, Parser::disableOperators = false;

std::vector<std::unique_ptr<AST::Function>> ParseTesting::allParsedFunctions;

std::vector<std::unique_ptr<AST::StructEx>> Parser::AllStructs;

SourceLocation Lexer::CurrentLocation;
SourceLocation Lexer::LexerLocation = {1, 0};

std::vector<std::string> NucleusTOML::folders;
std::vector<std::string> NucleusTOML::CPPIncludes;

void MainLoop()
{
	bool closeLoop = false;
	while(!closeLoop)
	{
		//fprintf(stderr, "nucleus> ");
		switch(Lexer::CurrentToken)
		{
			case Token::TK_EndOfFile:
				//std::cout << "Found " << Lexer::CurrentToken << ".\n Closing...\n";
				Lexer::CurrentToken = 0;
				closeLoop = true;
				break;
			case Token::TK_Define:
				//std::cout << "Looking for Function...\n";
				ParseTesting::Definition();
				Lexer::GetNextToken();
				break;
			case Token::TK_Extern:
				//std::cout << "Looking for Extern...\n";
				ParseTesting::Extern();
				Lexer::GetNextToken();
				break;
			case Token::TK_Struct:
				//std::cout << "Looking for Extern...\n";
				ParseTesting::Struct();
				Lexer::GetNextToken();
				break;
			//default:
			//	std::cout << "Looking for Top Level Expression...\n";
			//	ParseTesting::TopLevelExpression();
			//	break;
			default:
				Lexer::GetNextToken();

				//if(Lexer::CurrentToken != 0)
				//	std::cout << "Found " << Lexer::CurrentToken << ".\n Continue...\n";
				//closeLoop = true;
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

int main(int argc, const char* argv[])
{
	llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  Parser::BinaryOpPrecedence[';'] = 1;
  Parser::BinaryOpPrecedence['='] = 2;
	Parser::BinaryOpPrecedence['<'] = 10;
	Parser::BinaryOpPrecedence['>'] = 10;
	Parser::BinaryOpPrecedence['+'] = 20;
	Parser::BinaryOpPrecedence['-'] = 20;
	Parser::BinaryOpPrecedence['*'] = 40;

	//fprintf(stderr, "nucleus> ");
	Lexer::GetNextToken();

	CodeGeneration::StartJIT();

	CodeGeneration::Initialize();

	NucleusTOML::Read("Nucleus.toml");

	Lexer::EntireScriptContent = "extern ";

	for(auto i : NucleusTOML::folders)
	{
		for (const auto & entry : std::filesystem::directory_iterator(i))
	 	{
	 			if(entry.path().u8string().find(".nk") != std::string::npos)
	 			{
	 	   	std::ifstream ifs(entry.path().u8string().c_str());
					std::string content( (std::istreambuf_iterator<char>(ifs) ),
	 	                    			(std::istreambuf_iterator<char>()    ) );
	
					Lexer::EntireScriptContent += content;
					Lexer::EntireScriptContent += "\n";
	 			}
	 	}
	}

	std::string path = std::filesystem::current_path().u8string();

   for (const auto & entry : std::filesystem::directory_iterator(path))
   {
   		if(entry.path().u8string().find(".nk") != std::string::npos)
   		{
      	std::ifstream ifs(entry.path().u8string().c_str());
  			std::string content( (std::istreambuf_iterator<char>(ifs) ),
                       			(std::istreambuf_iterator<char>()    ) );

  			Lexer::EntireScriptContent += content;
  			Lexer::EntireScriptContent += "\n";
   		}
   }

  //std::cout << Lexer::EntireScriptContent << "\n";

	MainLoop();

	ParseTesting::CompileFunctions();

	CodeGeneration::CompileToObjectCode();

	return 0;
}
