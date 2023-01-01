#ifndef PARSER_H
#define PARSER_H

#include "AbstractSyntaxTree.hpp"
#include <memory>
#include <string>

struct Parser
{
	static AST::Array* lastArray;
	static AST::NestedArray* lastNestedArray;
	static std::vector<std::string> localArrayNames, localNestedArrayNames;

	static std::vector<std::pair<std::string, std::string>> localStructVariables;

	static std::string currentIdentifierString;
	static unsigned int bracketCount;
	static bool dotCommaAsOperator, beginNestedArray, endNestedArray, disableOperators;

	static std::vector<std::unique_ptr<AST::StructEx>> AllStructs;

	template<typename TO, typename FROM>
	static std::unique_ptr<TO> static_unique_pointer_cast (std::unique_ptr<FROM>&& old)
	{
    	return std::unique_ptr<TO>{static_cast<TO*>(old.release())};
    	// conversion: unique_ptr<FROM>->FROM*->TO*->unique_ptr<TO>
	}

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

	static std::unique_ptr<AST::Expression> ParseCharValue()
	{
		std::unique_ptr<AST::Number> Result = std::make_unique<AST::Number>(std::to_string((int)Lexer::CharVal));
		Result->bit = 32;
		Lexer::GetNextToken();
		return std::move(Result);
	}

	static std::unique_ptr<AST::Expression> ParseTrue()
	{
		std::unique_ptr<AST::Number> Result = std::make_unique<AST::Number>("1");
		Result->bit = 1;
		Lexer::GetNextToken();
		return std::move(Result);
	}

	static std::unique_ptr<AST::Expression> ParseFalse()
	{
		std::unique_ptr<AST::Number> Result = std::make_unique<AST::Number>("0");
		Result->bit = 1;
		Lexer::GetNextToken();
		return std::move(Result);
	}

	static std::unique_ptr<AST::Expression> ParseParenthesis()
	{
		Lexer::GetNextToken();
		auto V = ParseExpression();

		if(!V)
			return nullptr;

		if(Lexer::CurrentToken == '}')
			Lexer::GetNextToken();

		if(Lexer::CurrentToken != ')')
			return LogError("Expected ')'. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");

		Lexer::GetNextToken();
		return V;
	}

	static std::vector<std::string> SplitIntoVector(const std::string& s, const char* c)
	{
		std::vector<std::string> newV;
		std::string::size_type i = 0;
		std::string::size_type j = s.find(c);
	
		while (j != std::string::npos)
		{
			newV.push_back(s.substr(i, j - i));
			i = ++j;
			j = s.find(c, j);
	
			if (j == std::string::npos)
				newV.push_back(s.substr(i, s.length()));
		}
	
		if (newV.size() == 0)
			newV.push_back(s);
	
		return newV;
	}

	static std::unique_ptr<AST::Expression> ParseIdentifier(bool recursiveAnalysis = false)
	{
		std::string idName = Lexer::IdentifierStr;

		//std::cout << "Identifier Str: " << idName << "\n";

		SourceLocation LitLoc = Lexer::CurrentLocation;

		Lexer::GetNextToken();

		//std::unique_ptr<AST::Variable> structVar = std::make_unique<AST::Variable>(LitLoc, idName);

		if(Lexer::CurrentToken == '.')
		{
			std::unique_ptr<AST::Variable> structMember = std::make_unique<AST::Variable>(LitLoc, idName);
			structMember->isArrayElement = true;

			//std::cout << "Checking 0...\n";

			for(auto i : localStructVariables)
			{
				if(i.first == idName)
				{
					//std::cout << "Checking 1...\n";

					for(int j = 0; j < Parser::AllStructs.size(); j++)
					{
						if(Parser::AllStructs[j]->Name == i.second)
						{
							Lexer::GetNextToken();

							auto GetSecondVar = ParseIdentifier();

							if(static_cast<AST::Variable*>(GetSecondVar.get()))
							{
								//std::cout << "Checking 2...\n";

								AST::Variable* getV = (AST::Variable*)GetSecondVar.get();

								std::string GetName = getV->name;

								for(int m = 0; m < Parser::AllStructs[j]->variables.size(); m++)
								{
									AST::Var* p = (AST::Var*)Parser::AllStructs[j]->variables[m].get();

									//std::cout << "Checking 3...\n";

									if(p->VarNames[0].name == GetName)
									{
										auto ind = std::make_unique<AST::Number>(std::to_string(m));
										ind->bit = 32;

										structMember->arrayIndex = std::move(ind);

										return structMember;
									}
								}
							}
						}
					}
				}
			}
		}

		if(Lexer::CurrentToken == '<')
		{
			for(auto i : localNestedArrayNames)
			{
				if(i == idName)
				{
					std::unique_ptr<AST::Expression> Ind = nullptr;
	
					std::unique_ptr<AST::Variable> nestedArray = std::make_unique<AST::Variable>(LitLoc, idName);
					nestedArray->isTDArrayElement = true;
		
					//std::cout << "Parsing Nested Array Index...\n";
		
					Lexer::GetNextToken();
		
					Ind = ParseExpression();
		
					if(!Ind)
						return nullptr;
		
					nestedArray->TDArrayIndex = std::move(Ind);
		
					if(Lexer::CurrentToken != '>')
						return LogError("Expected '>' to close two-dimensional array item. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");
		
					Lexer::GetNextToken();
		
					return nestedArray;
				}
			}
		}
		if(Lexer::CurrentToken == '[')
		{
			std::unique_ptr<AST::Variable> arrayElement = std::make_unique<AST::Variable>(LitLoc, idName);
			arrayElement->isArrayElement = true;

			Lexer::GetNextToken();

			if(Lexer::CurrentToken != ']')
			{
				auto Ind = ParseExpression();

				if(!Ind)
					return nullptr;

				arrayElement->arrayIndex = std::move(Ind);

				if(Lexer::CurrentToken != ']')
					return LogError("Expected ']' to close index array. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");
			}

			Lexer::GetNextToken();

			return arrayElement;
		}
		else if(Lexer::CurrentToken != '(')
		{

			for(auto i : localArrayNames)
			{
				if(i == idName)
				{
					std::unique_ptr<AST::Variable> arrayPointer = std::make_unique<AST::Variable>(LitLoc, idName);

					auto Ind = std::make_unique<AST::Number>("0");

					arrayPointer->arrayIndex = std::move(Ind);
					arrayPointer->isArrayPointer = true;

					return arrayPointer;
				}
			}

			for(auto i : localNestedArrayNames)
			{
				if(i == idName)
				{
					std::unique_ptr<AST::Variable> nestedArray = std::make_unique<AST::Variable>(LitLoc, idName);

					auto Ind = std::make_unique<AST::Number>("0");

					nestedArray->TDArrayIndex = std::move(Ind);
					nestedArray->isTDArrayPointer = true;

					return nestedArray;
				}
			}

			//std::cout << idName << " not found in localNestedArrayNames.\n";

			return std::make_unique<AST::Variable>(LitLoc, idName);
		}

		//std::cout << idName << "'(' found!\n";

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

		return std::make_unique<AST::Call>(LitLoc, idName, std::move(Arguments));
	}

	static std::unique_ptr<AST::Expression> ParseArrayInit()
	{
		//std::cout << "Parsing Array Init Content...\n";

		std::vector<std::unique_ptr<AST::Expression>> variables;

		Lexer::GetNextToken();

		while(1)
		{
			auto V = ParseExpression();

			if(!V)
				return nullptr;

			variables.push_back(std::move(V));

			if(Lexer::CurrentToken != Token::TK_Comma)
			{
				break;
			}

			Lexer::GetNextToken();
		}

		if(Lexer::CurrentToken != ']')
			return LogError("Expected ']' to close array initialization. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");

		Lexer::GetNextToken();

		return std::make_unique<AST::ArrayInitContent>(std::move(variables));
	}

	static std::unique_ptr<AST::Expression> ParsePrimary()
	{
		switch(Lexer::CurrentToken)
		{
			default:
				if(Lexer::CurrentToken == Token::TK_Var || Lexer::CurrentToken == Token::TK_Ptr)
					return ParseVar();

				return LogError("Unknown token '" + std::to_string(Lexer::CurrentToken) + "' when expecting an expression");
			case Token::TK_Identifier:
				return ParseIdentifier();
			case Token::TK_Number:
				return ParseNumber();
			case Token::TK_CharValue:
				return ParseCharValue();

			case Token::TK_True:
				return ParseTrue();
			case Token::TK_False:
				return ParseFalse();	

			case '(':
				return ParseParenthesis();
			case Token::TK_If:
				return ParseIfExpression();
			case Token::TK_For:
				return ParseForExpression();

			case '[':
				return ParseArrayInit();

			case Token::TK_StringContent:
				return ParseStringContent();

			case '<':
				return ParseNestedArrayInit();
		}
	}

	static std::unique_ptr<AST::Expression> ParseNestedArrayInit()
	{
		//std::cout << "Parsing Nested Array...\n";

		std::vector<std::unique_ptr<AST::Expression>> arrays;

		beginNestedArray = false;

		Lexer::GetNextToken();

		while(1)
		{
			//std::cout << "Got an array!\n";
			auto arr = ParseExpression();

			if(dynamic_cast<AST::ArrayInitContent*>(arr.get()) == nullptr && dynamic_cast<AST::Array*>(arr.get()) == nullptr)
				return LogError("Initializer from two-dimensional array is not an array. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");

			arrays.push_back(std::move(arr));

			if(Lexer::CurrentToken != Token::TK_Comma)
			{
				if(Lexer::CurrentToken == '>')
				{
					endNestedArray = true;
					break;
				}
				else
				{
					return LogError("Expected ',' or '>' inside two-dimensional array initialization. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");
				}
			}

			Lexer::GetNextToken();
		}

		endNestedArray = true;

		Lexer::GetNextToken();

		endNestedArray = false;

		return std::make_unique<AST::NestedArrayContent>(std::move(arrays));
	}

	static std::unique_ptr<AST::Expression> ParseString()
	{
		std::unique_ptr<AST::Integer> type = std::make_unique<AST::Integer>(0);
		type->bit = 8;

		Lexer::GetNextToken();

		auto arr = std::make_unique<AST::Array>(std::move(type), 0);
		arr->dynamicInitialization = true;

		//std::cout << "Array " << currentIdentifierString << " parsed!\n";
		localArrayNames.push_back(currentIdentifierString);

		return arr;
	}

	static std::unique_ptr<AST::Expression> ParseStringContent()
	{
		std::vector<std::unique_ptr<AST::Expression>> variables;

		for(int i = 0; i < Lexer::StringString.size(); i++)
		{
			std::unique_ptr<AST::Number> Result = std::make_unique<AST::Number>(std::to_string((int)Lexer::StringString[i]));
			Result->bit = 8;

			variables.push_back(std::move(Result));
		}

		auto EndString = std::make_unique<AST::Number>(std::to_string((int)'\0'));
		EndString->bit = 8;

		variables.push_back(std::move(EndString));

		Lexer::GetNextToken();

		return std::make_unique<AST::ArrayInitContent>(std::move(variables));
	}

	static std::unique_ptr<AST::Expression> ParseExpression()
	{
		auto LHS = ParseUnary();
		if(!LHS)
			return nullptr;

		if(!disableOperators)
			return ParseBinaryOperatorRight(0, std::move(LHS));

		return LHS;
	}

	static std::unique_ptr<AST::Expression> ParseBracket()
	{
		bracketCount++;

		Lexer::GetNextToken();
		auto V = ParseExpression();

		if(!V)
			return nullptr;

		if(Lexer::CurrentToken != '}')
			return LogError("Expected '}'");

		bracketCount--;

		Lexer::GetNextToken();
		return V;
	}

	static std::unique_ptr<AST::Expression> ParseBinaryOperatorRight(int ExpressionPrecedence, std::unique_ptr<AST::Expression> LHS)
	{
		while(1)
		{
			int TokenPrecedence = GetTokenPrecedence();

			if(TokenPrecedence < ExpressionPrecedence || Lexer::CurrentToken == '}')
				return LHS;

			int BinaryOperator = Lexer::CurrentToken;
			SourceLocation BinLoc = Lexer::CurrentLocation;
			Lexer::GetNextToken();

			if(Lexer::CurrentToken == '}')
				return LHS;

			auto RHS = ParseUnary();
			if(!RHS)
				return nullptr;

			int NextTokenPrecedence = GetTokenPrecedence();

			if(TokenPrecedence < NextTokenPrecedence)
			{
				RHS = ParseBinaryOperatorRight(TokenPrecedence + 1, std::move(RHS));
				if(!RHS)
					return nullptr;
			}

			LHS = std::make_unique<AST::Binary>(BinLoc, BinaryOperator, std::move(LHS), std::move(RHS));
		}
	}

	static std::map<char, int> BinaryOpPrecedence;

	static int GetTokenPrecedence()
	{
		if(Lexer::CurrentToken == Token::TK_DotComma && dotCommaAsOperator)
			Lexer::CurrentToken = ';';

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
		Lexer::CurrentToken == Token::TK_Float ||
		Lexer::CurrentToken == Token::TK_String;
	}

	static std::unique_ptr<AST::FunctionPrototype> ParsePrototype()
	{
		//Lexer::GetNextToken();

		unsigned Kind = 0; // 0 = identifier, 1 = unary, 2 = binary.
		unsigned BinaryPrecedence = 30;
		std::string FunctionName;

		SourceLocation FnLoc = Lexer::CurrentLocation;

		std::vector<std::pair<std::unique_ptr<AST::Expression>, std::unique_ptr<AST::Variable>>> ArgumentNames;

		if(Lexer::CurrentToken == Token::TK_Identifier)
		{
			FunctionName = Lexer::IdentifierStr;
			Kind = 0;
			Lexer::GetNextToken();

			//if(FunctionName == "main")
			//	FunctionName = "__main";
		}
		else if(Lexer::CurrentToken == Token::TK_Unary)
		{
			Lexer::GetNextToken();

			if(!isascii(Lexer::CurrentToken))
				return LogErrorP("Expected binary operator.");

			FunctionName = "unary";
			FunctionName += (char)Lexer::CurrentToken;

			Kind = 1;
			Lexer::GetNextToken();
		}
		else if(Lexer::CurrentToken == Token::TK_Binary)
		{
			Lexer::GetNextToken();

			if(!isascii(Lexer::CurrentToken))
				return LogErrorP("Expected binary operator.");

			FunctionName = "binary";
			FunctionName += (char)Lexer::CurrentToken;
			Kind = 2;

			Lexer::GetNextToken();

			if(Lexer::CurrentToken == Token::TK_Number)
			{
				int NumValResult = 0;

				if(Lexer::NumValString.find(".") != std::string::npos)
				{
					if(Lexer::NumValString.back() == 'f')
						NumValResult = int(std::stof(Lexer::NumValString));
					else
						NumValResult = int(std::stod(Lexer::NumValString));
				}
				else
					NumValResult = std::stoi(Lexer::NumValString);

				if(NumValResult < 1 || NumValResult > 100)
					return LogErrorP("Binary Precedence must be between 1 and 100.");

				BinaryPrecedence = (unsigned)NumValResult;

				Lexer::GetNextToken();
			}
		}
		else
		{
			return LogErrorP("Expected function name in prototype");
		}

		if(Lexer::CurrentToken != '(')
			return LogErrorP("Expected '(' in function prototype");

		Lexer::GetNextToken();
		while(Lexer::CurrentToken == Token::TK_Identifier || Lexer::CurrentToken == Token::TK_Comma || IsTokenABasicType()
			|| Lexer::CurrentToken == '[' || Lexer::CurrentToken == ']' || Lexer::CurrentToken == '<')
		{
			Lexer::GetNextToken();

			SourceLocation VarLoc = Lexer::CurrentLocation;

			currentIdentifierString = Lexer::IdentifierStr;
			auto VarName = std::make_unique<AST::Variable>(VarLoc, Lexer::IdentifierStr);

			if(Lexer::CurrentToken != ':')
				return LogErrorP("Expected ':'");

			Lexer::GetNextToken();

			auto Expr = ParseFunctionType();

			if(!Expr)
				return LogErrorP("Expected argument type");

			ArgumentNames.push_back(std::make_pair(std::move(Expr), std::move(VarName)));

			if(Lexer::CurrentToken != Token::TK_Comma && Lexer::CurrentToken != ')' && Lexer::CurrentToken != '[' && Lexer::CurrentToken == '<')
				return LogErrorP("Expected ',' in function prototype. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");
			else
			{
				//std::cout << "Comma detected!\n";

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

		if(Kind && ArgumentNames.size() != Kind)
			return LogErrorP("Invalid number of operands for operator.");

		return std::make_unique<AST::FunctionPrototype>(FnLoc, std::move(FnType), FunctionName, std::move(ArgumentNames), Kind != 0, BinaryPrecedence);
	}

	static std::unique_ptr<AST::Expression> ParseFunctionType()
	{
		std::unique_ptr<AST::Expression> ty = nullptr;

		if(Lexer::CurrentToken == '<')
		{
			unsigned int nestedArraySize = 0;

			Lexer::GetNextToken();

			bool dyn = false;

			ty = ParseFunctionType();

			if(static_cast<AST::Array*>(ty.get()) == nullptr)
				return LogError("Type must've be array for two-dimensional setup. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");

			if(Lexer::CurrentToken != '>')
			{
				if(Lexer::CurrentToken == '(')
				{
					Lexer::GetNextToken();

					if(Lexer::CurrentToken == Token::TK_Number)
					{
						if(Lexer::NumValString.find(".") != std::string::npos || Lexer::NumValString.find("f") != std::string::npos)
							nestedArraySize = 0;
						else
							nestedArraySize = std::stoi(Lexer::NumValString);
		
						Lexer::GetNextToken();
					}
					else
						return LogError("Expected integer to set two-dimensional array size. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");

					if(Lexer::CurrentToken != ')')
						return LogError("Expected ')' to close two-dimensional array size. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");

					Lexer::GetNextToken();
				}
				else
					return LogError("Expected '(' to open two-dimensional array size. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");

				if(Lexer::CurrentToken != '>')
					return LogError("Expected '>' to close two-dimensional array. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");
			}
			else
			{
				dyn = true;
			}

			Lexer::GetNextToken();

			auto arr = std::make_unique<AST::NestedArray>(std::move(ty), nestedArraySize);
			arr->dynamicInitialization = dyn;

			//std::cout << "Nested Array " << currentIdentifierString << " parsed!\n";
			localNestedArrayNames.push_back(currentIdentifierString);

			return arr;
		}
		else if(Lexer::CurrentToken == Token::TK_Integer)
		{
			auto i = std::make_unique<AST::Integer>(0);

			Lexer::GetNextToken();

			if(Lexer::CurrentToken == Token::TK_Number)
			{
				if(Lexer::NumValString.find(".") != std::string::npos || Lexer::NumValString.find("f") != std::string::npos)
				{
					i->bit = 32;
				}

				int intByte = std::stoi(Lexer::NumValString);

				i->bit = intByte;

				Lexer::GetNextToken();
			}
			else
			{
				i->bit = 32;
			}

			ty = std::move(i);
		}
		else if(Lexer::CurrentToken == Token::TK_Bool)
		{
			auto i = std::make_unique<AST::Integer>(0);
			i->bit = 1;
			Lexer::GetNextToken();
			ty = std::move(i);
		}
		else if(Lexer::CurrentToken == Token::TK_Char)
		{
			auto i = std::make_unique<AST::Integer>(0);
			i->bit = 8;
			Lexer::GetNextToken();
			ty = std::move(i);
		}
		else if(Lexer::CurrentToken == Token::TK_Double)
		{
			Lexer::GetNextToken();
			ty = std::make_unique<AST::Double>(0);
		}
		else if(Lexer::CurrentToken == Token::TK_Float)
		{
			Lexer::GetNextToken();
			ty = std::make_unique<AST::Float>(0);
		}
		else if(Lexer::CurrentToken == Token::TK_String)
		{
			ty = ParseString();
		}
		else
		{
			for(int i = 0; i < AllStructs.size(); i++)
			{
				std::string GetName = std::string(AllStructs[i]->StructLLVM->getName());

				if(GetName == Lexer::IdentifierStr)
				{
					Lexer::GetNextToken();

					return std::make_unique<AST::StructTy>(AllStructs[i]->Name, AllStructs[i]->StructLLVM);
				}
			}
		}

		if(Lexer::CurrentToken == '[')
		{
			unsigned int arraySize = 0;

			Lexer::GetNextToken();

			bool dyn = false;
			
			if(Lexer::CurrentToken != ']')
			{
				if(Lexer::NumValString.find(".") != std::string::npos || Lexer::NumValString.find("f") != std::string::npos)
					return LogError("Array size can only be an integer.");
	
				arraySize = std::stoi(Lexer::NumValString);
	
				Lexer::GetNextToken();
	
				if(Lexer::CurrentToken != ']')
					return LogError("Expected ']' to close array size. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");
			}
			else
				dyn = true;

			Lexer::GetNextToken();

			auto arr = std::make_unique<AST::Array>(std::move(ty), arraySize);
			arr->dynamicInitialization = dyn;

			//std::cout << "Array " << currentIdentifierString << " parsed!\n";
			localArrayNames.push_back(currentIdentifierString);

			return arr;
		}

		return ty;
	}

	static std::unique_ptr<AST::Function> ParseDefinition()
	{
		localArrayNames.clear();
		localNestedArrayNames.clear();
		localStructVariables.clear();

		Lexer::GetNextToken();

		auto Proto = ParsePrototype();
		if(!Proto) 
			return nullptr;

		if(Lexer::CurrentToken != '{')
			LogErrorF("Expected '{'. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");

		bracketCount++;

		Lexer::GetNextToken();

		auto Expression = ParseExpression();

		if(Lexer::CurrentToken != '}')
			Lexer::GetNextToken();

		if(Lexer::CurrentToken != '}')
		{
			if(Lexer::CurrentToken != Token::TK_Identifier)
				return LogErrorF("Expected '}' at end of function. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");
			else
				return LogErrorF("Expected '}' at end of function. Current Identifier: " + Lexer::IdentifierStr + ".");
		}

		bracketCount--;

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
		//Lexer::GetNextToken();

		//if(auto Expression = ParseExpression())
		//{
		//	auto Proto = std::make_unique<AST::FunctionPrototype>(std::make_unique<AST::Double>(0), "", std::vector<std::pair<std::unique_ptr<AST::Expression>, std::unique_ptr<AST::Variable>>>());
		//	return std::make_unique<AST::Function>(std::move(Proto), std::move(Expression));
		//}

		return nullptr;
	}

	static std::unique_ptr<AST::Expression> ParseIfExpression()
	{
		SourceLocation IfLoc = Lexer::CurrentLocation;

		Lexer::GetNextToken();

		if(Lexer::CurrentToken != '(')
			return LogError("Expected '('.");

		Lexer::GetNextToken();

		auto Condition = ParseExpression();

		if(!Condition)
			return nullptr;

		if(Lexer::CurrentToken != ')')
			return LogError("Expected ')'. Current Token: " + std::to_string(Lexer::CurrentToken));

		Lexer::GetNextToken();

		bool openedBlock = false;

		if(Lexer::CurrentToken != '{' && Lexer::CurrentToken != Token::TK_Then)
		{
			if(Lexer::CurrentToken != Token::TK_Identifier)
				return LogError("Expected '{' at end of if condition. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");
			else
				return LogError("Expected '{' at end of if condition. Current Identifier: " + Lexer::IdentifierStr + ".");
		}

		if(Lexer::CurrentToken == '{')
			openedBlock = true;

		bracketCount++;

		Lexer::GetNextToken();

		auto Then = ParseExpression();

		if(!Then)
			return nullptr;

		if(openedBlock)
		{
			if(Lexer::CurrentToken != '}')
				return LogError("Expected '}' at end of if block. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");
			else
				openedBlock = false;
		}

		bracketCount--;

		Lexer::GetNextToken();

		if(Lexer::CurrentToken != Token::TK_Else)
			return std::make_unique<AST::If>(IfLoc, std::move(Condition), std::move(Then), nullptr);

		Lexer::GetNextToken();

		if(Lexer::CurrentToken == '{')
		{
			openedBlock = true;
		}

		bracketCount++;

		if(Lexer::CurrentToken != Token::TK_If)
			Lexer::GetNextToken();

		auto Else = ParseExpression();

		if(!Else)
			return nullptr;

		if(openedBlock)
		{	
			if(Lexer::CurrentToken != '}')
				return LogError("Expected '}' at end of else block. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");
		}

		bracketCount--;

		return std::make_unique<AST::If>(IfLoc, std::move(Condition), std::move(Then), std::move(Else));
	}

	static std::unique_ptr<AST::Expression> ParseForExpression()
	{
		Lexer::GetNextToken();

		if(Lexer::CurrentToken != '(')
			return LogError("Expected '(' at start of for loop.");

		Lexer::GetNextToken();

		if(Lexer::CurrentToken != Token::TK_Identifier)
			return LogError("Expected identifier after for. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");

		std::string idName = Lexer::IdentifierStr;
		Lexer::GetNextToken();

		if(Lexer::CurrentToken != ':')
			return LogError("Expected ':' to set identifier type.");

		Lexer::GetNextToken();

		auto Type = ParseFunctionType();
		if(!Type)
			return LogError("Expected type for identifier.");

		if(Lexer::CurrentToken != '=')
			return LogError("Expected '=' after identifier.");

		Lexer::GetNextToken();

		dotCommaAsOperator = false;

		//std::cout << "In Start...\n";

		auto Start = ParseExpression();

		if(!Start)
			return nullptr;

		if(Lexer::CurrentToken != TK_DotComma)
			return LogError("Expected ';' after start of for loop. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");

		Lexer::GetNextToken();

		//std::cout << "In End...\n";

		auto End = ParseExpression();
		if(!End)
			return nullptr;

		std::unique_ptr<AST::Expression> Step;
		if(Lexer::CurrentToken == TK_DotComma)
		{
			//std::cout << "In Step...\n";

			Lexer::GetNextToken();
			Step = ParseExpression();
			if(!Step)
				return nullptr;
		}

		if(Lexer::CurrentToken != ')')
			return LogError("Expected ')' to close for loop args.");

		dotCommaAsOperator = true;

		Lexer::GetNextToken();

		if(Lexer::CurrentToken != '{')
			return LogError("Expected '{' in for loop.");

		bracketCount++;

		Lexer::GetNextToken();

		//std::cout << "In Body...\n";

		auto Body = ParseExpression();
		if(!Body)
			return nullptr;

		//std::cout << "In End of Loop...\n";

		//Lexer::GetNextToken();

		if(Lexer::CurrentToken != '}')
			return LogError("Expected '}' at end of for loop. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");

		bracketCount--;

		std::unique_ptr<AST::Expression> Next = nullptr;

		Lexer::GetNextToken();

		if(Lexer::CurrentToken != '}')
			Next = ParseExpression();

		return std::make_unique<AST::For>(std::move(Type), idName, std::move(Start), std::move(End), std::move(Step), std::move(Body), std::move(Next));
	}

	static std::unique_ptr<AST::Expression> ParseUnary()
	{
		if(!isascii(Lexer::CurrentToken) || Lexer::CurrentToken == '(' || Lexer::CurrentToken == ',' || Lexer::CurrentToken == '[' || Lexer::CurrentToken == '<')
			return ParsePrimary();

		int Opc = Lexer::CurrentToken;
		Lexer::GetNextToken();

		if(auto Operand = ParseUnary())
			return std::make_unique<AST::Unary>(Opc, std::move(Operand));

		return nullptr;
	}

	static std::unique_ptr<AST::Expression> ParseVar(bool isInStruct = false)
	{
		//std::cout << "Parser Found Var!\n";

		bool isPointer = false;

		if(Lexer::CurrentToken == Token::TK_Ptr)
		{
			isPointer = true;
		}

		Lexer::GetNextToken();

		std::vector<AST::VarStruct> VarNames;

		dotCommaAsOperator = false;

		if(Lexer::CurrentToken != Token::TK_Identifier)
			return LogError("Expected identifier after var.");

		while (1)
		{
			std::string Name = Lexer::IdentifierStr;

			currentIdentifierString = Name;

			Lexer::GetNextToken();

			std::unique_ptr<AST::Expression> Type;
			if (Lexer::CurrentToken != ':')
				return LogError("Expected type after var identifier.");

			Lexer::GetNextToken();

			Type = ParseFunctionType();
			if(!Type) return nullptr;

			Type->isPointer = isPointer;

			if(dynamic_cast<AST::Array*>(Type.get()) != nullptr)
				Parser::lastArray = (AST::Array*)Type.get();

			if(dynamic_cast<AST::NestedArray*>(Type.get()) != nullptr)
			{
				Parser::lastNestedArray = (AST::NestedArray*)Type.get();
				beginNestedArray = true;
			}

			std::unique_ptr<AST::Expression> Init;
			if(Lexer::CurrentToken == '=')
			{
				Lexer::GetNextToken();

				Init = ParseExpression();
				if(!Init) return nullptr;

				Init->isPointer = isPointer;

				if(dynamic_cast<AST::ArrayInitContent*>(Init.get()) != nullptr)
				{
					if(Parser::lastArray == nullptr)
						return LogError("Unexpected array initialization.");

					AST::ArrayInitContent* getAIC = (AST::ArrayInitContent*)Init.get();

					if(getAIC == nullptr)
						return LogError("ArrayInitContent is nullptr.");

					Parser::lastArray->variables = std::move(getAIC->variables);

					if(Parser::lastArray->dynamicInitialization)
						Parser::lastArray->size = Parser::lastArray->variables.size();

					Parser::lastArray = nullptr;
				}
				else if(dynamic_cast<AST::NestedArrayContent*>(Init.get()) != nullptr)
				{
					if(Parser::lastNestedArray == nullptr)
						return LogError("Unexpected nested array initialization.");

					AST::NestedArrayContent* getNAC = (AST::NestedArrayContent*)Init.get();

					if(getNAC == nullptr)
						return LogError("NestedArrayContent is nullptr.");

					Parser::lastNestedArray->arrays = std::move(getNAC->variables);

					if(Parser::lastNestedArray->dynamicInitialization)
						Parser::lastNestedArray->size = Parser::lastNestedArray->arrays.size();

					Parser::lastNestedArray = nullptr;
				}
			}

			AST::VarStruct v;
			v.type = std::move(Type);
			v.name = Name;
			v.body = std::move(Init);

			if(v.name == "test")
			{
				//std::cout << "test Found!\n";
			}

			if(dynamic_cast<AST::StructTy*>(v.type.get()) != nullptr)
			{
				AST::StructTy* stru = (AST::StructTy*)v.type.get();
			
				//std::cout << "Adding Struct Local Variable... Name: " << Name << "\n";
			
				localStructVariables.push_back(std::make_pair(Name, stru->structName));
			}
			else
			{
				//std::cout << "test is not StructTy!\n";
			}

			VarNames.push_back(std::move(v));

			if(Lexer::CurrentToken != ',') { break; }

			Lexer::GetNextToken();

			if(Lexer::CurrentToken != Token::TK_Identifier)
				return LogError("Expected identifier list after var");
		}

		if(Lexer::CurrentToken != Token::TK_DotComma)
			return LogError("Expected ';' after 'var'. Current Token: " + std::to_string(Lexer::CurrentToken) + ".");

		dotCommaAsOperator = true;

		Lexer::GetNextToken();

		std::unique_ptr<AST::Expression> Body;

		if(!isInStruct)
		{
			Body = ParseExpression();
			if(!Body)
				return nullptr;
		}

		return std::make_unique<AST::Var>(std::move(VarNames), std::move(Body));
	}

	static std::unique_ptr<AST::StructEx> ParseStruct()
	{
		Lexer::GetNextToken();

		std::string Name = Lexer::IdentifierStr;
		currentIdentifierString = Name;

		Lexer::GetNextToken();

		if(Lexer::CurrentToken != '{')
		{
			//std::cout << "Error: Expected '{' at start of struct. Current Token: " << std::to_string(Lexer::CurrentToken) << ".\n";
			exit(1);
		}

		Lexer::GetNextToken();

		std::vector<std::unique_ptr<AST::Expression>> variables;

		if(Lexer::CurrentToken != '}')
		{
			while(1)
			{
				if(Lexer::CurrentToken != Token::TK_Var && Lexer::CurrentToken != Token::TK_Ptr)
				{
					//std::cout << "Error: Expected '{' at start of struct. Current Token: " << std::to_string(Lexer::CurrentToken) << ".\n";
					exit(1);
				}

				//std::cout << "Parsing Var..." << std::endl;

				auto Vars = ParseVar(true);
				if(!Vars)
				{
					//std::cout << "Error: Returned nullptr: " << std::to_string(Lexer::CurrentToken) << ".\n";
					exit(1);
				}

				variables.push_back(std::move(Vars));

				//std::cout << "Var Done!" << std::endl;

				if(Lexer::CurrentToken == '}')
					break;
			}
		}

		Lexer::GetNextToken();

		if(Lexer::CurrentToken != Token::TK_DotComma)
		{
			//std::cout << "Expected ';' at end of struct. Current Token: " << std::to_string(Lexer::CurrentToken) << ".\n";
			exit(1);
		}

		return std::make_unique<AST::StructEx>(Name, std::move(variables));
	}
};

struct ParseTesting
{
	static void Definition()
	{
		if(auto FnAST = Parser::ParseDefinition())
		{
			//std::cout << "Handling Definition...\n";

			if(auto *FnIR = FnAST->codegen())
			{
				//fprintf(stderr, "Parsed a function definition:\n\n");
				//FnIR->print(llvm::errs());
				//fprintf(stderr, "\n");
			}

			//std::cout << "Finished Definition...\n";
		}
		else
			Lexer::GetNextToken();
	}

	static void Extern()
	{
		if(auto ProtoAST = Parser::ParseExtern())
		{
			//std::cout << "Handling Extern...\n";

			if(auto *FnIR = ProtoAST->codegen())
			{
				//fprintf(stderr, "Parsed an extern:");
				//FnIR->print(llvm::errs());
				//fprintf(stderr, "\n");

				AST::FunctionProtos[ProtoAST->Name()] = std::move(ProtoAST);
				
				//std::cout << "Finished Extern...\n";
			}
		}
		else
			Lexer::GetNextToken();
	}

	static void Struct()
	{
		if(auto StAST = Parser::ParseStruct())
		{
			//std::cout << "Handling Definition...\n";

			if(auto *StIR = StAST->codegen())
			{
				Parser::AllStructs.push_back(std::move(StAST));
			}

			//std::cout << "Finished Definition...\n";
		}
		else
			Lexer::GetNextToken();
	}

	static void TopLevelExpression()
	{
		if(auto FnAST = Parser::ParseTopLevelExpression())
		{
			//std::cout << "Handling Top Level Expression...\n";

			if(auto *FnIR = FnAST->codegen())
			{
				//fprintf(stderr, "Parsed a top level expression:");
				//FnIR->print(llvm::errs());
				//fprintf(stderr, "\n");

				FnIR->eraseFromParent();
				return;
			}
		}
		else
			Lexer::GetNextToken();
	}
};

#endif