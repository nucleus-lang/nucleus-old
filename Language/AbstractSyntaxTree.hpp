#ifndef ABSTRACT_SYNTAX_TREE_H
#define ABSTRACT_SYNTAX_TREE_H

#include "Lexer.hpp"
#include <string>
#include "CodeGeneration.hpp"

struct AST
{
	struct Expression
	{
		virtual ~Expression() = default;
		virtual llvm::Value* codegen() = 0;
	};

	struct Number : public Expression
	{
		bool isDouble = false, isInt = false, isFloat = false;
		int intValue = 0;
		double doubleValue = 0;
		float floatValue = 0;
		Number(std::string val) 
		{
			//std::cout << "Number Input: " << val << "\n";
			if(val.find(".") != std::string::npos)
			{
				if(val.back() == 'f')
				{
					isFloat = true;
					floatValue = std::stof(val);
				}
				else
				{
					isDouble = true;
					doubleValue = std::stod(val);
				}
			}
			else
			{
				isInt = true;
				intValue = std::stoi(val);
			}
		}
		llvm::Value* codegen() override;
	};

	struct Integer : public Expression
	{
		int value;
		Integer(int val) : value(val) {}
		llvm::Value* codegen() override;
	};

	struct Double : public Expression
	{
		double value;
		Double(double val) : value(val) {}
		llvm::Value* codegen() override;
	};

	struct Float : public Expression
	{
		float value;
		Float(float val) : value(val) {}
		llvm::Value* codegen() override;
	};

	struct Variable : public Expression
	{
		std::string name;
		Variable(const std::string& n) : name(n) {}
		llvm::Value* codegen() override;
	};

	struct Binary : public Expression
	{
		char op;
		std::unique_ptr<Expression> lhs, rhs;

		Binary(char oper, std::unique_ptr<Expression> left,
                std::unique_ptr<Expression> right)
    	: op(oper), lhs(std::move(left)), rhs(std::move(right)) {}

    	llvm::Value* codegen() override;
	};

	struct Call : public Expression
	{
		std::string callee;
		std::vector<std::unique_ptr<Expression>> arguments;

		Call(const std::string& c, std::vector<std::unique_ptr<Expression>> args)
		: callee(c), arguments(std::move(args)) {}

		llvm::Value* codegen() override;
	};

	struct FunctionPrototype
	{
		std::unique_ptr<Expression> type;
		std::string name;
		std::vector<std::pair<std::unique_ptr<AST::Expression>, std::unique_ptr<AST::Variable>>> arguments;

		FunctionPrototype(std::unique_ptr<Expression> t, const std::string& n, std::vector<std::pair<std::unique_ptr<AST::Expression>, std::unique_ptr<AST::Variable>>> args)
		: type(std::move(t)), name(n), arguments(std::move(args)) {}

		const std::string& Name() const { return name; }

		llvm::Function* codegen();
	};

	static std::map<std::string, std::unique_ptr<FunctionPrototype>> FunctionProtos;

	struct Function
	{
		std::unique_ptr<FunctionPrototype> prototype;
		std::unique_ptr<Expression> body;

		Function(std::unique_ptr<FunctionPrototype> proto, std::unique_ptr<Expression> b)
		: prototype(std::move(proto)), body(std::move(b)) {}

		llvm::Function* codegen();
	};
};

#endif