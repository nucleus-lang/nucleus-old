#ifndef PARSER_H
#define PARSER_H

#include "AbstractSyntaxTree.hpp"
#include <memory>
#include <string>

struct Parser
{
	static std::unique_ptr<AST::Expression> LogError(std::string str)
	{
		fprintf(stderr, "Error: %s\n", str.c_str());
		exit(1);
		return nullptr;
	}

	static std::unique_ptr<AST::FunctionPrototype> LogErrorP(std::string str)
	{
		LogError(str);
		exit(1);
		return nullptr;
	}

	static std::unique_ptr<AST::Function> LogErrorF(std::string str)
	{
		LogError(str);
		exit(1);
		return nullptr;
	}

	static std::unique_ptr<AST::FunctionPrototype> LogErrorFP(std::string str)
	{
		LogError(str);
		exit(1);
		return nullptr;
	}

	static std::unique_ptr<AST::Expression> ParseNumber()
	{
		auto Result = std::make_unique<AST::Number>(Lexer::NumValString);
		Lexer::GetNextToken();
		return std::move(Result);
	}

	static std::unique_ptr<AST::Expression> ParseParenthesis()
	{
		Lexer::GetNextToken();
		auto V = ParseExpression();

		if(!V)
			return nullptr;

		if(Lexer::CurrentToken != ')')
			return LogError("Expected ')'");

		Lexer::GetNextToken();
		return V;
	}

	static std::unique_ptr<AST::Expression> ParseIdentifier()
	{
		std::string idName = Lexer::IdentifierStr;

		Lexer::GetNextToken();

		if(Lexer::CurrentToken != '(')
			return std::make_unique<AST::Variable>(idName);

		Lexer::GetNextToken();
		std::vector<std::unique_ptr<AST::Expression>> Arguments;

		if(Lexer::CurrentToken != ')')
		{
			while (1)
			{
				if(auto Argument = ParseExpression())
					Arguments.push_back(std::move(Argument));
				else
					return nullptr;

				if(Lexer::CurrentToken == ')')
					break;

				if(Lexer::CurrentToken != Token::TK_Comma)
					return LogError("Expected ')' or ',' in argument list");

				Lexer::GetNextToken();
			}
		}

		Lexer::GetNextToken();

		return std::make_unique<AST::Call>(idName, std::move(Arguments));
	}

	static std::unique_ptr<AST::Expression> ParseDotComma()
	{
		// Do nothing for now...

		Lexer::GetNextToken();

		return ParsePrimary();
	}

	static std::unique_ptr<AST::Expression> ParsePrimary()
	{
		switch(Lexer::CurrentToken)
		{
			default:
				return LogError("Unknown token '" + std::to_string(Lexer::CurrentToken) + "' when expecting an expression");
			case Token::TK_Identifier:
				return ParseIdentifier();
			case Token::TK_Number:
				return ParseNumber();
			case Token::TK_DotComma:
				return ParseDotComma();
			case '(':
				return ParseParenthesis();
			case Token::TK_If:
				return ParseIfExpression();
		}
	}

	static std::unique_ptr<AST::Expression> ParseExpression()
	{
		auto LHS = ParsePrimary();
		if(!LHS)
			return nullptr;

		return ParseBinaryOperatorRight(0, std::move(LHS));
	}

	static std::unique_ptr<AST::Expression> ParseBracket()
	{
		Lexer::GetNextToken();
		auto V = ParseExpression();

		if(!V)
			return nullptr;

		if(Lexer::CurrentToken != '}')
			return LogError("Expected '}'");

		Lexer::GetNextToken();
		return V;
	}

	static std::unique_ptr<AST::Expression> ParseBinaryOperatorRight(int ExpressionPrecedence, std::unique_ptr<AST::Expression> LHS)
	{
		while(1)
		{
			int TokenPrecedence = GetTokenPrecedence();

			if(TokenPrecedence < ExpressionPrecedence)
				return LHS;

			int BinaryOperator = Lexer::CurrentToken;
			Lexer::GetNextToken();

			auto RHS = ParsePrimary();
			if(!RHS)
				return nullptr;

			int NextTokenPrecedence = GetTokenPrecedence();

			if(TokenPrecedence < NextTokenPrecedence)
			{
				RHS = ParseBinaryOperatorRight(TokenPrecedence + 1, std::move(RHS));
				if(!RHS)
					return nullptr;
			}

			LHS = std::make_unique<AST::Binary>(BinaryOperator, std::move(LHS), std::move(RHS));
		}
	}

	static std::map<char, int> BinaryOpPrecedence;

	static int GetTokenPrecedence()
	{
		if(!isascii(Lexer::CurrentToken))
			return -1;

		int TokenPrecedence = BinaryOpPrecedence[Lexer::CurrentToken];

		if(TokenPrecedence <= 0) return -1;
		return TokenPrecedence;
	}

	static bool IsTokenABasicType()
	{
		return Lexer::CurrentToken == Token::TK_Integer ||
		Lexer::CurrentToken == Token::TK_Double ||
		Lexer::CurrentToken == Token::TK_Float;
	}

	static std::unique_ptr<AST::FunctionPrototype> ParsePrototype()
	{
		//Lexer::GetNextToken();

		std::vector<std::pair<std::unique_ptr<AST::Expression>, std::unique_ptr<AST::Variable>>> ArgumentNames;

		if(Lexer::CurrentToken != Token::TK_Identifier)
			return LogErrorP("Expected function name in prototype");

		std::string FunctionName = Lexer::IdentifierStr;

		if(FunctionName == "main")
			FunctionName = "__main";

		Lexer::GetNextToken();

		if(Lexer::CurrentToken != '(')
			return LogErrorP("Expected '(' in function prototype");

		Lexer::GetNextToken();
		while(Lexer::CurrentToken == Token::TK_Identifier || Lexer::CurrentToken == Token::TK_Comma || IsTokenABasicType())
		{
			Lexer::GetNextToken();

			auto VarName = std::make_unique<AST::Variable>(Lexer::IdentifierStr);

			if(Lexer::CurrentToken != ':')
				return LogErrorP("Expected ':'");

			Lexer::GetNextToken();

			auto Expr = ParseFunctionType();
			
			if(!Expr)
				return LogErrorP("Expected argument type");

			ArgumentNames.push_back(std::make_pair(std::move(Expr), std::move(VarName)));
			Lexer::GetNextToken();

			if(Lexer::CurrentToken != Token::TK_Comma && Lexer::CurrentToken != ')')
				return LogErrorP("Expected ',' in function prototype");
			else
			{
				std::cout << "Comma detected!\n";

				if(Lexer::CurrentToken == ')')
					break;
			}

			Lexer::GetNextToken();
		}
		if(Lexer::CurrentToken != ')')
			return LogErrorP("Expected ')' in function prototype");

		Lexer::GetNextToken();

		if(Lexer::CurrentToken != '-')
			return LogErrorP("Expected arrow to specify function type.");

		Lexer::GetNextToken();

		if(Lexer::CurrentToken != '>')
			return LogErrorP("Expected arrow to specify function type.");

		Lexer::GetNextToken();

		auto FnType = ParseFunctionType();
		if(!FnType)
			return LogErrorP("Expected function type.");

		Lexer::GetNextToken();

		return std::make_unique<AST::FunctionPrototype>(std::move(FnType), FunctionName, std::move(ArgumentNames));
	}

	static std::unique_ptr<AST::Expression> ParseFunctionType()
	{
		if(Lexer::CurrentToken == Token::TK_Integer)
			return std::make_unique<AST::Integer>(0);
		else if(Lexer::CurrentToken == Token::TK_Double)
			return std::make_unique<AST::Double>(0);
		else if(Lexer::CurrentToken == Token::TK_Float)
			return std::make_unique<AST::Float>(0);

		return nullptr;
	}

	static std::unique_ptr<AST::Function> ParseDefinition()
	{
		Lexer::GetNextToken();

		auto Proto = ParsePrototype();
		if(!Proto) 
			return nullptr;

		if(Lexer::CurrentToken != '{')
			LogErrorF("Expected '{'");

		Lexer::GetNextToken();

		auto Expression = ParseExpression();

		Lexer::GetNextToken();

		if(Lexer::CurrentToken != '}')
			LogErrorF("Expected '}'. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");

		if(Expression)
			return std::make_unique<AST::Function>(std::move(Proto), std::move(Expression));

		return nullptr;
	}

	static std::unique_ptr<AST::FunctionPrototype> ParseExtern()
	{
		Lexer::GetNextToken();
		auto Proto = ParsePrototype();

		if(Lexer::CurrentToken != Token::TK_DotComma)
			return LogErrorFP("Expected ';' at end of extern. Detected token: " + std::to_string(Lexer::CurrentToken) + ".");

		//Lexer::GetNextToken();

		return Proto;
	}

	static std::unique_ptr<AST::Function> ParseTopLevelExpression()
	{
		Lexer::GetNextToken();

		if(auto Expression = ParseExpression())
		{
			auto Proto = std::make_unique<AST::FunctionPrototype>(std::make_unique<AST::Double>(0), "", std::vector<std::pair<std::unique_ptr<AST::Expression>, std::unique_ptr<AST::Variable>>>());
			return std::make_unique<AST::Function>(std::move(Proto), std::move(Expression));
		}

		return nullptr;
	}

	static std::unique_ptr<AST::Expression> ParseIfExpression()
	{
		Lexer::GetNextToken();

		if(Lexer::CurrentToken != '(')
			return LogError("Expected '('.");

		auto Condition = ParseParenthesis();

		if(!Condition)
			return nullptr;

		if(Lexer::CurrentToken != '{')
			return LogError("Expected '{' at end of condition.");

		Lexer::GetNextToken();

		auto Then = ParseExpression();

		if(!Then)
			return nullptr;

		Lexer::GetNextToken();

		if(Lexer::CurrentToken != '}')
			return LogError("Expected '}' at end of if block. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");

		Lexer::GetNextToken();

		if(Lexer::CurrentToken != Token::TK_Else)
			return LogError("Expected 'else'.");

		Lexer::GetNextToken();

		if(Lexer::CurrentToken != '{')
			return LogError("Expected '{' at end of condition.");

		Lexer::GetNextToken();

		auto Else = ParseExpression();

		if(!Else)
			return nullptr;

		Lexer::GetNextToken();

		if(Lexer::CurrentToken != '}')
			return LogError("Expected '}' at end of else block. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");

		return std::make_unique<AST::If>(std::move(Condition), std::move(Then), std::move(Else));
	}

	//static std::unique_ptr<Expression> ParseForExpression()
	//{
	//	Lexer::GetNextToken();

		
	//}
};

struct ParseTesting
{
	static void Definition()
	{
		if(auto FnAST = Parser::ParseDefinition())
		{
			std::cout << "Handling Definition...\n";

			if(auto *FnIR = FnAST->codegen())
			{
				fprintf(stderr, "Parsed a function definition:\n\n");
				FnIR->print(llvm::errs());
				fprintf(stderr, "\n");

				if(FnAST->name == "__main")
				{
					CodeGeneration::ExitOnErr(
					CodeGeneration::TheJIT->addModule(
						llvm::orc::ThreadSafeModule(std::move(CodeGeneration::TheModule), std::move(CodeGeneration::TheContext))));
					
					auto ExprSymbol = CodeGeneration::ExitOnErr(CodeGeneration::TheJIT->lookup("__main"));

					if(dynamic_cast<AST::Integer*>(FnAST->type.get()) != nullptr)
					{
						int (*FP)() = (int(*)())ExprSymbol.getAddress();
      					std::cout << "Result: " << FP() << "\n";
      				}
      				else if(dynamic_cast<AST::Double*>(FnAST->type.get()) != nullptr)
      				{
      					double (*FP)() = (double(*)())ExprSymbol.getAddress();
      					std::cout << "Result: " << FP() << "\n";
      				}
      				else if(dynamic_cast<AST::Float*>(FnAST->type.get()) != nullptr)
      				{
      					float (*FP)() = (float(*)())ExprSymbol.getAddress();
      					std::cout << "Result: " << FP() << "\n";
      				}
      			}
			}

			std::cout << "Finished Definition...\n";
		}
		else
			Lexer::GetNextToken();
	}

	static void Extern()
	{
		if(auto ProtoAST = Parser::ParseExtern())
		{
			std::cout << "Handling Extern...\n";

			if(auto *FnIR = ProtoAST->codegen())
			{
				fprintf(stderr, "Parsed an extern:");
				FnIR->print(llvm::errs());
				fprintf(stderr, "\n");

				AST::FunctionProtos[ProtoAST->Name()] = std::move(ProtoAST);
				
				std::cout << "Finished Extern...\n";
			}
		}
		else
			Lexer::GetNextToken();
	}

	static void TopLevelExpression()
	{
		if(auto FnAST = Parser::ParseTopLevelExpression())
		{
			std::cout << "Handling Top Level Expression...\n";

			if(auto *FnIR = FnAST->codegen())
			{
				fprintf(stderr, "Parsed a top level expression:");
				FnIR->print(llvm::errs());
				fprintf(stderr, "\n");

				FnIR->eraseFromParent();
				return;
			}
		}
		else
			Lexer::GetNextToken();
	}
};

#endif